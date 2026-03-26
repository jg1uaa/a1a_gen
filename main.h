// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025-2026 SASANO Takayoshi <uaa@uaa.org.uk>

#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

#define IGNORE_CHARS 64

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
	double jitter;
	int sent_chars;

	double dah_ratio;
	double wordspace_ratio;
	double charspace_ratio;

	bool ignore_crlf;
	wchar_t ignore_char[IGNORE_CHARS + 1];
};

#endif
