// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 SASANO Takayoshi <uaa@uaa.org.uk>

#include <stdio.h>
#include <string.h>
#include <locale.h>
#include "player.h"
#include "table.h"

static struct params *ppar;

static FILE *fp;

static void play_char(const char *code)
{
	int i, code_len = strlen(code);

	for (i = 0; i < code_len; i++) {
		switch (code[i]) {
		case '.':
			(*ppar->outfunc)(true, ppar->dot_usec);
			break;
		case '-':
			(*ppar->outfunc)(true, ppar->dot_usec *
					 ppar->dah_ratio);
			break;
		}

		if (i != code_len - 1)
			(*ppar->outfunc)(false, ppar->dot_usec);
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

static bool space_sent = true;
static bool disable_character_space = false;

static void play_line(wchar_t *buf)
{
	wchar_t *p;
	const struct morse_table *t;

	/* ignore after '#' */
	if ((p = wcschr(buf, L'#')) != NULL) *p = L'\0';

	/* skip preceding space */
	for (p = buf; *p && fetch_code(*p) == NULL; p++);

	for (; *p; p++) {
		if (*p == L'<') {
			disable_character_space = true;
			continue;
		} else if (*p == L'>') {
			disable_character_space = false;
			continue;
		}

		if ((t = fetch_code(*p)) == NULL) {
			if (space_sent || disable_character_space) continue;
			space_sent = true;
			(*ppar->outfunc)(false, ppar->dot_usec *
					 ppar->wordspace_ratio);
		} else {
			if (!space_sent)
				(*ppar->outfunc)(false, ppar->dot_usec *
						 (disable_character_space ?
						  1 : ppar->charspace_ratio));
			space_sent = false;
			play_char(t->code);
			ppar->sent_chars++;
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

	setlocale(LC_CTYPE, "en_US.UTF-8"); /* UTF-8 locale required */

	fp = strcmp("-", ppar->infile) ? fopen(ppar->infile, "r") : stdin;
	if (fp == NULL) {
		fprintf(stderr, "player_init: file open error\n");
		return -1;
	}

	return 0;
}
