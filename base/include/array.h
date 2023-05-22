#pragma once

#include "types.h"
#include "arena.h"

#define MAKE_ARRAY(type) \
typedef struct type ## _array_t { \
	type ## _t* data; \
	usize size; \
} type ## _array_t; \
\
type ## _array_t type ## _array_convert(array_t array) { \
	type ## _array_t out; \
	out.data = (type ## _t*)array.data; \
	out.size = array.size; \
	return out; \
}

typedef struct array_t {
	void* data;
	usize size;
} array_t;

array_t array_new(arena_t* arena, usize size);
#define ARRAY_NEW(type, arena, size) type ## _array_convert(array_new(arena, size * sizeof(type ## _t)))

MAKE_ARRAY(u8)
MAKE_ARRAY(u16)
MAKE_ARRAY(u32)
MAKE_ARRAY(u64)
MAKE_ARRAY(usize)
MAKE_ARRAY(i8)
MAKE_ARRAY(i16)
MAKE_ARRAY(i32)
MAKE_ARRAY(i64)
MAKE_ARRAY(isize)
MAKE_ARRAY(f32)
MAKE_ARRAY(f64)
