#pragma once

#include <stdbool.h>

#include "types.h"
#include "arena.h"
#include "array.h"

typedef struct str_t {
	char* data;
	usize size;
} str_t;

MAKE_ARRAY(str);

str_t str_lit(arena_t* arena, const char* base);
str_t str_concat(arena_t* arena, str_t* a, str_t* b);
str_t str_view(str_t* base, usize from, usize to);
char* str_cstr(arena_t* arena, str_t* str);
void str_print(str_t* str);
str_t str_from_ascii(arena_t* arena, u8_array_t* ascii);
bool str_eq(str_t a, str_t b);
bool str_starts_with(str_t* str, str_t with);
void str_skip_chars(str_t* str, usize n);

