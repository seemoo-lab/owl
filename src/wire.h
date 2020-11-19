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

#ifndef OWL_WIRE_H
#define OWL_WIRE_H

#include <stdint.h>
#include <net/ethernet.h>

#ifdef __APPLE__

#include <machine/endian.h>
#include <libkern/OSByteOrder.h>

#define htobe16(x) OSSwapHostToBigInt16(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)

#define htobe32(x) OSSwapHostToBigInt32(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)

#define htobe64(x) OSSwapHostToBigInt64(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)

#else

#include <endian.h>

#endif /* __APPLE__ */


/** @brief Opaque buffer used to transport network packets.
 *
 * We can read from and write to a buffer using dedicated
 * read and write functions.
 *
 * @see buf_data
 * @see buf_len
 */
struct buf;

/* TODO change interface: only return wire_error or 0, do not return length */

enum wire_error {
  OUT_OF_BOUNDS = -1,
};

/** @brief Allocates a new {@code buf} of given length.
 *
 * Will free internal array when calling {@code buf_free}.
 *
 * @param len length of the buffer in bytes
 * @return a pointer to a mutable {@code buf} instance
 *
 * @see buf_new_const
 * @see buf_free
 */
struct buf *buf_new_owned(int len);

/** @brief Allocates a new {@code buf} an already existing bytes array.
 *
 * Will use {@code data} as its internal array which is _not_ freed when calling {@code buf_free}.
 *
 * @param data immutable reference to buffer array
 * @param len length of the buffer in bytes
 * @return a pointer to a mutable {@code buf} instance
 *
 * @see buf_new_owned
 * @see buf_free
 */
const struct buf *buf_new_const(const uint8_t *data, int len);

/** @brief Deallocates a {@code buf} instance.
 *
 * Will dellocate internal bytes array if {@code buf} is an 'owned' instance.
 *
 * @param buf instance to free
 *
 * @see buf_new_owned
 * @see buf_new_const
 */
void buf_free(const struct buf *buf);

/** @brief Read-only access to internal bytes array. */
const uint8_t *buf_data(const struct buf *buf);

/** @brief Length of internal bytes array. */
int buf_len(const struct buf *buf);

#define BUF_STRIP(buf, len) \
  do { \
    int result = buf_strip(buf, len); \
    if (result < 0) \
      goto wire_error; \
  } while (0)

/** @brief Strip {@code len} bytes from the front of {@code buf}.
 *
 * @return 0 if successful or -1 if {@code len} is longer than length of {@code buf}
 */
int buf_strip(const struct buf *buf, int len);

#define BUF_TAKE(buf, len) \
  do { \
    int result = buf_take(buf, len); \
    if (result < 0) \
      goto wire_error; \
  } while (0)

/** @brief Strip {@code len} bytes from the end of {@code buf}.
 *
 * @return 0 if successful or -1 if {@code len} is longer than length of {@code buf}
 */
int buf_take(const struct buf *buf, int len);

int write_u8(struct buf *buf, int offset, uint8_t value);

int write_le16(struct buf *buf, int offset, uint16_t value);

int write_be16(struct buf *buf, int offset, uint16_t value);

int write_le32(struct buf *buf, int offset, uint32_t value);

int write_be32(struct buf *buf, int offset, uint32_t value);

int write_ether_addr(struct buf *buf, int offset, const struct ether_addr *addr);

int write_bytes(struct buf *buf, int offset, const uint8_t *bytes, int length);

#define CHECKED_READ(type, ...) \
  do { \
    int result = read_##type(__VA_ARGS__); \
    if (result < 0) \
      goto wire_error; \
  } while (0)

#define READ_U8(...) CHECKED_READ(u8, __VA_ARGS__)
#define READ_LE16(...) CHECKED_READ(le16, __VA_ARGS__)
#define READ_BE16(...) CHECKED_READ(be16, __VA_ARGS__)
#define READ_LE32(...) CHECKED_READ(le32, __VA_ARGS__)
#define READ_BE32(...) CHECKED_READ(be32, __VA_ARGS__)
#define READ_ETHER_ADDR(...) CHECKED_READ(ether_addr, __VA_ARGS__)
#define READ_BYTES(...) CHECKED_READ(bytes, __VA_ARGS__)
#define READ_BYTES_COPY(...) CHECKED_READ(bytes_copy, __VA_ARGS__)
#define READ_INT_STRING(...) CHECKED_READ(int_string, __VA_ARGS__)
#define READ_TLV(...) CHECKED_READ(tlv, __VA_ARGS__)

/** @brief Read an unsigned char from a @code buffer.
 *
 * @param buf @code buffer to be read from
 * @param offset read at this offset in bytes
 * @param value read into this pointer, can be null if not wanted
 * @return the number of bytes read or a negative value if an error occured
 */
int read_u8(const struct buf *buf, int offset, uint8_t *value);

/** @brief Read an unsigned short in little-endian byte order.
 *
 * @see read_u8
 */
int read_le16(const struct buf *buf, int offset, uint16_t *value);

int read_be16(const struct buf *buf, int offset, uint16_t *value);

int read_le32(const struct buf *buf, int offset, uint32_t *value);

int read_be32(const struct buf *buf, int offset, uint32_t *value);

int read_ether_addr(const struct buf *buf, int offset, struct ether_addr *addr);

int read_bytes(const struct buf *buf, int offset, const uint8_t **bytes, int length);

int read_bytes_copy(const struct buf *buf, int offset, uint8_t *bytes, int length);

int read_int_string(const struct buf *buf, int offset, char *str, int length);

/** @brief Get offset to next TLV
 *
 * Can also be used to check whether buffer points to a complete TLV.
 *
 * @param buf @code buffer to be read from
 * @param offset read at this offset in bytes
 * @param type pointer is set to the type of the TLV, can be null if not wanted
 * @param len pointer is set to the length of the TLV, can be null if not wanted
 * @param val pointer is set to the value of the TLV, can be null if not wanted
 * @return offset to next TLV or a negative value if an error occured
 */
int read_tlv(const struct buf *buf, int offset, uint8_t *type, uint16_t *len, const uint8_t **val);

#endif /* OWL_WIRE_H */
