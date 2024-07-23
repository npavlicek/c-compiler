#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Error: please supply an input file\n");
		return -1;
	}

	CharBuffer *cb = make_char_buffer(5000);

	const char *const file_name = argv[1];
	FILE *source_file = fopen(file_name, "r");
	cb->_size = fread(cb->_buf, sizeof(char), cb->_max_size - 1, source_file);
	fclose(source_file);

	TokenData *tok_data = tokenize(cb);
	if (!tok_data)
	{
		return -1;
	}

	free_token_data(tok_data);

	delete_char_buffer(cb);

	return 0;
}
