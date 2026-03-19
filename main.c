// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 SASANO Takayoshi <uaa@uaa.org.uk>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include "player.h"
#include "output.h"
#include "rng.h"

extern char *optarg;
extern int optind;

struct params par = {
	.infile = NULL,
	.outfile = NULL,
	.outfunc = NULL,

	.sample_freq = 48000,
	.channels = 2,
	.tone1_freq = 600,
	.tone2_freq = 0,
	.volume = 0.8,
	.arg1 = NULL,
	.arg2 = NULL,

	.dot_usec = 60000,
	.sent_chars = 0,

	.dah_ratio = 3,
	.wordspace_ratio = 6,
	.charspace_ratio = 3,

	.ignore_crlf = false,
	.ignore_char = L"",
};

void parse_value(char *str, double *v1, double *v2)
{
	char *p, *q;

	p = str;
	*v1 = *v2 = strtod(str, &p);
	if (str == p || *p != ',')
		return;

	p++;
	*v2 = strtod(p, &q);
	if (p == q)
		*v2 = *v1;
}

double get_value(char *str)
{
	double v1, v2;

	parse_value(str, &v1, &v2);
	return random_value_double(v1, v2);
}

int main(int argc, char *argv[])
{
	int ch;
	size_t n;
	double d;
	int quiet = -1;

	initialize_random_generator();
	setlocale(LC_CTYPE, "C.UTF-8"); /* UTF-8 locale required */

	while ((ch = getopt(argc, argv,
			    "i:o:r:c:t:T:v:f:w:d:p:H:W:C:q:nI:")) != -1) {
		switch (ch) {
		case 'i': par.infile = optarg; break;
		case 'o': par.outfile = optarg; break;
		case 'r': par.sample_freq = atoi(optarg); break;
		case 'c': par.channels = atoi(optarg); break;
		case 't': par.tone1_freq = get_value(optarg); break;
		case 'T': par.tone2_freq = get_value(optarg); break;
		case 'v': par.volume = get_value(optarg); break;
		case 'f': par.arg1 = optarg; break;
		case 'w': par.arg2 = optarg; break;
		case 'd': par.dot_usec = get_value(optarg) * 1000; break;
		case 'p':
			if ((d = get_value(optarg)) > 0)
				par.dot_usec = 1200000 / d;
			break;
		case 'H': par.dah_ratio = get_value(optarg); break;
		case 'W': par.wordspace_ratio = get_value(optarg); break;
		case 'C': par.charspace_ratio = get_value(optarg); break;
		case 'q': quiet = atoi(optarg); break;
		case 'n': par.ignore_crlf = true; break;
		case 'I':
			n = mbstowcs(par.ignore_char, optarg, IGNORE_CHARS);
			par.ignore_char[(n == (size_t)-1) ? 0 : n] = L'\0';
			break;
		}
	}

	if (par.infile == NULL || par.outfile == NULL ||
	    par.sample_freq < 8000 || par.channels < 1 || par.channels > 2 ||
	    par.tone1_freq < 1 || par.tone2_freq < 0 || par.volume < 0 ||
	    par.dot_usec < 1000 || par.dah_ratio < 1 ||
	    par.wordspace_ratio < 1 || par.charspace_ratio < 1) {
		fprintf(stderr,
			"usage: %s -i [infile] -o [outfile] "
			"-d [dot_msec] -p [paris_wpm]\n",
			argv[0]);
		goto fin0;
	}		

	if (player_init(&par)) goto fin0;
	if (output_init(&par)) goto fin1;

	player_start();

	switch (quiet) {
	default:
		fprintf(stderr, "tone %.2f hz", par.tone1_freq);
		if (par.tone2_freq > 0)
			fprintf(stderr," + %.2f hz", par.tone2_freq);
		fprintf(stderr, " volume %.2f\n", par.volume);

		fprintf(stderr, "dot %.2f msec (%.2f wpm) ",
			par.dot_usec / 1000.0, 1200000.0 / par.dot_usec);
		fprintf(stderr, "dah %.2f wordspace %.2f charspace %.2f\n",
			par.dah_ratio, par.wordspace_ratio,
			par.charspace_ratio);
		/*FALLTHROUGH*/
	case 1:
		if ((d = output_sec())) {
			fprintf(stderr, "%d characters %.2f sec %.2f cpm\n",
				par.sent_chars, d, 60 * par.sent_chars / d);
		} else {
			fprintf(stderr, "%d characters\n", par.sent_chars);
		}
		/*FALLTHROUGH*/
	case 0:
		break;
	}		

//fin2:
	output_init(NULL);
fin1:
	player_init(NULL);
fin0:
	return 0;
}
