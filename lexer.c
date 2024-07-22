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

int cb_back(CharBuffer *cb)
{
	if (cb->eob)
		return 0;

	if (cb->_cur_idx > 0)
	{
		cb->_cur_idx--;
		cb->cur_char = cb->_buf[cb->_cur_idx];
		cb->next_char = cb->_buf[cb->_cur_idx + 1];
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
int is_punctuator(CharBuffer *cb)
{
	char f = cb->cur_char;
	char s = cb->next_char;

	// In order to process the double char only once when we return to the tokenize function
	cb_next(cb);

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

	cb_back(cb);

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

typedef struct TokenData
{
	int _overflow;

	int _buf_max_size;
	int _str_max_size;

	int _tok_idx;
	int _ident_idx;
	int _str_lit_idx;
	int _int_const_idx;
	int _float_const_idx;

	int *tokens;

	char **identifiers;
	char **string_literals;

	int *integer_constants;
	float *floating_constants;
} TokenData;

TokenData *alloc_token_data(int buf_max_size, int str_max_size)
{
	TokenData *res = calloc(1, sizeof(TokenData));
	res->_buf_max_size = buf_max_size;
	res->_str_max_size = str_max_size;
	res->tokens = calloc(buf_max_size, sizeof(int));
	res->identifiers = calloc(buf_max_size, sizeof(char *));
	res->string_literals = calloc(buf_max_size, sizeof(char *));
	res->integer_constants = calloc(buf_max_size, sizeof(int));
	res->floating_constants = calloc(buf_max_size, sizeof(float));
	for (int i = 0; i < buf_max_size; i++)
	{
		res->identifiers[i] = calloc(str_max_size, sizeof(char));
		res->string_literals[i] = calloc(str_max_size, sizeof(char));
	}
	return res;
}

void free_token_data(TokenData *td)
{
	for (int i = 0; i < td->_buf_max_size; i++)
	{
		free(td->identifiers[i]);
		free(td->string_literals[i]);
	}
	free(td->tokens);
	free(td->identifiers);
	free(td->string_literals);
	free(td->integer_constants);
	free(td->floating_constants);
	free(td);
}

char get_escaped_char(char chr)
{
	switch (chr)
	{
	case 'a':
		return '\a';
	case 'b':
		return '\b';
	case 'e':
		return '\e';
	case 'f':
		return '\f';
	case 'n':
		return '\n';
	case 'r':
		return '\r';
	case 't':
		return '\t';
	case 'v':
		return '\v';
	case '\\':
		return '\\';
	case '\'':
		return '\'';
	case '\"':
		return '\"';
	case '\?':
		return '\?';
	default:
		return 0;
	}
}

void emit_token(TokenData *buf, int tok)
{
	if (buf->_tok_idx >= buf->_buf_max_size)
	{
		buf->_overflow = 1;
		return;
	}

	buf->tokens[buf->_tok_idx] = tok;
	buf->_tok_idx++;
}

void emit_char_literal(TokenData *buf, unsigned char chr)
{
	emit_token(buf, TOK_CONSTANT_CHAR);
	if (isgraph(chr))
	{
		printf("\n\nEmitting: %c\n\n", chr);
	}
	else
	{
		printf("\n\nEmitting: %d\n\n", chr);
	}
	emit_token(buf, chr);
}

TokenData *tokenize(CharBuffer *cb)
{
	TokenData *token_data = alloc_token_data(5000, 250);

	int comment_line_mode = 0;
	int comment_block_mode = 0;
	int char_literal_mode = 0;
	int current_line = 1;

	while (cb_next(cb))
	{
		// Make sure we dont overflow the token buffer
		if (token_data->_overflow)
		{
			printf("Error: encountered an overflow in the TokenData struct!");
			return NULL;
		}

		// detect if a new line is encountered
		if (cb->cur_char == '\n' || cb->cur_char == '\r')
		{
			current_line++;
		}

		// check to exit char literal mode
		if (char_literal_mode)
		{
			if (cb->cur_char != '\'')
			{
				printf("Error: char literals cannot be longer than one character. Line: %d\n", current_line);
				return NULL;
			}
			char_literal_mode = 0;
			continue;
		}

		// Check if we are exiting comment modes
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

		// TODO: ALLOW SHORTER OCT VALUES such as 33 instead of 330
		// TODO: implement char literal prefixes

		// char literal mode
		if (cb->cur_char == '\'')
		{
			char_literal_mode = 1;
			// check if the char is an escape character
			if (cb->next_char == '\\')
			{
				cb_next(cb);
				if (cb->next_char == 'x')
				{
					unsigned char char_digits[2];
					int hex_digits[2];

					cb_next(cb);
					char_digits[0] = tolower(cb->next_char);
					cb_next(cb);
					char_digits[1] = tolower(cb->next_char);

					if (isalpha(char_digits[0]))
						hex_digits[0] = char_digits[0] - 'a' + 10;
					else
						hex_digits[0] = char_digits[0] - '0';

					if (isalpha(char_digits[1]))
						hex_digits[1] = char_digits[1] - 'a' + 10;
					else
						hex_digits[1] = char_digits[1] - '0';

					// check each digit
					for (int i = 0; i < 2; i++)
					{
						if (hex_digits[i] > 15 || hex_digits[i] < 0)
						{
							printf("Error: invalid hex escape sequence. Line: %d\n", current_line);
							return NULL;
						}
					}

					int final_val = hex_digits[0] * 16 + hex_digits[1];

					if (final_val > 255 || final_val < 0)
					{
						printf("Error: hex escape sequence out of range. Line: %d\n", current_line);
						return NULL;
					}

					emit_char_literal(token_data, final_val);
				}
				else if (isdigit(cb->next_char))
				{
					int oct_digits[3];

					oct_digits[0] = cb->next_char - '0';

					cb_next(cb);
					oct_digits[1] = cb->next_char - '0';

					cb_next(cb);
					oct_digits[2] = cb->next_char - '0';

					for (int i = 0; i < 3; i++)
					{
						if (oct_digits[i] > 7 || oct_digits[i] < 0)
						{
							printf("Error: invalid oct escape sequence. Line: %d\n", current_line);
							return NULL;
						}
					}

					int final_val = oct_digits[0] * 64 + oct_digits[1] * 8 + oct_digits[2];

					if (final_val > 255 || final_val < 0)
					{
						printf("Error: oct escape sequence out of range. Line: %d\n", current_line);
						return NULL;
					}

					emit_char_literal(token_data, final_val);
				}
				else
				{
					char escaped_char = get_escaped_char(cb->next_char);
					if (!escaped_char)
					{
						printf("Error: not a valid escape character. Line: %d\n", current_line);
						return NULL;
					}

					emit_char_literal(token_data, escaped_char);
				}
			}
			else
			{
				emit_char_literal(token_data, cb->next_char);
			}
			// Shift our buffer over by an extra one
			cb_next(cb);
			continue;
		}

		// parse out comments
		if (cb->cur_char == '/' && cb->next_char == '/')
		{
			comment_line_mode = 1;
			continue;
		}

		// block comments
		if (cb->cur_char == '/' && cb->next_char == '*')
		{
			comment_block_mode = 1;
			// This is in order to avoid allowing this type of comment: /*/
			cb_next(cb);
			continue;
		}

		// punctuators
		int pctr = is_punctuator(cb);
		if (pctr != -1)
		{
			emit_token(token_data, pctr);
			continue;
		}

		printf("%c", cb->cur_char);
	}

	return token_data;
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

	TokenData *tok_data = tokenize(cb);
	if (!tok_data)
	{
		return -1;
	}

	free_token_data(tok_data);

	delete_char_buffer(cb);

	return 0;
}
