#include <ctype.h>
#include <stdio.h>

#include "lexer.h"

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Error: please supply an input file\n");
		return -1;
	}

	const char *const file_name = argv[1];
	FILE *source_file = fopen(file_name, "r");

	if (!source_file)
	{
		printf("Error: please supply a valid file name\n");
		return -1;
	}

	// get file size
	fseek(source_file, 0L, SEEK_END);
	long file_size = ftell(source_file);
	fseek(source_file, 0L, SEEK_SET);

	if (file_size <= 0)
	{
		printf("Error: file is either too large or empty\n");
		return -1;
	}

	// +1 for the NULL char
	CharBuffer *cb = alloc_char_buffer(file_size + 1);

	// -1 to not overwrite the NULL char
	cb->_size = fread(cb->_buf, sizeof(char), cb->_max_size - 1, source_file);
	fclose(source_file);

	TokenData *tok_data = tokenize(cb);
	if (!tok_data)
	{
		return -1;
	}

	// debug info
	printf("\n\nEmitted tokens:\n");
	for (int i = 0; i < tok_data->_tok_idx; i++)
	{
		printf("%d ", tok_data->tokens[i]);
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

	free_token_data(tok_data);

	delete_char_buffer(cb);

	return 0;
}
