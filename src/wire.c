/*
 * OWL: an open Apple Wireless Direct Link (AWDL) implementation
 * Copyright (C) 2018  The Open Wireless Link Project (https://owlink.org)
 * Copyright (C) 2018  Milan Stute
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdlib.h>

#include "wire.h"

struct buf {
	const uint8_t *orig;
	uint8_t *data;
	int len;
	int owned;
};

struct buf *buf_new_owned(int len) {
	if (len < 0)
		return NULL;
	struct buf *buf = (struct buf *) malloc(sizeof(struct buf));
	buf->data = (uint8_t *) malloc(len);
	buf->orig = buf->data;
	buf->len = len;
	buf->owned = 1;
	return buf;
}

const struct buf *buf_new_const(const uint8_t *data, int len) {
	if (len < 0)
		return NULL;
	struct buf *buf = (struct buf *) malloc(sizeof(struct buf));
	buf->data = (uint8_t *) data;
	buf->orig = data;
	buf->len = len;
	buf->owned = 0;
	return buf;
}

void buf_free(const struct buf *buf) {
	if (buf->owned)
		free((void *) buf->orig);
	free((struct buf *) buf);
}

const uint8_t *buf_data(const struct buf *buf) {
	return buf->data;
}

int buf_len(const struct buf *buf) {
	return buf->len;
}

int buf_strip(const struct buf *buf, int len) {
	if (len > buf->len || len < 0)
		return OUT_OF_BOUNDS;
	((struct buf *) buf)->data += len;
	((struct buf *) buf)->len -= len;
	return len;
}

int buf_take(const struct buf *buf, int len) {
	if (len > buf->len || len < 0)
		return OUT_OF_BOUNDS;
	((struct buf *) buf)->len -= len;
	return len;
}

int write_u8(struct buf *buf, int offset, uint8_t value) {
	if (offset < 0 || offset + 1 > buf->len)
		return OUT_OF_BOUNDS;
	*(buf->data + offset) = value;
	return 1;
}

int write_le16(struct buf *buf, int offset, uint16_t value) {
	if (offset < 0 || offset + 2 > buf->len)
		return OUT_OF_BOUNDS;
	*(uint16_t *) (buf->data + offset) = htole16(value);
	return 2;
}

int write_be16(struct buf *buf, int offset, uint16_t value) {
	if (offset < 0 || offset + 2 > buf->len)
		return OUT_OF_BOUNDS;
	*(uint16_t *) (buf->data + offset) = htobe16(value);
	return 2;
}

int write_le32(struct buf *buf, int offset, uint32_t value) {
	if (offset < 0 || offset + 4 > buf->len)
		return OUT_OF_BOUNDS;
	*(uint32_t *) (buf->data + offset) = htole32(value);
	return 4;
}

int write_be32(struct buf *buf, int offset, uint32_t value) {
	if (offset < 0 || offset + 4 > buf->len)
		return OUT_OF_BOUNDS;
	*(uint32_t *) (buf->data + offset) = htobe32(value);
	return 4;
}

int write_ether_addr(struct buf *buf, int offset, const struct ether_addr *addr) {
	if (offset < 0 || offset + ETHER_ADDR_LEN > buf->len)
		return OUT_OF_BOUNDS;
	*(struct ether_addr *) (buf->data + offset) = *addr;
	return ETHER_ADDR_LEN;
}

int write_bytes(struct buf *buf, int offset, const uint8_t *bytes, int length) {
	if (offset < 0 || offset + length > buf->len)
		return OUT_OF_BOUNDS;
	memcpy(buf->data + offset, bytes, length);
	return length;
}

int read_u8(const struct buf *buf, int offset, uint8_t *value) {
	if (offset < 0 || offset + 1 > buf->len)
		return OUT_OF_BOUNDS;
	if (value)
		*value = *(buf->data + offset);
	return 1;
}

int read_le16(const struct buf *buf, int offset, uint16_t *value) {
	if (offset < 0 || offset + 2 > buf->len)
		return OUT_OF_BOUNDS;
	if (value)
		*value = le16toh(*(uint16_t *) (buf->data + offset));
	return 2;
}

int read_be16(const struct buf *buf, int offset, uint16_t *value) {
	if (offset < 0 || offset + 2 > buf->len)
		return OUT_OF_BOUNDS;
	if (value)
		*value = be16toh(*(uint16_t *) (buf->data + offset));
	return 2;
}

int read_le32(const struct buf *buf, int offset, uint32_t *value) {
	if (offset < 0 || offset + 4 > buf->len)
		return OUT_OF_BOUNDS;
	if (value)
		*value = le32toh(*(uint32_t *) (buf->data + offset));
	return 4;
}

int read_be32(const struct buf *buf, int offset, uint32_t *value) {
	if (offset < 0 || offset + 4 > buf->len)
		return OUT_OF_BOUNDS;
	if (value)
		*value = be32toh(*(uint32_t *) (buf->data + offset));
	return 4;
}

int read_ether_addr(const struct buf *buf, int offset, struct ether_addr *addr) {
	if (offset < 0 || offset + ETHER_ADDR_LEN > buf->len)
		return OUT_OF_BOUNDS;
	if (addr)
		*addr = *(struct ether_addr *) (buf->data + offset);
	return ETHER_ADDR_LEN;
}

int read_bytes(const struct buf *buf, int offset, const uint8_t **bytes, int length) {
	if (offset < 0 || offset + length > buf->len)
		return OUT_OF_BOUNDS;
	if (bytes)
		*bytes = buf->data + offset;
	return length;
}

int read_bytes_copy(const struct buf *buf, int offset, uint8_t *bytes, int length) {
	if (offset < 0 || offset + length > buf->len)
		return OUT_OF_BOUNDS;
	if (bytes)
		memcpy(bytes, buf->data + offset, length);
	return length;
}

int read_int_string(const struct buf *buf, int offset, char *str, int length) {
	uint8_t _len;
	READ_U8(buf, offset, &_len);
	if (_len > length)
		_len = length;
	READ_BYTES_COPY(buf, offset + 1, (uint8_t *) str, _len);
	if (str)
		str[_len] = 0; /* add trailing zero */
	return _len + 1;
wire_error:
	return OUT_OF_BOUNDS;
}

int read_tlv(const struct buf *buf, int offset, uint8_t *type, uint16_t *len, const uint8_t **val) {
	uint8_t _type;
	uint16_t _len;
	READ_U8(buf, offset, &_type);
	READ_LE16(buf, offset + 1, &_len);
	READ_BYTES(buf, offset + 3, val, _len);
	if (type)
		*type = _type;
	if (len)
		*len = _len;
	return _len + 3;
wire_error:
	return OUT_OF_BOUNDS;
}
