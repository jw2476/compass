#include <stdio.h>
#include <stdlib.h>

#include "base.h"
#include "token.h"	
#include "ast.h"

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

	ast_t ast = ast_parse(&arena, &tokens);
	ast_print(&ast);

	arena_destroy(&arena);
	return 0;
}
