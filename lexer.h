#pragma once

#include "char_buffer.h"
#include "token.h"

void free_token_data(TokenData *td);
TokenData *tokenize(CharBuffer *cb);
