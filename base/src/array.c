#include "array.h"

array_t array_new(arena_t* arena, usize size, usize sizePerItem) {
	array_t array;
	array.size = size;
	array.sizePerItem = sizePerItem;
	array.data = arena_push(arena, size * sizePerItem);

	return array;
}

array_t array_view(array_t* base, usize from, usize to) {
	array_t array;
	array.data = &base->data[from * base->sizePerItem];
	array.size = (to - from);

	return array;
}

void array_skip(array_t* base, usize n) {
	base->data = &base->data[n * base->sizePerItem];
	base->size -= n;
}
