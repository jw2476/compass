#include "token.h"

#include <stdlib.h>
#include <stdio.h>

void literal_print(literal_t* literal) {
	if (literal->type == LITERAL_STRING) {
		printf("\"");
		str_print(&literal->data.string);
		printf("\"");
	} else if (literal->type == LITERAL_INTEGER) {
		printf("%lld", literal->data.integer);
	} else {
		printf("%f", literal->data.decimal);
	}
}

const char* BRACKETS[] = { "(", "[", "{", "<", ")", "]", "}", ">" };
void bracket_print(bracket_t* bracket) {
	u8 n = bracket->kind | (bracket->side << 2);
	printf("%s", BRACKETS[n]);
	
}

char bracket_as_char(bracket_t* bracket) {
	u8 n = bracket->kind | (bracket->side << 2);
	return *BRACKETS[n];
}

void token_print(token_t* token) {
	static const char* TOKEN_TYPES[] = { "IDENT", "LITERAL", "BRACKET", "DCOLON", "SCOLON", "EQ", "DOLLAR", "ARROW", "PIPE", "EOF" };
	
	printf("%s ", TOKEN_TYPES[token->type]);
	if (token->type == TOKEN_IDENTIFIER) {
		str_print(&token->data.identifier);
	} else if (token->type == TOKEN_LITERAL) {
		literal_print(&token->data.literal);
	} else if (token->type == TOKEN_BRACKET) {
		bracket_print(&token->data.bracket);
	}
	printf("\n");
}

char to_lower(char c) {
	return c | 0b00100000;
}

bool is_alphabetic(char c) {
	return 'a' <= to_lower(c) && to_lower(c) <= 'z'; 
}

bool is_digit(char c) {
	return '0' <= c && c <= '9';
}

bool is_alphanumeric(char c) {
	return is_alphabetic(c) || is_digit(c);
}

token_t tokenize(str_t* code) {
	token_t token;
	if (code->size == 0) { token.type = TOKEN_EOF; return token; }

	arena_t arena = arena_new();
	
	if (str_starts_with(code, str_lit(&arena, "//"))) {
		usize i;
		for (i = 0; code->data[i] != '\n'; i++) {}
		str_skip_chars(code, i);
		token = tokenize(code);
	} else if (str_starts_with(code, str_lit(&arena, "::"))) {
		str_skip_chars(code, 2);
		token.type = TOKEN_DOUBLE_COLON;
	} else if (str_starts_with(code, str_lit(&arena, "\n"))) {
		str_skip_chars(code, 1);
		token.type = TOKEN_NEWLINE;
	} else if (str_starts_with(code, str_lit(&arena, ":"))) {
		str_skip_chars(code, 1);
		token.type = TOKEN_SINGLE_COLON;
	} else if (str_starts_with(code, str_lit(&arena, "="))) { 
		str_skip_chars(code, 1);
		token.type = TOKEN_EQUALS;
	} else if (str_starts_with(code, str_lit(&arena, "$"))) {
		str_skip_chars(code, 1);
		token.type = TOKEN_DOLLAR;
	} else if (str_starts_with(code, str_lit(&arena, "->"))) {
		str_skip_chars(code, 2);
		token.type = TOKEN_ARROW;
	} else if (str_starts_with(code, str_lit(&arena, "|"))) {
		str_skip_chars(code, 1);
		token.type = TOKEN_PIPE;
	} else if (str_starts_with(code, str_lit(&arena, "\""))) {
		str_skip_chars(code, 1);
		usize i;
		for (i = 0; code->data[i] != '"'; i++) {}

		token.type = TOKEN_LITERAL;
		token.data.literal.type = LITERAL_STRING;

		token.data.literal.data.string = str_view(code, 0, i);
		str_skip_chars(code, i + 1);
	} else if (is_alphabetic(code->data[0])) {
		usize i;
		for (i = 0; is_alphanumeric(code->data[i]) || code->data[i] == '_'; i++) {}
		token.type = TOKEN_IDENTIFIER;
		token.data.identifier = str_view(code, 0, i);

		str_skip_chars(code, i);
	} else if (is_digit(code->data[0])) {
		bool isDecimal = false;
		usize i;
		for (i = 0; is_digit(code->data[i]) || code->data[i] == '.'; i++) {
			if (code->data[i] == '.') isDecimal = true;
		}

		str_t str = str_view(code, 0, i);
		str_skip_chars(code, i);
		if (!isDecimal) {
			token.data.literal.type = LITERAL_INTEGER;
			token.data.literal.data.integer = strtoll(str.data, NULL, 10);
		} else {
			token.data.literal.type = LITERAL_DECIMAL;
			token.data.literal.data.decimal = strtod(str.data, NULL);
		}

		token.type = TOKEN_LITERAL;
	} else {
		bool bracket = false;
		for (usize i = 0; i < 8; i++) {
			if (code->data[0] == *BRACKETS[i]) {
				token.data.bracket.kind = i & 0b11;
				token.data.bracket.side = (i & 0b100) >> 2;
				token.type = TOKEN_BRACKET;
				str_skip_chars(code, 1);
				bracket = true;
				break;
			}
		}

		if (!bracket) {
			str_skip_chars(code, 1);
			token = tokenize(code);
		}
	}

	arena_destroy(&arena);
	return token;
}
