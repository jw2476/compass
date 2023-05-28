#include "file.h"

#include <stdio.h>
#include <stdlib.h>

u8_array_t file_read(arena_t* arena, str_t path) {
	arena_t scratch = arena_new();

	FILE* f = fopen(str_cstr(&scratch, &path), "rb");
	if (f == NULL) {
		exit(-1);
	}

	fseek(f, 0, SEEK_END);
	usize size = ftell(f);
	fseek(f, 0, SEEK_SET);

	u8_array_t array = u8_array_new(arena, size, sizeof(1));
	fread(array.data, size, 1, f);
	fclose(f);

	return array;
}

void file_write(str_t path, u8_array_t* data) {
	arena_t scratch = arena_new();
	FILE* f = fopen(str_cstr(&scratch, &path), "wb");
	fwrite(data->data, data->size, 1, f);
	fclose(f);
}
