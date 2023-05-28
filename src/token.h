#pragma once

#include "base.h"

typedef enum literal_type_t {
	LITERAL_STRING,
	LITERAL_INTEGER,
	LITERAL_DECIMAL
} literal_type_t;

typedef union literal_data_t {
	str_t string;
	i64 integer;
	f64 decimal;
} literal_data_t;

typedef struct literal_t {
	literal_type_t type;
	literal_data_t data;
} literal_t;

void literal_print(literal_t* literal);

typedef enum bracket_kind_t {
	BRACKET_ROUND = 0,
	BRACKET_SQUARE,
	BRACKET_CURLY,
	BRACKET_ANGLE
} bracket_kind_t;

typedef enum bracket_side_t {
	BRACKET_OPEN = 0,
	BRACKET_CLOSE
} bracket_side_t;

typedef struct bracket_t {
	bracket_kind_t kind;
	bracket_side_t side;
} bracket_t;

void bracket_print(bracket_t* bracket);
char bracket_as_char(bracket_t* bracket);

typedef enum token_type_t {
	TOKEN_IDENTIFIER,
	TOKEN_LITERAL,
	TOKEN_BRACKET,
	TOKEN_DOUBLE_COLON,
	TOKEN_SINGLE_COLON,
	TOKEN_EQUALS,
	TOKEN_DOLLAR,
	TOKEN_ARROW,
	TOKEN_PIPE,
	TOKEN_NEWLINE,
	TOKEN_EOF
} token_type_t;

typedef union token_data_t {
	str_t identifier;
	literal_t literal;
	bracket_t bracket;
} token_data_t;

typedef struct token_t {
	token_type_t type;
	token_data_t data;
} token_t;

MAKE_ARRAY(token);

token_t tokenize(str_t* code);
void token_print(token_t* token);
