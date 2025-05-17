// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 SASANO Takayoshi <uaa@uaa.org.uk>

#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdbool.h>
#include <stdint.h>

struct params {
	char *infile;
	char *outfile;
	void (*outfunc)(bool, int64_t);

	int sample_freq;
	int channels;
	double tone1_freq;
	double tone2_freq;
	double volume;
	void *arg1;		// data format
	void *arg2;		// wave type

	int64_t dot_usec;
	int sent_chars;

	double dah_ratio;
	double wordspace_ratio;
	double charspace_ratio;
};

#endif
