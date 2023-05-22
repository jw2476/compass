#include "arena.h"

#include <sys/mman.h>
#include <stdlib.h>

arena_t arena_new() {
	arena_t arena;
	arena.data = mmap(NULL, 1024 * 1024 * 1024, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);
	arena.ptr = 0;

	return arena;
}

void arena_destroy(arena_t* arena) {
	munmap(arena->data, 1024 * 1024 * 1024);
}

void* arena_push(arena_t* arena, usize size) {
	void* data = arena->data + arena->ptr;
	arena->ptr += size;

	return data;
}
