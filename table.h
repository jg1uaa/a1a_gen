// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 SASANO Takayoshi <uaa@uaa.org.uk>

#include <wchar.h>

struct morse_table {
	const wchar_t *characters;
	const char *code;
};

extern const struct morse_table codetable[];
extern const int codetable_entry;
	
