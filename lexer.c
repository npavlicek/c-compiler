#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

// clang-format off
// The string representation of keywords
static const char *const keywords_str[] = 
{
	"auto",
	"break",
	"case",
	"char",
	"const",
	"continue",
	"default",
	"do",
	"double",
	"else",
	"enum",
	"float",
	"for",
	"goto",
	"if",
	"inline",
	"int",
	"long",
	"return",
	"short",
	"signed",
	"sizeof",
	"static",
	"typedef",
	"union",
	"unsigned",
	"void",
	"while"
};

// The token equivalent of each element in keywords_str
static const enum Token keywords_tok[] = 
{
	TOK_AUTO,
	TOK_BREAK,
	TOK_CASE,
	TOK_CHAR,
	TOK_CONST,
	TOK_CONTINUE,
	TOK_DEFAULT,
	TOK_DO,
	TOK_DOUBLE,
	TOK_ELSE,
	TOK_ENUM,
	TOK_FLOAT,
	TOK_FOR,
	TOK_GOTO,
	TOK_IF,
	TOK_INLINE,
	TOK_INT,
	TOK_LONG,
	TOK_RETURN,
	TOK_SHORT,
	TOK_SIGNED,
	TOK_SIZEOF,
	TOK_STATIC,
	TOK_TYPEDEF,
	TOK_UNION,
	TOK_UNSIGNED,
	TOK_VOID,
	TOK_WHILE
};

static const char single_punctuator_char[] = 
{
		'[',
		']',
		'(',
		')',
		'{',
		'}',
		'.',
		'&',
		'*',
		'+',
		'-',
		'~',
		'!',
		'/',
		'%',
		'<',
		'>',
		'^',
		'|',
		':',
		';',
		'=',
		',',
};

static const enum Token single_punctuator_tok[] =
{
		TOK_OPEN_SQR_BRACK,
		TOK_CLOSE_SQR_BRACK,
		TOK_OPEN_PAREN,
		TOK_CLOSE_PAREN,
		TOK_OPEN_BRACK,
		TOK_CLOSE_BRACK,
		TOK_PERIOD,
		TOK_AMPERSAND,
		TOK_STAR,
		TOK_PLUS,
		TOK_MINUS,
		TOK_TILDE,
		TOK_BANG,
		TOK_FORWARD_SLASH,
		TOK_PERCENT,
		TOK_LSS,
		TOK_GTR,
		TOK_CARET,
		TOK_PIPE,
		TOK_COLON,
		TOK_SEMI_COLON,
		TOK_EQUAL,
		TOK_COMMA
};
// clang-format on

// Returns -1 if theres no match, otherwise the token
static int is_keyword(const char *const str)
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
static int is_punctuator(CharBuffer *cb)
{
	char f = cb->cur_char;
	char s = cb->next_char;

	// Shift the char buffer over one in case we return early
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

	// If we didnt detect a double char punctuator we shift back
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

static TokenData *alloc_token_data(int buf_max_size, int str_max_size, int str_lit_max_len, int ident_max_len)
{
	TokenData *res = calloc(1, sizeof(TokenData));
	res->_buf_max_size = buf_max_size;
	res->_str_buf_max_size = str_max_size;
	res->_str_lit_max_len = str_lit_max_len;
	res->_ident_max_len = ident_max_len;
	res->tokens = calloc(buf_max_size, sizeof(int));
	res->identifiers = calloc(buf_max_size, sizeof(char *));
	res->string_literals = calloc(str_max_size, sizeof(char *));
	res->num_constants = calloc(buf_max_size, sizeof(NumConstant *));
	for (int i = 0; i < buf_max_size; i++)
	{
		res->identifiers[i] = calloc(ident_max_len, sizeof(char));
	}
	for (int i = 0; i < str_max_size; i++)
	{
		res->string_literals[i] = calloc(str_lit_max_len, sizeof(char));
	}
	return res;
}

// Returns -1 if no match, otherwise the index of the identifier
static int check_identifier(TokenData *token_data, const char *const ident)
{
	for (int i = 0; i < token_data->_ident_idx; i++)
	{
		if (strcmp(ident, token_data->identifiers[i]) == 0)
		{
			return i;
		}
	}
	return -1;
}

void free_token_data(TokenData *td)
{
	for (int i = 0; i < td->_buf_max_size; i++)
	{
		free(td->identifiers[i]);
		free(td->num_constants[i]);
	}
	for (int i = 0; i < td->_str_buf_max_size; i++)
	{
		free(td->string_literals[i]);
	}
	free(td->tokens);
	free(td->identifiers);
	free(td->string_literals);
	free(td->num_constants);
	free(td);
}

static char get_escaped_char(char chr)
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

static void emit_token(TokenData *buf, int tok)
{
	if (buf->_tok_idx >= buf->_buf_max_size)
	{
		buf->_overflow = 1;
		return;
	}

	buf->tokens[buf->_tok_idx] = tok;
	buf->_tok_idx++;
}

static void emit_char_literal(TokenData *buf, char chr)
{
	emit_token(buf, TOK_CHAR_LITERAL);
	emit_token(buf, chr);
}

// The string buffer supplied should already have a null terminator
static void emit_str_literal(TokenData *buf, const char *str)
{
	// sanity check the length of the string
	if (strlen(str) >= buf->_str_lit_max_len)
	{
		buf->_overflow = 1;
		return;
	}

	// check for overflows
	if (buf->_str_lit_idx >= buf->_str_buf_max_size)
	{
		buf->_overflow = 1;
		return;
	}

	strcpy(buf->string_literals[buf->_str_lit_idx], str);

	emit_token(buf, TOK_STRING_LITERAL);
	emit_token(buf, buf->_str_lit_idx);

	buf->_str_lit_idx++;
}

// Returns a value other than zero if theres an error or overflow
static void emit_ident(TokenData *token_data, const char *const ident)
{
	emit_token(token_data, TOK_IDENTIFIER);

	int idnt_idx = check_identifier(token_data, ident);
	if (idnt_idx != -1)
	{
		emit_token(token_data, idnt_idx);
	}
	else
	{
		// check for overflows
		if (strlen(ident) >= token_data->_ident_max_len || token_data->_ident_idx >= token_data->_buf_max_size)
		{
			token_data->_overflow = 1;
			return;
		}

		strcpy(token_data->identifiers[token_data->_ident_idx], ident);
		emit_token(token_data, token_data->_ident_idx);
		token_data->_ident_idx++;
	}
}

static int current_line = 1;

static void print_error(const char *message)
{
	printf("[Line %d] Error: %s\n", current_line, message);
	raise(SIGTRAP);
}

// returns anything other than zero if an error is present
// function assumes the current char is the backslash that enters the escape sequence
// function will return with the charbuffer being on the last char in the escape sequence
static int process_escape_sequence(char *res, CharBuffer *cb, TokenData *token_data)
{
	if (cb->next_char == 'x')
	{
		int hex_digits[2];
		int num_digits = 0;

		for (int i = 0; i < 2; i++)
		{
			cb_next(cb);
			if (isxdigit(cb->next_char))
			{
				if (isalpha(cb->next_char))
					hex_digits[i] = tolower(cb->next_char) - 'a' + 10;
				else
					hex_digits[i] = cb->next_char - '0';
				num_digits++;
			}
			else
			{
				cb_back(cb);
			}
		}

		if (num_digits == 0)
		{
			print_error("hex escape character not followed by hex digits");
			return 1;
		}

		int final_val = 0;
		int powers_of_16[] = {1, 16};
		int powers_idx = 0;
		for (int i = num_digits - 1; i >= 0; i--)
		{
			final_val += hex_digits[i] * powers_of_16[powers_idx];
			powers_idx++;
		}

		if (final_val > 255 || final_val < 0)
		{
			print_error("hex escape sequence out of range.");
			return 1;
		}

		*res = final_val;
	}
	else if (isdigit(cb->next_char))
	{
		int oct_digits[3];
		int num_digits = 0;

		for (int i = 0; i < 3; i++)
		{
			if (i != 0)
				cb_next(cb);
			oct_digits[i] = cb->next_char - '0';
			if (oct_digits[i] < 0 || oct_digits[i] > 7)
			{
				cb_back(cb);
				break;
			}
			num_digits++;
		}

		if (num_digits == 0)
		{
			print_error("oct escape character not followed by any octal digits");
			return 1;
		}

		int final_val = 0;
		int powers_of_8[] = {1, 8, 64};
		int powers_idx = 0;
		for (int i = num_digits - 1; i >= 0; i--)
		{
			final_val += oct_digits[i] * powers_of_8[powers_idx];
			powers_idx++;
		}

		if (final_val > 255 || final_val < 0)
		{
			print_error("oct escape sequence out of range.");
			return 1;
		}

		*res = final_val;
	}
	else
	{
		char escaped_char = get_escaped_char(cb->next_char);
		if (!escaped_char)
		{
			print_error("not a valid escape character.");
			return 1;
		}

		*res = escaped_char;
	}

	cb_next(cb);
	return 0;
}

#define NUM_INTEGER_SUFFIXES 22
#define NUM_FLOATING_SUFFIXES 4

// clang-format off
static const char *integer_suffixes[NUM_INTEGER_SUFFIXES] = {
	"llu",
	"llU",
	"LLu",
	"LLU",

	"lu",
	"lU",
	"Lu",
	"LU",
	
	"ull",
	"Ull",
	"uLL",
	"ULL",

	"ul",
	"Ul",
	"uL",
	"UL",

	"u",
	"U",

	"ll",
	"LL",
	
	"l",
	"L"
};

static const int_types integer_suffix_types[NUM_INTEGER_SUFFIXES] = {
	INT_TYPE_UNSIGNED_LLONG,
	INT_TYPE_UNSIGNED_LLONG,
	INT_TYPE_UNSIGNED_LLONG,
	INT_TYPE_UNSIGNED_LLONG,

	INT_TYPE_UNSIGNED_LONG,
	INT_TYPE_UNSIGNED_LONG,
	INT_TYPE_UNSIGNED_LONG,
	INT_TYPE_UNSIGNED_LONG,

	INT_TYPE_UNSIGNED_LLONG,
	INT_TYPE_UNSIGNED_LLONG,
	INT_TYPE_UNSIGNED_LLONG,
	INT_TYPE_UNSIGNED_LLONG,

	INT_TYPE_UNSIGNED_LONG,
	INT_TYPE_UNSIGNED_LONG,
	INT_TYPE_UNSIGNED_LONG,
	INT_TYPE_UNSIGNED_LONG,

	INT_TYPE_UNSIGNED_INT,
	INT_TYPE_UNSIGNED_INT,

	INT_TYPE_SIGNED_LLONG,
	INT_TYPE_SIGNED_LLONG,

	INT_TYPE_SIGNED_LONG,
	INT_TYPE_SIGNED_LONG,
};

static const char *floating_suffixes[NUM_FLOATING_SUFFIXES] = {
	"f",
	"F",
	"l",
	"L",
};

static const floating_types floating_suffix_types[NUM_FLOATING_SUFFIXES] = {
	FLOATING_TYPE_FLOAT,
	FLOATING_TYPE_FLOAT,
	FLOATING_TYPE_LDOUBLE,
	FLOATING_TYPE_LDOUBLE,
};
// clang-format on

static int is_digit_in_base(char chr, int base)
{
	if (base == 16)
	{
		if (isxdigit(chr))
			return 1;
	}
	else if (base == 8)
	{
		int digit = chr - '0';
		if (digit >= 0 && digit <= 7)
			return 1;
	}
	else
	{
		if (isdigit(chr))
			return 1;
	}
	return 0;
}

static int digits(CharBuffer *cb, int base, int *res, int *digits_read)
{
	static const int MAX_DIGIT_LENGTH = 100;
	char digit_buffer[MAX_DIGIT_LENGTH];
	memset(digit_buffer, 0, MAX_DIGIT_LENGTH);
	int digit_idx = 0;
	int found_digit = 0;
	if (cb->cur_char == '+' || cb->cur_char == '-')
	{
		digit_buffer[digit_idx++] = cb->cur_char;
		cb_next(cb);
	}
	while (is_digit_in_base(cb->cur_char, base))
	{
		if (digit_idx >= MAX_DIGIT_LENGTH)
		{
			print_error("numerical constant too long");
			return 1;
		}
		digit_buffer[digit_idx] = cb->cur_char;
		digit_idx += 1;
		cb_next(cb);
		found_digit = 1;
	}
	*res = strtol(digit_buffer, NULL, base);
	if (digits_read && found_digit)
		*digits_read = 1;
	return 0;
}

static int read_constant(CharBuffer *cb, NumConstant *nc)
{
	int res;
	int err = digits(cb, nc->base, &res, NULL);
	if (err)
		return err;

	nc->before_point = res;

	if (cb->cur_char == '.')
	{
		nc->floating = 1;
		cb_next(cb);
		int err = digits(cb, nc->base, &res, NULL);
		if (err)
			return err;

		nc->after_point = res;

		if (nc->base == 16 && tolower(cb->cur_char) != 'p')
		{
			print_error("hex floating constant requires exponent");
			return 1;
		}
	}

	// consume the exponent
	if ((nc->base == 10 && tolower(cb->cur_char == 'e')) ||
	    (nc->base == 16 && (tolower(cb->cur_char == 'p') || tolower(cb->cur_char == 'e'))))
	{
		// consume the char
		cb_next(cb);
		int digits_read = 0;
		err = digits(cb, 10, &res, &digits_read);
		if (err)
			return err;

		if (!digits_read)
		{
			print_error("hex floating constant requires exponent");
			return 1;
		}

		nc->exponent = res;
	}

	// read suffix
	static const int MAX_LEN_SUFFIX_BUFFER = 50;
	char suffix_buffer[MAX_LEN_SUFFIX_BUFFER];
	memset(suffix_buffer, 0, MAX_LEN_SUFFIX_BUFFER);
	int suffix_buffer_idx = 0;
	while (isalpha(cb->cur_char))
	{
		if (suffix_buffer_idx >= MAX_LEN_SUFFIX_BUFFER - 1)
		{
			print_error("suffix is too long");
			return 1;
		}
		suffix_buffer[suffix_buffer_idx++] = cb->cur_char;
		cb_next(cb);
	}

	// We make sure we dont accidentally consume the next char
	cb_back(cb);

	// check suffixes
	int found_suffix = 0;
	if (strlen(suffix_buffer) > 0)
	{
		if (nc->floating)
		{
			for (int i = 0; i < NUM_FLOATING_SUFFIXES; i++)
			{
				if (strcmp(suffix_buffer, floating_suffixes[i]) == 0)
				{
					nc->floating_type = floating_suffix_types[i];
					found_suffix = 1;
				}
			}
		}
		else
		{
			for (int i = 0; i < NUM_INTEGER_SUFFIXES; i++)
			{
				if (strcmp(suffix_buffer, integer_suffixes[i]) == 0)
				{
					nc->int_type = integer_suffix_types[i];
					found_suffix = 1;
				}
			}
		}
	}
	else
	{
		if (nc->floating)
		{
			nc->floating_type = FLOATING_TYPE_DOUBLE;
		}
		else
		{
			nc->int_type = INT_TYPE_SIGNED_INT;
		}
		found_suffix = 1;
	}

	if (!found_suffix && strlen(suffix_buffer) > 0)
	{
		print_error("invalid suffix");
		return 1;
	}

	return 0;
}

// Returns !0 if error
static int emit_num_constant(NumConstant *nc, TokenData *token_data)
{
	if (token_data->_num_const_idx >= token_data->_buf_max_size)
	{
		token_data->_overflow = 1;
		return 1;
	}

	emit_token(token_data, TOK_NUMERICAL_CONSTANT);

	token_data->num_constants[token_data->_num_const_idx] = nc;
	emit_token(token_data, token_data->_num_const_idx);
	token_data->_num_const_idx++;

	return 0;
}

// returns !0 if an error was encountered
static int num_constant(CharBuffer *cb, TokenData *token_data)
{
	if (isdigit(cb->cur_char))
	{
		NumConstant *nc = calloc(1, sizeof(NumConstant));
		if (cb->cur_char == '0' && tolower(cb->next_char) == 'x')
		{
			cb_next(cb);
			cb_next(cb);

			nc->base = 16;
		}
		else if (cb->cur_char == '0')
		{
			cb_next(cb);

			nc->base = 8;
		}
		else
		{
			nc->base = 10;
		}
		int err = read_constant(cb, nc);
		if (err)
			return err;

		err = emit_num_constant(nc, token_data);
		if (err)
			return err;
	}
	return 0;
}

TokenData *tokenize(CharBuffer *cb)
{
	current_line = 1;

	const int BUFFERS_MAX_NUM = 50000;
	const int STRING_LITERAL_MAX_NUM = 5000;
	const int IDENTIFIER_MAX_LEN = 249;
	const int STRING_LITERAL_MAX_LEN = 4999;
	TokenData *token_data =
		alloc_token_data(BUFFERS_MAX_NUM, STRING_LITERAL_MAX_NUM, STRING_LITERAL_MAX_LEN + 1, IDENTIFIER_MAX_LEN + 1);

	if (!token_data)
	{
		print_error("failed to allocate token data struct");
		return NULL;
	}

	int comment_line_mode = 0;
	int comment_block_mode = 0;
	int string_literal_mode = 0;

	char string_literal_buffer[STRING_LITERAL_MAX_LEN + 1];
	memset(string_literal_buffer, 0, STRING_LITERAL_MAX_LEN + 1);
	int string_literal_idx = 0;

	char identifier_buffer[IDENTIFIER_MAX_LEN + 1];
	memset(identifier_buffer, 0, IDENTIFIER_MAX_LEN + 1);
	int identifier_buf_idx = 0;

	while (cb_next(cb))
	{
		// Make sure we dont overflow the token buffer
		if (token_data->_overflow)
		{
			print_error("encountered an overflow in the TokenData struct");
			return NULL;
		}

		// detect if a new line is encountered
		if (cb->cur_char == '\n' || cb->cur_char == '\r')
		{
			current_line++;
		}

		// check string literal mode
		if (string_literal_mode)
		{
			// error if a new line is encountered before the exit char
			if (cb->cur_char == '\n' || cb->cur_char == '\r')
			{
				print_error("string literals must be ended before a new line");
				return NULL;
			}

			// end the string literal and emit
			if (cb->cur_char == '\"')
			{
				emit_str_literal(token_data, string_literal_buffer);
				memset(string_literal_buffer, 0, STRING_LITERAL_MAX_LEN + 1);
				string_literal_idx = 0;
				string_literal_mode = 0;
				continue;
			}

			// check if we are gonna overflow the string literal
			if (string_literal_idx >= STRING_LITERAL_MAX_LEN)
			{
				print_error("string literal is too long");
				return NULL;
			}

			char cur_char = cb->cur_char;

			// check for an escape char
			if (cb->cur_char == '\\')
			{
				int err = process_escape_sequence(&cur_char, cb, token_data);
				if (err)
					return NULL;
			}

			string_literal_buffer[string_literal_idx] = cur_char;
			string_literal_idx++;
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

		// for now lets just consume all the preprocessor directives
		// must be after all comment checks and string literal checks or else we will parse out any strings with # in
		// them or mess up the comment block mode
		if (cb->cur_char == '#')
		{
			while (cb->next_char != '\n')
			{
				cb_next(cb);
			}
			continue;
		}

		// TODO: implement char literal prefixes

		// char literal mode
		if (cb->cur_char == '\'')
		{
			// check if the char is an escape character
			if (cb->next_char == '\\')
			{
				cb_next(cb);
				char chr;
				int err = process_escape_sequence(&chr, cb, token_data);
				if (err)
					return NULL;
				cb_next(cb);
				emit_char_literal(token_data, chr);
			}
			else
			{
				cb_next(cb);
				emit_char_literal(token_data, cb->cur_char);
				cb_next(cb);
			}

			if (cb->cur_char != '\'')
			{
				print_error("char literals cannot be longer than one character.");
				return NULL;
			}

			continue;
		}

		// string literals
		if (cb->cur_char == '\"')
		{
			string_literal_mode = 1;
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

		// check for identifiers or keywords
		if (isalpha(cb->cur_char) || cb->cur_char == '_')
		{
			identifier_buffer[identifier_buf_idx] = cb->cur_char;
			identifier_buf_idx++;
			while (isalnum(cb->next_char) || cb->next_char == '_')
			{
				if (identifier_buf_idx >= IDENTIFIER_MAX_LEN)
				{
					print_error("identifier name too long");
					return NULL;
				}
				cb_next(cb);
				identifier_buffer[identifier_buf_idx] = cb->cur_char;
				identifier_buf_idx++;
			}

			int kwd = is_keyword(identifier_buffer);
			if (kwd != -1)
			{
				emit_token(token_data, kwd);
			}
			else
			{
				emit_ident(token_data, identifier_buffer);
			}

			// finally reset the buffer
			memset(identifier_buffer, 0, IDENTIFIER_MAX_LEN + 1);
			identifier_buf_idx = 0;
			continue;
		}

		// Numerical constants
		int err = num_constant(cb, token_data);
		if (err)
			return NULL;
		else
			continue;

		printf("%c", cb->cur_char);
	}

	return token_data;
}
