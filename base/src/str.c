#include "str.h"

#include <string.h>
#include <stdio.h>

str_t str_lit(arena_t* arena, const char* base) {
	usize size = strlen(base);
	
	str_t str;
	str.data = arena_push(arena, size);
	str.size = size;

	memcpy(str.data, base, size);

	return str;
}

str_t str_concat(arena_t* arena, str_t* a, str_t* b) {
	usize size = a->size + b->size;
	
	str_t str;
	str.data = arena_push(arena, size);
	str.size = size;

	memcpy(str.data, a->data, a->size);
	memcpy(&str.data[a->size], b->data, b->size);

	return str;
}

str_t str_view(str_t* base, usize from, usize to) {
	str_t str;
	str.data = &base->data[from];
	str.size = to - from;

	return str;
}

char* str_cstr(arena_t* arena, str_t* str) {
	char* buf = arena_push(arena, str->size + 1);
	memcpy(buf, str->data, str->size);
	return buf;
}

void str_print(str_t* base) {
	arena_t scratch = arena_new();
	fputs(str_cstr(&scratch, base), stdout);
	arena_destroy(&scratch);

}

str_t str_from_ascii(arena_t* arena, u8_array_t* ascii) {
	str_t str;
	str.data = arena_push(arena, ascii->size);
	str.size = ascii->size;
	memcpy(str.data, ascii->data, ascii->size);

	return str;
}

bool str_eq(str_t a, str_t b) {
	if (a.size != b.size) return false;
	bool result = true;
	for (usize i = 0; i < a.size; i++) {
		if (a.data[i] != b.data[i]) result = false;
	}
	return result;
}

bool str_starts_with(str_t* str, str_t with) {
	return str_eq(str_view(str, 0, with.size), with);
}

void str_skip_chars(str_t* str, usize n) {
	if (n > str->size) return;

	str->data = &str->data[n];
	str->size -= n;
}
