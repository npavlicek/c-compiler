#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

typedef struct CharBuffer
{
	char *_buf;
	int _cur_idx;

	int _max_size;
	int _size;

	// end-of-buffer
	int eob;

	char cur_char;
	char next_char;
} CharBuffer;

CharBuffer *make_char_buffer(int max_size)
{
	CharBuffer *res = calloc(1, sizeof(CharBuffer));
	res->_buf = calloc(max_size, sizeof(char));
	res->_cur_idx = -1;
	res->_max_size = max_size;
	res->eob = 0;
	return res;
}

void delete_char_buffer(CharBuffer *cb)
{
	free(cb->_buf);
	free(cb);
}

int cb_next(CharBuffer *cb)
{
	if (cb->eob)
		return 0;

	cb->_cur_idx++;

	if (cb->_cur_idx >= cb->_size || cb->_cur_idx >= cb->_max_size)
	{
		cb->eob = 1;
		return 0;
	}

	cb->cur_char = cb->_buf[cb->_cur_idx];

	if (cb->_cur_idx + 1 < cb->_size)
	{
		cb->next_char = cb->_buf[cb->_cur_idx + 1];
	}
	else
	{
		cb->next_char = 0;
	}

	return 1;
}

// Returns -1 if theres no match, otherwise the token
int is_keyword(const char *const str)
{
	for (int i = 0; i < sizeof(keywords_str) / sizeof(char *); i++)
	{
		if (strcmp(str, keywords_str[i]) == 0)
		{
			return keywords_tok[i];
		}
	}
	return -1;
}

// return -1 if no match
int is_punctuator(const CharBuffer *cb)
{
	char f = cb->cur_char;
	char s = cb->next_char;

	// double char tokens
	if (f == '-' && s == '>')
		return TOK_RIGHT_ARROW;
	if (f == '+' && s == '+')
		return TOK_INCREMENT;
	if (f == '-' && s == '-')
		return TOK_DECREMENT;
	if (f == '<' && s == '<')
		return TOK_BIT_SHIFT_LEFT;
	if (f == '>' && s == '>')
		return TOK_BIT_SHIFT_RIGHT;
	if (f == '<' && s == '=')
		return TOK_LSS_EQL;
	if (f == '>' && s == '=')
		return TOK_GTR_EQL;
	if (f == '=' && s == '=')
		return TOK_EQUALITY;
	if (f == '!' && s == '=')
		return TOK_EQUALITY_NOT;
	if (f == '&' && s == '&')
		return TOK_AND;
	if (f == '|' && s == '|')
		return TOK_OR;

	// single char tokens
	for (int i = 0; i < sizeof(single_punctuator_char) / sizeof(char); i++)
	{
		if (f == single_punctuator_char[i])
		{
			return single_punctuator_tok[i];
		}
	}

	return -1;
}

int tokenize(CharBuffer *cb)
{
	int comment_line_mode = 0;
	int comment_block_mode = 0;

	while (cb_next(cb))
	{
		if (comment_line_mode)
		{
			if (cb->cur_char == '\n')
				comment_line_mode = 0;
			continue;
		}

		if (comment_block_mode)
		{
			if (cb->cur_char == '*' && cb->next_char == '/')
			{
				comment_block_mode = 0;
				// To make sure we shift an additional char
				cb_next(cb);
			}
			continue;
		}

		// skip white space and new lines
		if (isspace(cb->cur_char))
			continue;

		// parse out comments
		if (cb->cur_char == '/' && cb->next_char == '/')
		{
			comment_line_mode = 1;
			continue;
		}

		if (cb->cur_char == '/' && cb->next_char == '*')
		{
			comment_block_mode = 1;
			// This is in order to avoid allowing this type of comment: /*/
			cb_next(cb);
			continue;
		}

		printf("%c", cb->cur_char);
	}

	return 0;
}

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

	tokenize(cb);

	delete_char_buffer(cb);

	return 0;
}
