#include "char_buffer.h"

#include <stdlib.h>

CharBuffer *alloc_char_buffer(unsigned long max_size)
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
