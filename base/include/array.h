#pragma once

#include "types.h"
#include "arena.h"

typedef struct array_t {
	void* data;
	usize size;
	usize sizePerItem;
} array_t;

array_t array_new(arena_t* arena, usize size, usize sizePerItem);
array_t array_view(array_t* base, usize from, usize to);
void array_skip(array_t* array, usize n);

#define MAKE_ARRAY(TYPE) \
typedef struct TYPE ## _array_t { \
	TYPE ## _t* data; \
	usize size; \
	usize sizePerItem; \
} TYPE ## _array_t; \
\
static TYPE ## _array_t(*TYPE ## _array_new)(arena_t*, usize, usize) = (void*)array_new; \
static TYPE ## _array_t(*TYPE ## _array_view)(TYPE ## _array_t*, usize, usize) = (void*)array_view; \
static void(*TYPE ## _array_skip)(TYPE ## _array_t*, usize) = (void*)array_skip;

#define MAKE_STD_ARRAYS() \
MAKE_ARRAY(u8); \
MAKE_ARRAY(u16); \
MAKE_ARRAY(u32); \
MAKE_ARRAY(u64); \
MAKE_ARRAY(usize); \
MAKE_ARRAY(i8); \
MAKE_ARRAY(i16); \
MAKE_ARRAY(i32); \
MAKE_ARRAY(i64); \
MAKE_ARRAY(isize); \
MAKE_ARRAY(f32); \
MAKE_ARRAY(f64);
