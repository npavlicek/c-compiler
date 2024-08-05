#include "parser.h"

#ifndef NDEBUG
#include <signal.h>
#endif

#include <stdio.h>
#include <stdlib.h>

static TokenData *token_data_internal;
static LLVMModuleRef llvm_module_internal;
static int current_token = 0;

/**
 * Will print error and exit
 */
static void print_error(const char *msg)
{
	printf("[Line %d] Error: %s\n", token_data_internal->line_numbers[current_token], msg);

	// Only execute if compiling in debug
#ifndef NDEBUG
	raise(SIGTRAP);
#endif

	exit(EXIT_FAILURE);
}

static void check_token_idx_bounds()
{
	if (current_token < 0 || current_token >= token_data_internal->_buf_max_size)
	{
		printf("FATAL ERROR: current token index is out of bounds!\n");
#ifndef NDEBUG
		raise(SIGTRAP);
#endif
		exit(EXIT_FAILURE);
	}
}

static Token peek_token()
{
	check_token_idx_bounds();

	return token_data_internal->tokens[current_token];
}

static Token get_token()
{
	check_token_idx_bounds();

	return token_data_internal->tokens[current_token++];
}

static void put_token()
{
	current_token--;
}

static struct
{
	Token storage_class;
} decleration;

static void storage_class_specifier()
{
}

static void type_specifier()
{
}

static void type_qualifier()
{
}

static void function_specifier()
{
}

static void decl_specifiers()
{
	storage_class_specifier();
	type_specifier();
	type_qualifier();
	function_specifier();
	// TODO: alignment specifier?
}

static void top_level_decl()
{
	// while we havent reached 
	while (peek_token() != TOK_SEMI_COLON || peek_token() != TOK_COMMA || peek_token() != TOK_IDENTIFIER)
	{
		decl_specifiers();
	}

	// TODO: OPT init declerator list
	// TODO: emit the decleration

	if (get_token() != TOK_SEMI_COLON)
	{
		print_error("declerations must end with a semicolon");
	}
}

void parse(LLVMModuleRef llvm_module, TokenData *token_data)
{
	token_data_internal = token_data;
	llvm_module_internal = llvm_module;

	top_level_decl();
}
