#pragma once

typedef struct CharBuffer
{
	char *_buf;
	int _cur_idx;

	unsigned long _max_size;
	unsigned long _size;

	// end-of-buffer
	int eob;

	char cur_char;
	char next_char;
} CharBuffer;

CharBuffer *alloc_char_buffer(unsigned long max_size);
void delete_char_buffer(CharBuffer *cb);

int cb_next(CharBuffer *cb);
int cb_back(CharBuffer *cb);
