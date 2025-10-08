// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 SASANO Takayoshi <uaa@uaa.org.uk>

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "output.h"

static struct params *ppar;

static double (*wave)(int, int);
static bool format_bigendian;
static bool format_signed;
static bool format_16bit;
static int64_t samples_counter;
static FILE *fp;

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

static void play_tone(bool on, int64_t usec)
{
	int64_t i;
	double t;
	short s, n;

	for (i = 0; i < (ppar->sample_freq * usec) / 1000000; i++) {
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

		for (n = 0; n < ppar->channels; n++) {
			if (format_16bit && !format_bigendian) fputc(s, fp);
			fputc((s >> 8) ^ (format_signed ? 0x00 : 0x80), fp);
			if (format_16bit && format_bigendian) fputc(s, fp);
		}
	}

	samples_counter += i;
}

int output_init(struct params *par)
{
	if (par == NULL) {
		fclose(fp);
		return 0;
	}

	ppar = par;
	ppar->outfunc = play_tone;
	samples_counter = 0;
	format_signed = format_16bit = true;
	format_bigendian = false;
	wave = wave_sine;

	fp = strcmp("-", ppar->outfile) ? fopen(ppar->outfile, "w") : stdout;
	if (fp == NULL) {
		fprintf(stderr, "output_init: file open error\n");
		return -1;
	}

	if (ppar->arg1 != NULL) {
		if (!strcmp("u8", ppar->arg1))
			format_signed = format_16bit = format_bigendian = false;
		else if (!strcmp("s16be", ppar->arg1))
			format_signed = format_16bit = format_bigendian = true;
	}

	if (ppar->arg2 != NULL) {
		if (!strcmp("square", ppar->arg2)) wave = wave_square;
		else if (!strcmp("saw", ppar->arg2)) wave = wave_saw;
		else if (!strcmp("triangle", ppar->arg2)) wave = wave_triangle;
	}

	return 0;
}

double output_sec(void)
{
	return (double)samples_counter / ppar->sample_freq;
}
