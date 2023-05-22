#pragma once

#include "types.h"

typedef struct arena_t {
	void* data;
	usize ptr;
} arena_t;

arena_t arena_new();
void arena_destroy(arena_t* arena);

void* arena_push(arena_t* arena, usize size);

