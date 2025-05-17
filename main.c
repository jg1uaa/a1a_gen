// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 SASANO Takayoshi <uaa@uaa.org.uk>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "player.h"
#include "output.h"

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
	.wordspace_ratio = 5,
	.charspace_ratio = 3,
};

int main(int argc, char *argv[])
{
	int ch;
	double d;
	bool quiet = false;

	while ((ch = getopt(argc, argv,
			    "i:o:r:c:t:T:v:f:w:d:p:H:W:C:q")) != -1) {
		switch (ch) {
		case 'i': par.infile = optarg; break;
		case 'o': par.outfile = optarg; break;
		case 'r': par.sample_freq = atoi(optarg); break;
		case 'c': par.channels = atoi(optarg); break;
		case 't': par.tone1_freq = atof(optarg); break;
		case 'T': par.tone2_freq = atof(optarg); break;
		case 'v': par.volume = atof(optarg); break;
		case 'f': par.arg1 = optarg; break;
		case 'w': par.arg2 = optarg; break;
		case 'd': par.dot_usec = atof(optarg) * 1000; break;
		case 'p': par.dot_usec = 1200000 / atof(optarg); break;
		case 'H': par.dah_ratio = atof(optarg); break;
		case 'W': par.wordspace_ratio = atof(optarg); break;
		case 'C': par.charspace_ratio = atof(optarg); break;
		case 'q': quiet = true; break;
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

	if (!quiet) {
		if ((d = output_sec())) {
			fprintf(stderr, "%d characters %.2f sec %.2f cpm\n",
				par.sent_chars, d, 60 * par.sent_chars / d);
		} else {
			fprintf(stderr, "%d characters\n", par.sent_chars);
		}
	}		

//fin2:
	output_init(NULL);
fin1:
	player_init(NULL);
fin0:
	return 0;
}
