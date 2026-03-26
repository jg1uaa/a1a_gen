// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025-2026 SASANO Takayoshi <uaa@uaa.org.uk>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <limits.h>
#include <math.h>
#include "output.h"
#include "rng.h"

static struct params *ppar;

static double (*wave)(int, int);
static void *(*fill)(void *, short);
static bool format_bigendian;
static bool format_16bit;
static int64_t samples_counter;
static FILE *fp;
static unsigned int tcache_index;

struct tone_cache {
	bool on;
	int64_t usec;
	int64_t bufsize;
	void *buf;
};

#define TCACHE_SIZE 8
static struct tone_cache tcache[TCACHE_SIZE];

enum wave_type {
	WAVE_INVALID,
	WAVE_SINE,
	WAVE_SQUARE,
	WAVE_SAW,
	WAVE_TRIANGLE,
};

static void init_tone_cache(void)
{
	tcache_index = 0;
	memset(tcache, 0, sizeof(tcache));
}

static void add_tone_cache(bool on, int64_t usec, int64_t bufsize, void *buf)
{
	struct tone_cache *t = &tcache[tcache_index % TCACHE_SIZE];

	free(t->buf);
	t->on = on;
	t->usec = usec;
	t->bufsize = bufsize;
	t->buf = buf;

	tcache_index++;
}

static void *find_tone_cache(bool on, int64_t usec, int64_t *bufsize)
{
	int i;
	struct tone_cache *t;

	for (i = 0; i < TCACHE_SIZE; i++) {
		t = &tcache[i];
		if (t->on == on && t->usec == usec) {
			*bufsize = t->bufsize;
			return t->buf;
		}
	}

	return NULL;
}

static void free_tone_cache(void)
{
	int i;

	for (i = 0; i < TCACHE_SIZE; i++)
		free(tcache[i].buf);

	init_tone_cache();
}

static double wave_sine(int pos, int cycle)
{
	return sin(2 * M_PI * pos / cycle);
}

static double wave_square(int pos, int cycle)
{
	return ((pos % cycle) < (cycle / 2)) ? 1.0 : -1.0;
}

static double wave_saw(int pos, int cycle)
{
	double v = 2.0 * (pos %= cycle) / cycle;
	return (pos < cycle / 2) ? v : (v - 2.0);
}

static double wave_triangle(int pos, int cycle)
{
	double v = 4.0 * (pos %= cycle) / cycle;
	if (pos < cycle / 4) return v;
	else if (pos < cycle * 3 / 4) return 2.0 - v;
	else return v - 4.0;
}

static void *fill_u8(void *buf, short sample)
{
	int i;
	uint8_t s = (sample >> 8) ^ 0x80;
	uint8_t *p = buf;

	for (i = 0; i < ppar->channels; i++)
		*p++ = s;

	return p;
}

static void *fill_s16(void *buf, short sample)
{
	int i;
	short s = format_bigendian ? htobe16(sample) : htole16(sample);
	short *p = buf;

	for (i = 0; i < ppar->channels; i++)
		*p++ = s;

	return p;
}

#define bufsize_factor ((format_16bit ? 2 : 1) * ppar->channels)

static void *alloc_tone(bool on, int64_t usec, int64_t *bufsize)
{
	int64_t i, samples;
	double t;
	short s;
	void *buf, *p;

	samples = (ppar->sample_freq * usec) / 1000000;
	*bufsize = samples * bufsize_factor;

	if ((buf = p = malloc(*bufsize)) == NULL)
		abort();

	for (i = 0; i < samples; i++) {
		if (on) {
			t = (*wave)(i, ppar->sample_freq / ppar->tone1_freq);
			if (ppar->tone2_freq) {
				t += (*wave)(i, ppar->sample_freq / ppar->tone2_freq);
				t /= 2.0;
			}
			s = t * SHRT_MAX * ppar->volume;
		} else {
			s = 0;
		}

		p = (*fill)(p, s);
	}

	return buf;
}

static void play_tone(bool on, int64_t usec)
{
	int64_t bufsize;
	void *buf;

	if ((buf = find_tone_cache(on, usec, &bufsize)) == NULL) {
		buf = alloc_tone(on, usec, &bufsize);
		add_tone_cache(on, usec, bufsize, buf);
	}

	fwrite(buf, bufsize, 1, fp);
	samples_counter += bufsize / bufsize_factor;
}

int output_init(struct params *par)
{
	enum wave_type w;

	if (par == NULL) {
		free_tone_cache();
		fclose(fp);
		return 0;
	}

	ppar = par;
	ppar->outfunc = play_tone;
	samples_counter = 0;
	format_16bit = true;
	format_bigendian = false;
	wave = wave_sine;
	init_tone_cache();

	fp = strcmp("-", ppar->outfile) ? fopen(ppar->outfile, "w") : stdout;
	if (fp == NULL) {
		fprintf(stderr, "output_init: file open error\n");
		return -1;
	}

	if (ppar->arg1 != NULL) {
		if (!strcmp("u8", ppar->arg1))
			format_16bit = format_bigendian = false;
		else if (!strcmp("s16be", ppar->arg1))
			format_16bit = format_bigendian = true;
	}

	if (ppar->arg2 != NULL) {
		if (!strcmp("sine", ppar->arg2)) w = WAVE_SINE;
		else if (!strcmp("square", ppar->arg2)) w = WAVE_SQUARE;
		else if (!strcmp("saw", ppar->arg2)) w = WAVE_SAW;
		else if (!strcmp("triangle", ppar->arg2)) w = WAVE_TRIANGLE;
		else if (!strcmp("random", ppar->arg2))
			w = random_value_int(WAVE_SINE, WAVE_TRIANGLE);
		else w = WAVE_INVALID;

		switch (w) {
		case WAVE_SINE: wave = wave_sine; break;
		case WAVE_SQUARE: wave = wave_square; break;
		case WAVE_SAW: wave = wave_saw; break;
		case WAVE_TRIANGLE: wave = wave_triangle; break;
		default: break;
		}
	}

	fill = format_16bit ? fill_s16 : fill_u8;

	return 0;
}

double output_sec(void)
{
	return (double)samples_counter / ppar->sample_freq;
}
