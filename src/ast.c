#include "ast.h"

void type_print(type_t* type);
type_t type_parse(arena_t* arena, token_array_t* tokens, bool canBeFunction);

void type_value_print(type_value_t* type_value) {
	str_print(&type_value->name);

	if (type_value->generics.size == 0) return;
	printf("<");
	for (usize i = 0; i < type_value->generics.size; i++) {
		type_print(&type_value->generics.data[i]);
		if ((i + 1) != type_value->generics.size) printf(", ");
	}
	printf(">");
}

type_value_t type_value_parse(arena_t* arena, token_array_t* tokens) {
	if (tokens->data[0].type != TOKEN_IDENTIFIER) {
		token_print(&tokens->data[0]);
		CRITICAL("^ First token of type value isn't identifier");
	}

	type_value_t type_value;
	type_value.name = tokens->data[0].data.identifier;
	token_array_skip(tokens, 1); // Skip IDENT

	bool hasGenerics = tokens->data[0].type == TOKEN_BRACKET && bracket_as_char(&tokens->data[0].data.bracket) == '<';
	if (!hasGenerics) {
		type_value.generics = type_array_new(arena, 0, sizeof(type_t));
		return type_value;
	}

	token_array_skip(tokens, 1); // Skip '>'

	type_value.generics = type_array_new(arena, 128, sizeof(type_t));
	usize i = 0;
	while (!(tokens->data[0].type == TOKEN_BRACKET && bracket_as_char(&tokens->data[0].data.bracket) == '>')) {
		type_value.generics.data[i] = type_parse(arena, tokens, true);
		i++;
	}

	token_array_skip(tokens, 1); // Skip '>'

	type_value.generics = type_array_view(&type_value.generics, 0, i);
	return type_value;
}

void type_function_print(type_function_t* function_type) {
	type_print(function_type->input);
	printf(" -> ");
	type_print(function_type->output);
}

type_function_t type_function_parse(arena_t* arena, token_array_t* tokens) {
	type_t input = type_parse(arena, tokens, false);
	if (tokens->data[0].type != TOKEN_ARROW) {
		CRITICAL("All function types should have '->' between their input and output types");
	}
	token_array_skip(tokens, 1); // Skip '->'
	type_t output = type_parse(arena, tokens, false);

	type_function_t function_type;
	function_type.input = arena_push(arena, sizeof(type_t));
	function_type.output = arena_push(arena, sizeof(type_t));

	memcpy(function_type.input, &input, sizeof(type_t));
	memcpy(function_type.output, &output, sizeof(type_t));

	return function_type;
}

void named_type_print(named_type_t* named_type, str_t seperator) {
	str_print(&named_type->name);
	str_print(&seperator);
	type_print(&named_type->type);
}

void type_print(type_t* type) {
	arena_t scratch = arena_new();

	if (type->type == TYPE_VALUE) {
		type_value_print(&type->data.value);
	} else if (type->type == TYPE_TUPLE) {
		printf("(");
		for (usize i = 0; i < type->data.tuple.size; i++) {
			type_print(&type->data.tuple.data[i]);
			if ((i+1) != type->data.tuple.size) printf(", ");
		}
		printf(")");
	} else if (type->type == TYPE_STRUCT) {
		printf("{");
		for (usize i = 0; i < type->data._struct.size; i++) {
			named_type_print(&type->data._struct.data[i], str_lit(&scratch, ": "));
			if ((i+1) != type->data._struct.size) printf(", ");
		}
		printf("}");
	} else if (type->type == TYPE_ENUM) {
		printf("[");
		for (usize i = 0; i < type->data._enum.size; i++) {
			named_type_print(&type->data._enum.data[i], str_lit(&scratch, " "));
			if ((i+1) != type->data._enum.size) printf(" | ");
		}
		printf("]");
	} else if (type->type == TYPE_FUNCTION) {
		type_function_print(&type->data.function);
	} else {
		CRITICAL("Trying to print invalid type");
	}

	arena_destroy(&scratch);
}

type_t type_make_unit(arena_t* arena) {
	type_t type;
	type.type = TYPE_TUPLE;
	type.data.tuple = type_array_new(arena, 0, sizeof(type_t));
	return type;
}

type_t type_parse_tuple(arena_t* arena, token_array_t* tokens) {
	token_array_skip(tokens, 1); // Skip '('
	type_array_t types = type_array_new(arena, 128, sizeof(type_t)); // Assume there will never be a tuple with more than 128 values
	usize i = 0;
	while (!(tokens->data[0].type == TOKEN_BRACKET && bracket_as_char(&tokens->data[0].data.bracket) == ')')) {
		types.data[i++] = type_parse(arena, tokens, true);
	}
	token_array_skip(tokens, 1); // Skip ')'

	type_t type;
	type.type = TYPE_TUPLE;
	type.data.tuple = type_array_view(&types, 0, i);
	return type;
}

type_t type_parse_struct(arena_t* arena, token_array_t* tokens) {
	token_array_skip(tokens, 1); // Skip '{'
	named_type_array_t types = named_type_array_new(arena, 128, sizeof(named_type_t)); // Assume there will never be a struct with more that 128 values
	usize i = 0;
	while (!(tokens->data[0].type == TOKEN_BRACKET && bracket_as_char(&tokens->data[0].data.bracket) == '}')) {
		types.data[i].name = tokens->data[0].data.identifier;
		if (tokens->data[1].type != TOKEN_SINGLE_COLON) {
			CRITICAL("Should be a single colon after each identifier in a struct");
		}
		token_array_skip(tokens, 2); // Skip 'IDENT:'
		types.data[i].type = type_parse(arena, tokens, true);
		
		i++;
	}
	token_array_skip(tokens, 1); // Skip '}'
	
	type_t type;
	type.type = TYPE_STRUCT;
	type.data._struct = named_type_array_view(&types, 0, i);
	return type;
}

type_t type_parse_enum(arena_t* arena, token_array_t* tokens) {
	token_array_skip(tokens, 1); // Skip '['
	named_type_array_t variants = named_type_array_new(arena, 128, sizeof(named_type_t)); // Assume there will never be a struct with more than 128 values
	usize numVariants = 0;
	usize i = 0;

	while (true) {
		if (tokens->data[0].type == TOKEN_IDENTIFIER) {
			named_type_t staging;
			staging.name = tokens->data[0].data.identifier;

			token_array_skip(tokens, 1);
			
			if (tokens->data[0].type == TOKEN_PIPE) {
				staging.type = type_make_unit(arena);
				variants.data[i++] = staging;
			} else if (tokens->data[0].type == TOKEN_BRACKET && bracket_as_char(&tokens->data[0].data.bracket) == ']') {
				staging.type = type_make_unit(arena);
				variants.data[i++] = staging;
				break;
			} else {
				staging.type = type_parse(arena, tokens, true);
				variants.data[i++] = staging;
				if (tokens->data[0].type == TOKEN_BRACKET && bracket_as_char(&tokens->data[0].data.bracket) == ']') break;
			}
		} else {
			CRITICAL("Incorrectly formatted enum, it should be [VARIANT TYPE | VARIANT TYPE] e.g. [Some T | None]");
		}
	}
	token_array_skip(tokens, 1); // Skip ']'
	
	type_t type;
	type.type = TYPE_ENUM;
	type.data._enum = named_type_array_view(&variants, 0, i);
	return type;
}

type_t type_parse(arena_t* arena, token_array_t* tokens, bool canBeFunction) {
	type_t type;
	
	bool isFunction = false;

	if (canBeFunction) {
		token_array_t tokens_copy = *tokens;
		type_parse(arena, tokens, false);
		isFunction = tokens->data[0].type == TOKEN_ARROW;
		*tokens = tokens_copy;
	}
	
	if (isFunction) {
		type.type = TYPE_FUNCTION;
		type.data.function = type_function_parse(arena, tokens);
	} else if (tokens->data[0].type == TOKEN_IDENTIFIER) {
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

void type_decl_print(type_decl_t* type_decl) {
	str_print(&type_decl->name);
	if (type_decl->generics.size) {
		printf("<");
		for (usize i = 0; i < type_decl->generics.size; i++) {
			str_print(&type_decl->generics.data[i]);
			if ((i+1) != type_decl->generics.size) printf(", ");
		}
		
		if (type_decl->conditions.size) {
			printf(" : ");
			for (usize i = 0; i < type_decl->conditions.size; i++) {
				type_value_print(&type_decl->conditions.data[i]);
				if ((i+1) != type_decl->conditions.size) printf(", ");
			}
		}
		
		printf(">");
	}
	printf(" :: ");
	type_print(&type_decl->type);
}

void type_decl_parse_generics(arena_t* arena, token_array_t* tokens, type_decl_t* type_decl) {
	token_array_skip(tokens, 1); // Skip '<'

	type_decl->generics = str_array_new(arena, 128, sizeof(str_t));
	type_decl->conditions = type_value_array_new(arena, 16, sizeof(type_value_t));

	usize numGenerics = 0;
	while (tokens->data[0].type != TOKEN_SINGLE_COLON && !(tokens->data[0].type == TOKEN_BRACKET && bracket_as_char(&tokens->data[0].data.bracket) == '>')) {
		type_decl->generics.data[numGenerics++] = tokens->data[0].data.identifier;
		token_array_skip(tokens, 1);
	}
	type_decl->generics = str_array_view(&type_decl->generics, 0, numGenerics);

	if (tokens->data[0].type == TOKEN_SINGLE_COLON) {
		token_array_skip(tokens, 1);
	}

	usize numConditions = 0;
	while (!(tokens->data[0].type == TOKEN_BRACKET && bracket_as_char(&tokens->data[0].data.bracket) == '>')) {
		type_decl->conditions.data[numConditions++] = type_value_parse(arena, tokens);
	}
	type_decl->conditions = type_value_array_view(&type_decl->conditions, 0, numConditions);

	token_array_skip(tokens, 1); // Skip '>'
}

type_decl_t type_decl_parse(arena_t* arena, token_array_t* tokens) {
	if (tokens->data[0].type != TOKEN_IDENTIFIER) {
		token_print(&tokens->data[0]);
		CRITICAL("^ Invalid token for type declaration, should be an identifier");
	}

	type_decl_t type_decl;
	type_decl.name = tokens->data[0].data.identifier;
	token_array_skip(tokens, 1); // Skip IDENT
	
	if (tokens->data[0].type == TOKEN_BRACKET && bracket_as_char(&tokens->data[0].data.bracket) == '<') {
		type_decl_parse_generics(arena, tokens, &type_decl);
	} else {
		type_decl.generics = str_array_new(arena, 0, sizeof(str_t));
		type_decl.conditions = type_value_array_new(arena, 0, sizeof(type_value_t));
	}

	if (tokens->data[0].type != TOKEN_DOUBLE_COLON) {
		CRITICAL("^ Invalid token for type declaration, should be ::");
	}
	token_array_skip(tokens, 1); // Skip ::
	
	type_decl.type = type_parse(arena, tokens, true);
	return type_decl;
}


void expr_print(expr_t* expression) {
	for (usize i = 0; i < expression->size; i++) {
		if (expression->data[i].type == TOKEN_IDENTIFIER) {
			str_print(&expression->data[i].data.identifier);
		} else if (expression->data[i].type == TOKEN_DOLLAR) {
			printf("$");
		} else if (expression->data[i].type == TOKEN_LITERAL) {
			literal_print(&expression->data[i].data.literal);
		} else if (expression->data[i].type == TOKEN_BRACKET) {
			bracket_print(&expression->data[i].data.bracket);
		} else {
			token_print(&expression->data[i]);
			CRITICAL("^ Invalid token for expression");
		}
		
		bool isOpeningBracket = expression->data[i].type == TOKEN_BRACKET && expression->data[i].data.bracket.side == BRACKET_OPEN;
		bool nextIsClosingBracket = expression->data[i+1].type == TOKEN_BRACKET && expression->data[i+1].data.bracket.side == BRACKET_CLOSE;
		if (!isOpeningBracket && !nextIsClosingBracket) printf(" ");
	}
}

usize expr_get_size(token_array_t* tokens) {
	usize i = 0;
	while (true) {
		if (tokens->data[i].type == TOKEN_IDENTIFIER || tokens->data[i].type == TOKEN_DOLLAR || tokens->data[i].type == TOKEN_LITERAL) { 
			i++; 
		} else if (tokens->data[i].type == TOKEN_BRACKET && bracket_as_char(&tokens->data[i].data.bracket) == '(') {
			token_array_t subExpression = token_array_view(tokens, i+1, tokens->size - (i+1)); 
			i += 2 + expr_get_size(&subExpression);
		} else {
			return i;
		}
	}
}

expr_t expr_parse(token_array_t* tokens) {
	usize i = expr_get_size(tokens);
	expr_t expr = token_array_view(tokens, 0, i);
	token_array_skip(tokens, i);
	return expr;
}


void function_body_print(function_body_t* body) {
	expr_print(&body->data[0]);
	for (usize i = 1; i < body->size; i++) {
		printf("\n\t-> ");
		expr_print(&body->data[i]);
	}
}

function_body_t function_body_parse(arena_t* arena, token_array_t* tokens) {
	usize numExpressions = 1;
	for (usize i = 0; i < tokens->size; i++) {
		if (tokens->data[i].type == TOKEN_ARROW) numExpressions++;
		if (tokens->data[i].type == TOKEN_NEWLINE && tokens->data[i+1].type != TOKEN_ARROW) break;
	}

	function_body_t body = expr_array_new(arena, numExpressions, sizeof(expr_t));
	usize i = 0;
	while (true) {
		if (tokens->data[0].type == TOKEN_ARROW) { token_array_skip(tokens, 1); continue; }
		if (tokens->data[0].type == TOKEN_NEWLINE && tokens->data[1].type == TOKEN_ARROW) { token_array_skip(tokens, 1); continue; }
		if (tokens->data[0].type == TOKEN_NEWLINE && tokens->data[1].type != TOKEN_ARROW) break;

		body.data[i++] = expr_parse(tokens);
	}

	return body;
}

void function_decl_print(function_decl_t* function_decl) {
	str_print(&function_decl->name);
	if (function_decl->isPatternMatched) {
		printf(" ");
		str_print(&function_decl->pattern);
	}
	printf(" = ");
	function_body_print(&function_decl->body);
}

function_decl_t function_decl_parse(arena_t* arena, token_array_t* tokens) {
	if (tokens->data[0].type != TOKEN_IDENTIFIER) {
		token_print(&tokens->data[0]);
		CRITICAL("^ Invalid token for function decl, should be an identifier");
	}
	function_decl_t function_decl;
	function_decl.name = tokens->data[0].data.identifier;
	token_array_skip(tokens, 1);

	if (tokens->data[0].type == TOKEN_IDENTIFIER) {
		function_decl.isPatternMatched = true;
		function_decl.pattern = tokens->data[0].data.identifier;
		token_array_skip(tokens, 1);
	} else {
		function_decl.isPatternMatched = false;
	}

	if (tokens->data[0].type != TOKEN_EQUALS) {
		token_print(&tokens->data[0]);
		CRITICAL("^ Invalid token for function decl, should be '='");
	}
	token_array_skip(tokens, 1);

	function_decl.body = function_body_parse(arena, tokens);

	return function_decl;
}

void ast_print(ast_t* ast) {
	printf("Types:\n");
	for (usize i = 0; i < ast->types.size; i++) {
		type_decl_print(&ast->types.data[i]);
		printf("\n");
	}
	printf("Functions:\n");
	for (usize i = 0; i < ast->functions.size; i++) {
		function_decl_print(&ast->functions.data[i]);
		printf("\n");
	}
}

ast_t ast_parse(arena_t* arena, token_array_t* tokens) {
	usize numTypes = 0;
	usize numFunctions = 0;

	for (usize i = 0; i < tokens->size; i++) {
		if (tokens->data[i].type == TOKEN_DOUBLE_COLON) numTypes++;
		if (tokens->data[i].type == TOKEN_EQUALS) numFunctions++;
	}

	ast_t ast;
	ast.types = type_decl_array_new(arena, numTypes, sizeof(type_decl_t));
	ast.functions = function_decl_array_new(arena, numFunctions, sizeof(function_decl_t));

	usize nTypes = 0;
	usize nFunctions = 0;

	while (tokens->data[0].type != TOKEN_EOF && tokens->size != 0) {
		if (tokens->data[0].type == TOKEN_NEWLINE) { token_array_skip(tokens, 1); continue; }

		if (tokens->data[1].type == TOKEN_DOUBLE_COLON || (tokens->data[1].type == TOKEN_BRACKET && bracket_as_char(&tokens->data[1].data.bracket) == '<')) {
			ast.types.data[nTypes++] = type_decl_parse(arena, tokens);
			continue;
		} else if (tokens->data[1].type == TOKEN_EQUALS || tokens->data[2].type == TOKEN_EQUALS) {
			ast.functions.data[nFunctions++] = function_decl_parse(arena, tokens);
			continue;
		}
	}

	return ast;
}
