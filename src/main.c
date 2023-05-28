#include <stdio.h>
#include <stdlib.h>

#include "base.h"
#include "token.h"	
#include "ast.h"

bool ast_check_symbol_exists(ast_t* ast, str_t symbol, str_array_t* symbols) {
	static const char* PRIMITIVES[] = { "u8", "u16", "u32", "u64", "usize", "i8", "i16", "i32", "i64", "isize", "f32", "f64" };
	arena_t scratch = arena_new();
	
	for (usize i = 0; i < symbols->size; i++) {
		if (str_eq(symbol, symbols->data[i])) return true;
	}
	for (usize i = 0; i < ast->types.size; i++) {
		if (str_eq(symbol, ast->types.data[i].name)) return true;
	}
	for (usize i = 0; i < (sizeof(PRIMITIVES) / sizeof(*PRIMITIVES)); i++) {
		if (str_eq(symbol, str_lit(&scratch, PRIMITIVES[i]))) return true;
	}

	ERROR("Cannot find type symbol: %s", str_cstr(&scratch, &symbol));
	arena_destroy(&scratch);
	return false;
}


bool ast_check_type(ast_t* ast, type_t* type, str_array_t* symbols);
bool ast_check_type_value(ast_t* ast, type_value_t* type_value, str_array_t* symbols) {
	bool out = ast_check_symbol_exists(ast, type_value->name, symbols);
	for (usize i = 0; i < type_value->generics.size; i++) {
		out &= ast_check_type(ast, &type_value->generics.data[i], symbols);
	}
	return out;
}

bool ast_check_type(ast_t* ast, type_t* type, str_array_t* symbols) {
	if (type->type == TYPE_VALUE) {
		return ast_check_type_value(ast, &type->data.value, symbols);
	} else {
		return true;
	}
}

bool ast_check_type_decl(ast_t* ast, type_decl_t* type_decl) {
	return ast_check_type(ast, &type_decl->type, &type_decl->generics);
}

bool ast_check(ast_t* ast) {
	bool out = true;

	for (usize i = 0; i < ast->types.size; i++) {
		out &= ast_check_type_decl(ast, &ast->types.data[i]);
	}

	return out;
}

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
	ast_check(&ast);

	arena_destroy(&arena);
	return 0;
}
