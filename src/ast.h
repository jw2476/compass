#pragma once

#include "token.h"
#include "base.h"

typedef struct type_t type_t;
typedef struct named_type_t named_type_t;

MAKE_ARRAY(type);
MAKE_ARRAY(named_type);

typedef struct type_value_t {
	str_t name;
	type_array_t generics;
} type_value_t;
MAKE_ARRAY(type_value);


typedef struct type_function_t {
	type_t* input;
	type_t* output;
} type_function_t;

typedef enum type_type_t {
	TYPE_VALUE,
	TYPE_TUPLE,
	TYPE_STRUCT,
	TYPE_ENUM,
	TYPE_FUNCTION
} type_type_t;

typedef union type_data_t {
	type_value_t value;
	type_array_t tuple;
	named_type_array_t _struct;
	named_type_array_t _enum;
	type_function_t function;
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
	str_array_t generics;
	type_value_array_t conditions;
	type_t type;
} type_decl_t;

typedef token_array_t expr_t;
MAKE_ARRAY(expr);
typedef expr_array_t function_body_t;

typedef struct function_decl_t {
	str_t name;
	bool isPatternMatched;
	str_t pattern;
	function_body_t body;
} function_decl_t;

MAKE_ARRAY(type_decl);
MAKE_ARRAY(function_decl);

typedef struct ast_t {
	type_decl_array_t types;
	function_decl_array_t functions;
} ast_t;

void ast_print(ast_t* ast);
ast_t ast_parse(arena_t* arena, token_array_t* tokens);

