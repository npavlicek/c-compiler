#pragma once

#include "char_buffer.h"

// Not all tokens are going to be implemented yet
enum Token
{
	// KEYWORDS
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
	TOK_STRUCT,
	TOK_SWITCH,
	TOK_TYPEDEF,
	TOK_UNION,
	TOK_UNSIGNED,
	TOK_VOID,
	TOK_WHILE,

	// GENERAL
	TOK_IDENTIFIER,
	TOK_CHAR_LITERAL,
	TOK_NUMERICAL_CONSTANT,
	TOK_STRING_LITERAL,

	// PUNCTUATORS
	TOK_OPEN_SQR_BRACK,
	TOK_CLOSE_SQR_BRACK,
	TOK_OPEN_PAREN,
	TOK_CLOSE_PAREN,
	TOK_OPEN_BRACK,
	TOK_CLOSE_BRACK,
	TOK_PERIOD,
	TOK_RIGHT_ARROW,
	TOK_INCREMENT,
	TOK_DECREMENT,
	TOK_AMPERSAND,
	TOK_STAR,
	TOK_PLUS,
	TOK_MINUS,
	TOK_TILDE,
	TOK_BANG,
	TOK_FORWARD_SLASH,
	TOK_PERCENT,
	TOK_BIT_SHIFT_LEFT,
	TOK_BIT_SHIFT_RIGHT,
	TOK_LSS,
	TOK_GTR,
	TOK_LSS_EQL,
	TOK_GTR_EQL,
	TOK_EQUALITY,
	TOK_EQUALITY_NOT,
	TOK_CARET,
	TOK_PIPE,
	TOK_AND,
	TOK_OR,
	TOK_COLON,
	TOK_SEMI_COLON,
	TOK_EQUAL,
	TOK_COMMA,
};

typedef enum int_types
{
	INT_TYPE_SIGNED_LLONG,
	INT_TYPE_UNSIGNED_LLONG,
	INT_TYPE_SIGNED_LONG,
	INT_TYPE_UNSIGNED_LONG,
	INT_TYPE_SIGNED_INT,
	INT_TYPE_UNSIGNED_INT
} int_types;

typedef enum floating_types
{
	FLOATING_TYPE_DOUBLE,
	FLOATING_TYPE_FLOAT,
	FLOATING_TYPE_LDOUBLE
} floating_types;

typedef struct NumConstant
{
	int base;
	int before_point;
	int after_point;
	int floating;
	int exponent;
	floating_types floating_type;
	int_types int_type;
} NumConstant;

typedef struct TokenData
{
	int _overflow;

	int _buf_max_size;
	int _str_buf_max_size;

	int _str_lit_max_len;
	int _ident_max_len;

	int _tok_idx;
	int _ident_idx;
	int _str_lit_idx;
	int _num_const_idx;

	int *tokens;
	char **identifiers;
	char **string_literals;
	NumConstant **num_constants;
} TokenData;

void free_token_data(TokenData *td);
TokenData *tokenize(CharBuffer *cb);
