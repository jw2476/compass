#pragma once

#include "arena.h"
#include "array.h"
#include "str.h"

typedef struct u8_array_t u8_array_t;

u8_array_t file_read(arena_t* arena, str_t path);
void file_write(str_t path, u8_array_t* data);
