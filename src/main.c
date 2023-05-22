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

	if (type_value->generics.size == 0) return;
	printf("<");
	for (usize i = 0; i < type_value->generics.size; i++) {
		str_print(&type_value->generics.data[i]);
		if ((i + 1) != type_value->generics.size) printf(", ");
	}
	printf(">\n");
}

type_value_t type_value_parse(arena_t* arena, token_array_t* tokens) {
	if (tokens->data[0].type != TOKEN_IDENTIFIER) {
		CRITICAL("First token of type value isn't identifier");
	}

	type_value_t type_value;
	type_value.name = tokens->data[0].data.identifier;
	token_array_skip(tokens, 1);

	bool hasGenerics = tokens->data[0].type == TOKEN_BRACKET && bracket_as_char(&tokens->data[0].data.bracket) == '<';
	if (!hasGenerics) {
		type_value.generics = str_array_new(arena, 0, sizeof(str_t));
		return type_value;
	}

	token_array_skip(tokens, 1);

	usize i;
	for (i = 0; !(tokens->data[i].type == TOKEN_BRACKET && bracket_as_char(&tokens->data[i].data.bracket) == '>'); i++) {}

	type_value.generics = str_array_new(arena, i, sizeof(str_t));
	for (usize j = 0; j < i; j++) {
		type_value.generics.data[j] = tokens->data[j].data.identifier;
	}

	token_array_skip(tokens, i);

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

void type_print(type_t* type);

void named_type_print(named_type_t* named_type) {
	str_print(&named_type->name);
	printf(": ");
	type_print(&named_type->type);
}

void type_print(type_t* type) {
	printf("At least got here\n");
	if (type->type == TYPE_VALUE) {
		printf("Printing value\n");
		type_value_print(&type->data.value);
	} else if (type->type == TYPE_TUPLE) {
		printf("(");
		for (usize i = 0; i < type->data.tuple.size; i++) {
			type_print(&type->data.tuple.data[i]);
			if ((i+1) != type->data.tuple.size) printf(", ");
		}
		printf(")\n");
	} else if (type->type == TYPE_STRUCT) {
		printf("{");
		for (usize i = 0; i < type->data._struct.size; i++) {
			named_type_print(&type->data._struct.data[i]);
			if ((i+1) != type->data._struct.size) printf(", ");
		}
		printf("}\n");
	} else if (type->type == TYPE_ENUM) {
		printf("[");
		for (usize i = 0; i < type->data._enum.size; i++) {
			named_type_print(&type->data._enum.data[i]);
			if ((i+1) != type->data._enum.size) printf(", ");
		}
		printf("]\n");
	} else {
		CRITICAL("Trying to print invalid type");
	}
}

type_t type_make_unit(arena_t* arena) {
	type_t type;
	type.type = TYPE_TUPLE;
	type.data.tuple = type_array_new(arena, 0, sizeof(type_t));
	return type;
}

type_t type_parse(arena_t* arena, token_array_t* tokens);

type_t type_parse_tuple(arena_t* arena, token_array_t* tokens) {
	printf("Parsing tuple\n");
	token_array_skip(tokens, 1); // Skip '('
	type_array_t types = type_array_new(arena, 128, sizeof(type_t)); // Assume there will never be a tuple with more than 128 values
	usize i = 0;
	while (tokens->data[0].type != TOKEN_BRACKET && bracket_as_char(&tokens->data[0].data.bracket) != ')') {
		types.data[i++] = type_parse(arena, tokens);
	}
	token_array_skip(tokens, 1); // Skip ')'

	type_t type;
	type.type = TYPE_TUPLE;
	type.data.tuple = type_array_view(&types, 0, i);
	return type;
}

type_t type_parse_struct(arena_t* arena, token_array_t* tokens) {
	printf("Parsing struct\n");
	token_array_skip(tokens, 1); // Skip '{'
	named_type_array_t types = named_type_array_new(arena, 128, sizeof(named_type_t)); // Assume there will never be a struct with more that 128 values
	usize i = 0;
	while (tokens->data[0].type != TOKEN_BRACKET && bracket_as_char(&tokens->data[0].data.bracket) != '}') {
		types.data[i].name = tokens->data[0].data.identifier;
		if (tokens->data[1].type != TOKEN_SINGLE_COLON) {
			CRITICAL("Should be a single colon after each identifier in a struct");
		}
		token_array_skip(tokens, 1); // Skip '}'
		types.data[i].type = type_parse(arena, tokens);
		
		i++;
	}
	token_array_skip(tokens, 1); // Skip '}'
	
	type_t type;
	type.type = TYPE_STRUCT;
	type.data._struct = named_type_array_view(&types, 0, i);
	return type;
}

type_t type_parse_enum(arena_t* arena, token_array_t* tokens) {
	printf("Parsing enum\n");
	token_array_skip(tokens, 1); // Skip '['
	named_type_array_t variants = named_type_array_new(arena, 128, sizeof(named_type_t)); // Assume there will never be a struct with more than 128 values
	usize numVariants = 0;
	usize i = 0;
	
	named_type_t staging;

	while (true) {
		token_print(&tokens->data[0]);
		if (tokens->data[0].type == TOKEN_BRACKET && bracket_as_char(&tokens->data[0].data.bracket) == ']') {
			variants.data[i++] = staging;
			break;
		} else if (tokens->data[0].type == TOKEN_PIPE) {
			printf("Staging name after: ");
			str_print(&staging.name);
			variants.data[i++] = staging;
			staging.type = type_make_unit(arena);
			token_array_skip(tokens, 1);
		} else if (tokens->data[0].type == TOKEN_IDENTIFIER) {
			staging.name = tokens->data[0].data.identifier;
			printf("Staging name before: ");
			str_print(&staging.name);
			token_array_skip(tokens, 1);
			
			if (tokens->data[0].type != TOKEN_PIPE) {
				staging.type = type_parse(arena, tokens);  
			}
		} else {
			CRITICAL("Invalid token in enum");
		}
	}
	token_array_skip(tokens, 1); // Skip ']'

	type_t type;
	type.type = TYPE_ENUM;
	type.data._enum = named_type_array_view(&variants, 0, i);
	return type;
}

// TODO: Refactor this once I implement linked lists
type_t type_parse(arena_t* arena, token_array_t* tokens) {
	type_t type;
	
	if (tokens->data[0].type == TOKEN_IDENTIFIER) {
		type.type = TYPE_VALUE;
		type.data.value = type_value_parse(arena, tokens);
	} else if (tokens->data[0].type == TOKEN_BRACKET) {
		if (bracket_as_char(&tokens->data[0].data.bracket) == '(') {
			type = type_parse_tuple(arena, tokens);
		} else if (bracket_as_char(&tokens->data[0].data.bracket) == '{') {
			type = type_parse_struct(arena, tokens);
		} else if (bracket_as_char(&tokens->data[0].data.bracket) == '[') {
			type = type_parse_enum(arena, tokens);
		}
	} else {
		ERROR("Invalid token for type:");
		token_print(&tokens->data[0]);
		assert(false);
	}

	return type;
}

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
	}

	type_t type = type_parse(&arena, &tokens);
	printf("Parsed\n");
	type_print(&type);

	arena_destroy(&arena);
	return 0;
}
