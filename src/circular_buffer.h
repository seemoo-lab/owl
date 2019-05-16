#ifndef CIRCULAR_BUFFER_H_
#define CIRCULAR_BUFFER_H_

/// Handle type, the way users interact with the API
typedef struct circular_buf *cbuf_handle_t;

/// Pass in a size, returns a circular buffer handle
/// Requires: size > 0
/// Ensures: cbuf has been created and is returned in an empty state
cbuf_handle_t circular_buf_init(size_t size);

/// Free a circular buffer structure
/// Requires: cbuf is valid and created by circular_buf_init
void circular_buf_free(cbuf_handle_t cbuf);

/// Reset the circular buffer to empty, head == tail. Data not cleared
/// Requires: cbuf is valid and created by circular_buf_init
void circular_buf_reset(cbuf_handle_t cbuf);

/// Put version 1 continues to add data if the buffer is full
/// Old data is overwritten
/// Requires: cbuf is valid and created by circular_buf_init
void circular_buf_put(cbuf_handle_t cbuf, void *data);

/// Put Version 2 rejects new data if the buffer is full
/// Requires: cbuf is valid and created by circular_buf_init
/// Returns 0 on success, -1 if buffer is full
int circular_buf_put2(cbuf_handle_t cbuf, void *data);

/// Retrieve a value from the buffer
/// Requires: cbuf is valid and created by circular_buf_init
/// Returns 0 on success, -1 if the buffer is empty
int circular_buf_get(cbuf_handle_t cbuf, void **data, int peek);

/// CHecks if the buffer is empty
/// Requires: cbuf is valid and created by circular_buf_init
/// Returns true if the buffer is empty
int circular_buf_empty(cbuf_handle_t cbuf);

/// Checks if the buffer is full
/// Requires: cbuf is valid and created by circular_buf_init
/// Returns true if the buffer is full
int circular_buf_full(cbuf_handle_t cbuf);

/// Check the capacity of the buffer
/// Requires: cbuf is valid and created by circular_buf_init
/// Returns the maximum capacity of the buffer
size_t circular_buf_capacity(cbuf_handle_t cbuf);

/// Check the number of elements stored in the buffer
/// Requires: cbuf is valid and created by circular_buf_init
/// Returns the current number of elements in the buffer
size_t circular_buf_size(cbuf_handle_t cbuf);

#endif /* CIRCULAR_BUFFER_H_ */
