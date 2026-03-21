// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 SASANO Takayoshi <uaa@uaa.org.uk>

#include <stdio.h>
#include <string.h>
#include "player.h"
#include "table.h"
#include "rng.h"

static struct params *ppar;

static FILE *fp;

static bool space_sent = true;
static bool disable_char_space = false;
static bool char_space_is_disabled = false;

/* fluctuation 1/f */
static double calc_1f(int index)
{
	const double lim[] = {0.05, 0.25}; // 0.25 for word start
	static double x = 0;

	if (x < lim[index] || x > 1 - lim[index])
		return (x = random_value_double(lim[index], 1 - lim[index]));

	if (x <= 0.5)
		return (x = x + 2 * x * x);
	else
		return (x = x - 2 * (1 - x) * (1 - x));
}

static int64_t dot_usec(int index)
{
	int64_t us = ppar->dot_usec;

	if (ppar->jitter)
		us *= (1 - ppar->jitter) + (2 * ppar->jitter * calc_1f(index));

	return us;
}

static void play_char(const char *code)
{
	int i, code_len = strlen(code);

	for (i = 0; i < code_len; i++) {
		switch (code[i]) {
		case '.':
			(*ppar->outfunc)(true, dot_usec(0));
			break;
		case '-':
			(*ppar->outfunc)(true, dot_usec(0) *
					 ppar->dah_ratio);
			break;
		case ' ':
			if (!char_space_is_disabled)
				(*ppar->outfunc)(false, dot_usec(0) *
						 ppar->charspace_ratio);
			continue;
		}

		if (i < code_len - 1 && code[i + 1] != ' ')
			(*ppar->outfunc)(false, dot_usec(0));
	}
}

static const struct morse_table *fetch_code(wchar_t c)
{
	int i;

	for (i = 0; i < codetable_entry; i++) {
		if (wcschr(codetable[i].characters, c) != NULL)
			return &codetable[i];
	}

	return NULL;
}

static void play_line(wchar_t *buf)
{
	wchar_t *p;
	const struct morse_table *t;

	/* ignore after '#' */
	if ((p = wcschr(buf, L'#')) != NULL) *p = L'\0';

	/* skip preceding space */
	for (p = buf; *p && fetch_code(*p) == NULL; p++);

	for (; *p; p++) {
		if (ppar->ignore_crlf && (*p == L'\n' || *p == L'\r'))
			continue;

		if (*p == L'<') {
			disable_char_space = true;
			char_space_is_disabled = space_sent;
			continue;
		} else if (*p == L'>') {
			disable_char_space = false;
			char_space_is_disabled = false;
			continue;
		}

		if (!disable_char_space &&
		    wcschr(ppar->ignore_char, *p) != NULL)
			continue;

		if ((t = fetch_code(*p)) == NULL) {
			if (space_sent || disable_char_space) continue;
			space_sent = true;
			(*ppar->outfunc)(false, dot_usec(1) *
					 ppar->wordspace_ratio);
		} else {
			if (!space_sent) {
				(*ppar->outfunc)(false, dot_usec(0) *
						 (char_space_is_disabled ?
						  1 : ppar->charspace_ratio));
				char_space_is_disabled =
					disable_char_space;
			}
			space_sent = false;
			play_char(t->code);
			ppar->sent_chars += t->count;
		}
	}
}

void player_start(void)
{
	wchar_t buf[1024];

	while (fgetws(buf, sizeof(buf) / sizeof(wchar_t), fp) != NULL)
		play_line(buf);
}

int player_init(struct params *par)
{
	if (par == NULL) {
		fclose(fp);
		return 0;
	}

	ppar = par;
	ppar->sent_chars = 0;

	fp = strcmp("-", ppar->infile) ? fopen(ppar->infile, "r") : stdin;
	if (fp == NULL) {
		fprintf(stderr, "player_init: file open error\n");
		return -1;
	}

	return 0;
}
