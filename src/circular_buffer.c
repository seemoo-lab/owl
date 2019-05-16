#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#include "circular_buffer.h"

// The definition of our circular buffer structure is hidden from the user
struct circular_buf {
	void **buffer;
	size_t head;
	size_t tail;
	size_t max; //of the buffer
	int full;
};

static void advance_pointer(cbuf_handle_t cbuf)
{
	assert(cbuf);

	if(cbuf->full)
	{
		cbuf->tail = (cbuf->tail + 1) % cbuf->max;
	}

	cbuf->head = (cbuf->head + 1) % cbuf->max;

	// We mark full because we will advance tail on the next time around
	cbuf->full = (cbuf->head == cbuf->tail);
}

static void retreat_pointer(cbuf_handle_t cbuf)
{
	assert(cbuf);

	cbuf->full = 0;
	cbuf->tail = (cbuf->tail + 1) % cbuf->max;
}

cbuf_handle_t circular_buf_init(size_t size)
{
	assert(size);

	cbuf_handle_t cbuf = malloc(sizeof(struct circular_buf));
	assert(cbuf);

	cbuf->buffer = malloc(size * sizeof(void*));
	cbuf->max = size;
	circular_buf_reset(cbuf);

	assert(circular_buf_empty(cbuf));

	return cbuf;
}

void circular_buf_free(cbuf_handle_t cbuf)
{
	assert(cbuf);
	assert(cbuf->buffer);
	free(cbuf->buffer);
	free(cbuf);
}

void circular_buf_reset(cbuf_handle_t cbuf)
{
	assert(cbuf);

	cbuf->head = 0;
	cbuf->tail = 0;
	cbuf->full = 0;
}

size_t circular_buf_size(cbuf_handle_t cbuf)
{
	assert(cbuf);

	size_t size = cbuf->max;

	if(!cbuf->full)
	{
		if(cbuf->head >= cbuf->tail)
		{
			size = (cbuf->head - cbuf->tail);
		}
		else
		{
			size = (cbuf->max + cbuf->head - cbuf->tail);
		}

	}

	return size;
}

size_t circular_buf_capacity(cbuf_handle_t cbuf)
{
	assert(cbuf);

	return cbuf->max;
}

void circular_buf_put(cbuf_handle_t cbuf, void *data)
{
	assert(cbuf && cbuf->buffer);

	cbuf->buffer[cbuf->head] = data;

	advance_pointer(cbuf);
}

int circular_buf_put2(cbuf_handle_t cbuf, void *data)
{
	int r = -1;

	assert(cbuf && cbuf->buffer);

	if(!circular_buf_full(cbuf))
	{
		cbuf->buffer[cbuf->head] = data;
		advance_pointer(cbuf);
		r = 0;
	}

	return r;
}

int circular_buf_get(cbuf_handle_t cbuf, void **data, int peek)
{
	assert(cbuf && cbuf->buffer);

	int r = -1;

	if(!circular_buf_empty(cbuf))
	{
		if (data)
			*data = cbuf->buffer[cbuf->tail];
		if (!peek)
			retreat_pointer(cbuf);

		r = 0;
	}

	return r;
}

int circular_buf_empty(cbuf_handle_t cbuf)
{
	assert(cbuf);

	return (!cbuf->full && (cbuf->head == cbuf->tail));
}

int circular_buf_full(cbuf_handle_t cbuf)
{
	assert(cbuf);

	return cbuf->full;
}
