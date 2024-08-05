#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <llvm-c/Core.h>

#include "debug_tokens.h"
#include "lexer.h"
#include "parser.h"

void print_debug_info(TokenData *tok_data)
{
	// debug info
	printf("Emitted tokens:\n");
	for (int i = 0; i < tok_data->_tok_idx; i++)
	{
		printf("%s ", debug_tokens[tok_data->tokens[i]]);
		if (tok_data->tokens[i] == TOK_IDENTIFIER || tok_data->tokens[i] == TOK_CHAR_LITERAL ||
		    tok_data->tokens[i] == TOK_NUMERICAL_CONSTANT || tok_data->tokens[i] == TOK_STRING_LITERAL)
		{
			i++;
		}
	}

	printf("\n\nEmitted string literals:\n");
	for (int i = 0; i < tok_data->_str_lit_idx; i++)
	{
		printf("\"%s\"\n", tok_data->string_literals[i]);
	}

	printf("\n\nEmitted char literals:\n");
	for (int i = 0; i < tok_data->_tok_idx; i++)
	{
		if (tok_data->tokens[i] == TOK_CHAR_LITERAL)
		{
			if (isgraph(tok_data->tokens[i + 1]))
			{

				printf("'%c'\n", tok_data->tokens[i + 1]);
			}
			else
			{
				printf("%d\n", tok_data->tokens[i + 1]);
			}
			i++;
		}
	}

	printf("\n\nEmitted identifiers:\n");
	for (int i = 0; i < tok_data->_tok_idx; i++)
	{
		if (tok_data->tokens[i] == TOK_IDENTIFIER)
		{
			printf("%s\n", tok_data->identifiers[tok_data->tokens[i + 1]]);
			i++;
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Error: please supply an input file\n");
		return EXIT_FAILURE;
	}

	const char *const file_name = argv[1];
	FILE *source_file = fopen(file_name, "r");

	if (!source_file)
	{
		printf("Error: please supply a valid file name\n");
		return EXIT_FAILURE;
	}

	// get file size
	fseek(source_file, 0L, SEEK_END);
	long file_size = ftell(source_file);
	fseek(source_file, 0L, SEEK_SET);

	if (file_size <= 0)
	{
		printf("Error: file is either too large or empty\n");
		return EXIT_FAILURE;
	}

	// +1 for the NULL char
	CharBuffer *cb = alloc_char_buffer(file_size + 1);

	// -1 to not overwrite the NULL char
	cb->_size = fread(cb->_buf, sizeof(char), cb->_max_size - 1, source_file);
	fclose(source_file);

	TokenData *tok_data = tokenize(cb);
	if (!tok_data)
	{
		return EXIT_FAILURE;
	}

	printf("# tokens: %d\n", tok_data->_tok_idx);
	printf("# string literals: %d\n", tok_data->_str_lit_idx);
	printf("# identifiers: %d\n", tok_data->_ident_idx);
	printf("# num constants: %d\n", tok_data->_num_const_idx);

	unsigned ma, mi, pa;
	LLVMGetVersion(&ma, &mi, &pa);
	printf("\nUsing LLVM version: %d.%d.%d\n", ma, mi, pa);

	char *module_name = calloc(strlen(file_name) + 1, sizeof(char));
	strcpy(module_name, file_name);

	module_name = strtok(module_name, ".");

	LLVMModuleRef module = LLVMModuleCreateWithName(module_name);
	LLVMSetSourceFileName(module, file_name, strlen(file_name));

	free(module_name);

	size_t module_id_len;
	const char *module_id = LLVMGetModuleIdentifier(module, &module_id_len);

	size_t module_source_file_name_len;
	const char *module_source_file_name = LLVMGetSourceFileName(module, &module_source_file_name_len);

	printf("LLVM Module Name: %s\n", module_id);
	printf("LLVM Source File Name: %s\n", module_source_file_name);

	ParserErrorCode err = parse(module, tok_data);
	if (err != PARSER_NO_ERROR)
	{
		printf("Parser encountered a %s error! terminating...\n", ParserErrorStrings[err]);

		return EXIT_FAILURE;
	}

	LLVMDisposeModule(module);

	free_token_data(tok_data);
	delete_char_buffer(cb);

	LLVMShutdown();

	return EXIT_SUCCESS;
}
