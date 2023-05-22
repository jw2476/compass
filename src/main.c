#include <stdio.h>
#include <stdlib.h>

#include "base.h"
#include "token.h"	

typedef struct type_t type_t;
typedef struct named_type_t named_type_t;

MAKE_STD_ARRAYS();
MAKE_ARRAY(str);
MAKE_ARRAY(token);
MAKE_ARRAY(type);
MAKE_ARRAY(named_type);

// u32 or Add<T, T, T>
typedef struct type_value_t {
	str_t name;
	str_array_t generics;
} type_value_t;

void type_value_print(type_value_t* type_value) {
	str_print(&type_value->name);
	printf("<");
	for (usize i = 0; i < type_value->generics.size; i++) {
		str_print(&type_value->generics.data[i]);
		if ((i + 1) != type_value->generics.size) printf(", ");
	}
	printf(">\n");
}

bool is_closing_angle(token_t* token) {
	return token->type == TOKEN_BRACKET && token->data.bracket.kind == BRACKET_ANGLE && token->data.bracket.side == BRACKET_CLOSE;
}

type_value_t type_value_parse(arena_t* arena, token_array_t* tokens) {
	if (tokens->data[0].type != TOKEN_IDENTIFIER) {
		CRITICAL("First token of type value isn't identifier");
	}

	type_value_t type_value;
	type_value.name = tokens->data[0].data.identifier;
	token_array_skip(tokens, 1);

	bool hasGenerics = tokens->data[0].type == TOKEN_BRACKET && tokens->data[0].data.bracket.kind == BRACKET_ANGLE && tokens->data[0].data.bracket.side == BRACKET_OPEN;
	if (!hasGenerics) {
		type_value.generics = str_array_new(arena, 0, sizeof(str_t));
		return type_value;
	}

	token_array_skip(tokens, 1);

	usize i;
	for (i = 0; !is_closing_angle(&tokens->data[i]); i++) {}

	type_value.generics = str_array_new(arena, i, sizeof(str_t));
	for (usize j = 0; j < i; j++) {
		type_value.generics.data[j] = tokens->data[j].data.identifier;
	}

	return type_value;
}

typedef enum type_type_t {
	TYPE_VALUE,
	TYPE_TUPLE,
	TYPE_STRUCT,
	TYPE_ENUM
} type_type_t;

typedef union type_data_t {
	type_value_t value;
	type_array_t tuple;
	named_type_array_t _struct;
	named_type_array_t _enum;
} type_data_t;

typedef struct type_t {
	type_type_t type;
	type_data_t data;
} type_t;

typedef struct named_type_t {
	str_t name;
	type_t type;
} named_type_t;

typedef struct type_decl_t {
	str_t name;
	type_t type;
} type_decl_t;

MAKE_ARRAY(type_decl);

typedef struct ast_t {
	type_decl_array_t types;
} ast_t;

int compass_main() {
	arena_t arena = arena_new();
	u8_array_t file = file_read(&arena, str_lit(&arena, "test.tog"));
	str_t code = str_from_ascii(&arena, &file);
	str_t code_original = code;

	usize numTokens = 0;
	while (code.size != 0) {
		tokenize(&code);
		numTokens++;
	}

	token_array_t tokens = token_array_new(&arena, numTokens, sizeof(token_t));
	code = code_original;
	
	for (usize i = 0; code.size != 0; i++) {
		tokens.data[i] = tokenize(&code);
		token_print(&tokens.data[i]);
	}

	type_value_t type_value = type_value_parse(&arena, &tokens);
	type_value_print(&type_value);

	arena_destroy(&arena);
	return 0;
}
