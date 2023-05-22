#include <stdio.h>
#include <stdlib.h>

#include "base.h"
#include "token.h"	

typedef enum type_kind_t {
	TYPE_VALUE,
	TYPE_TUPLE,
	TYPE_FUNCTION,
	TYPE_STRUCT,
	TYPE_NEVER
} type_kind_t;

typedef struct value_type_t value_type_t;
typedef union type_data_t {
	value_type_t* value;
} type_data_t;

typedef struct type_t {
	type_kind_t kind;
	type_data_t data;
} type_t;

MAKE_ARRAY(type);

typedef struct value_type_t {
	str_t name;
	type_array_t generics;	
} value_type_t;

MAKE_ARRAY(value_type);
MAKE_ARRAY(str);

typedef struct generic_decl_t {
	str_array_t types;
	value_type_array_t conditions;	
} generic_decl_t;

typedef struct variant_t {
	str_t name;
	type_t type;
} variant_t;

MAKE_ARRAY(variant);

typedef struct enum_decl_t {
	str_t name;
	generic_decl_t generic;
	variant_array_t variant;
} enum_decl_t;

MAKE_ARRAY(enum_decl);

typedef struct ast_t {
	enum_decl_array_t enums;
} ast_t;


ast_t parse()

int compass_main() {
	arena_t arena = arena_new();
	u8_array_t file = file_read(&arena, str_lit(&arena, "test.tog"));
	str_t code = str_from_ascii(&arena, &file);

	while (code.size != 0) {
		token_t token = tokenize(&code);
		token_print(&token);
		printf("\n");
	}

	arena_destroy(&arena);
	return 0;
}
