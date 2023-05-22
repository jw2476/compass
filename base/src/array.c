#include "array.h"

array_t array_new(arena_t* arena, usize size) {
	array_t array;
	array.size = size;
	array.data = arena_push(arena, size);

	return array;
}
