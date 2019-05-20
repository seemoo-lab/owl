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

extern "C" {
#include "state.h"
#include "wire.h"
#include "tx.h"
#include "channel.h"
}

#include "gtest/gtest.h"

static struct buf *create_test_buf(int len) {
  return buf_new_owned(len);
}

static void destroy_test_buf(struct buf *buf) {
  buf_free(buf);
}

TEST(wire, buf_take) {
	bool success = 0;
	struct buf *frame = create_test_buf(2);

	write_u8(frame, 0, 4);
	write_u8(frame, 1, 2);

	BUF_TAKE(frame, 1);

	uint8_t x;
	read_u8(frame, 0, &x);
	EXPECT_EQ(x, 4);

	success = 1;

wire_error:
	EXPECT_TRUE(success);
	EXPECT_EQ(buf_len(frame), 1);
	destroy_test_buf(frame);
}

TEST(wire, buf_take_oob) {
	bool success = 0;
	struct buf *frame = create_test_buf(1);

	BUF_TAKE(frame, buf_len(frame) + 1);
	success = 1;

wire_error:
	EXPECT_FALSE(success);
	EXPECT_EQ(buf_len(frame), 1);
	destroy_test_buf(frame);
}


TEST(wire, buf_take_negative) {
	bool success = 0;
	struct buf *frame = create_test_buf(1);

	BUF_TAKE(frame, -1);
	success = 1;

wire_error:
	EXPECT_FALSE(success);
	EXPECT_EQ(buf_len(frame), 1);
	destroy_test_buf(frame);
}

TEST(wire, buf_strip) {
	bool success = 0;
	struct buf *frame = create_test_buf(2);

	write_u8(frame, 0, 4);
	write_u8(frame, 1, 2);

	BUF_STRIP(frame, 1);

	uint8_t x;
	read_u8(frame, 0, &x);
	EXPECT_EQ(x, 2);

	success = 1;

wire_error:
	EXPECT_TRUE(success);
	EXPECT_EQ(buf_len(frame), 1);
	destroy_test_buf(frame);
}

TEST(wire, buf_strip_oob) {
	bool success = 0;
	struct buf *frame = create_test_buf(1);

	BUF_STRIP(frame, buf_len(frame) + 1);
	success = 1;

wire_error:
	EXPECT_FALSE(success);
	EXPECT_EQ(buf_len(frame), 1);
	destroy_test_buf(frame);
}


TEST(wire, buf_strip_negative) {
	bool success = 0;
	struct buf *frame = create_test_buf(1);

	BUF_STRIP(frame, -1);
	success = 1;

wire_error:
	EXPECT_FALSE(success);
	EXPECT_EQ(buf_len(frame), 1);
	destroy_test_buf(frame);
}

TEST(wire, read_tlv) {
  struct ether_addr addr;
  struct awdl_state state;
  struct buf *frame;
  uint8_t *ptr, *start;
  int tlv_len;

	awdl_init_state(&state, "", &addr, CHAN_OPCLASS_6, 0);

  frame = create_test_buf(1024);
  // TODO use wire API in awdl_init_ functions, so cast is not necessary
  ptr = (uint8_t *) buf_data(frame);
  start = ptr;

  ptr += awdl_init_sync_params_tlv(ptr, &state);
  ptr += awdl_init_election_params_tlv(ptr, &state);
  ptr += awdl_init_chanseq_tlv(ptr, &state);
  ptr += awdl_init_service_params_tlv(ptr, &state);
  ptr += awdl_init_arpa_tlv(ptr, &state);
  ptr += awdl_init_data_path_state_tlv(ptr, &state);
  ptr += awdl_init_version_tlv(ptr, &state);

  buf_take(frame, buf_len(frame) - (ptr - start));

  uint8_t type;
  while ((tlv_len = read_tlv(frame, 0, &type, NULL, NULL)) > 0) {
    EXPECT_STRNE(awdl_tlv_as_str(type), "Unknown");
    buf_strip(frame, tlv_len);
  }

  EXPECT_EQ(buf_len(frame), 0);

  buf_free(frame);
}

TEST(wire, read_u8) {
  struct buf *frame = create_test_buf(1);

  EXPECT_LT(read_u8(frame, -1, NULL), 0);
  EXPECT_EQ(read_u8(frame, 0, NULL), 1);
  EXPECT_LT(read_u8(frame, 1, NULL), 0);

  destroy_test_buf(frame);
}

TEST(wire, checked_read_u8) {
  struct buf *frame = create_test_buf(1);

  int expect_fail = 0;
  READ_U8(frame, 0, NULL);
  EXPECT_FALSE(expect_fail);

  expect_fail = 1;
  READ_U8(frame, 1, NULL);
  EXPECT_FALSE(expect_fail); // we should not be here

wire_error:
  EXPECT_TRUE(expect_fail);
  destroy_test_buf(frame);
}

TEST(wire, read_le16) {
  struct buf *frame = create_test_buf(2);

  EXPECT_LT(read_le16(frame, -1, NULL), 0);
  EXPECT_EQ(read_le16(frame, 0, NULL), 2);
  EXPECT_LT(read_le16(frame, 1, NULL), 0);

  destroy_test_buf(frame);
}

TEST(wire, checked_read_le16) {
  struct buf *frame = create_test_buf(2);

  int expect_fail = 0;
  READ_LE16(frame, 0, NULL);
  EXPECT_FALSE(expect_fail);

  expect_fail = 1;
  READ_LE16(frame, 1, NULL);
  EXPECT_FALSE(expect_fail); // we should not be here

wire_error:
  EXPECT_TRUE(expect_fail);
  destroy_test_buf(frame);
}

TEST(wire, read_be16) {
  struct buf *frame = create_test_buf(2);

  EXPECT_LT(read_be16(frame, -1, NULL), 0);
  EXPECT_EQ(read_be16(frame, 0, NULL), 2);
  EXPECT_LT(read_be16(frame, 1, NULL), 0);

  destroy_test_buf(frame);
}

TEST(wire, checked_read_be16) {
  struct buf *frame = create_test_buf(2);

  int expect_fail = 0;
  READ_BE16(frame, 0, NULL);
  EXPECT_FALSE(expect_fail);

  expect_fail = 1;
  READ_BE16(frame, 1, NULL);
  EXPECT_FALSE(expect_fail); // we should not be here

wire_error:
  EXPECT_TRUE(expect_fail);
  destroy_test_buf(frame);
}

TEST(wire, read_le32) {
  struct buf *frame = create_test_buf(4);

  EXPECT_LT(read_le32(frame, -1, NULL), 0);
  EXPECT_EQ(read_le32(frame, 0, NULL), 4);
  EXPECT_LT(read_le32(frame, 1, NULL), 0);

  destroy_test_buf(frame);
}

TEST(wire, checked_read_le32) {
  struct buf *frame = create_test_buf(4);

  int expect_fail = 0;
  READ_LE32(frame, 0, NULL);
  EXPECT_FALSE(expect_fail);

  expect_fail = 1;
  READ_LE32(frame, 1, NULL);
  EXPECT_FALSE(expect_fail); // we should not be here

wire_error:
  EXPECT_TRUE(expect_fail);
  destroy_test_buf(frame);
}


TEST(wire, read_be32) {
  struct buf *frame = create_test_buf(4);

  EXPECT_LT(read_be32(frame, -1, NULL), 0);
  EXPECT_EQ(read_be32(frame, 0, NULL), 4);
  EXPECT_LT(read_be32(frame, 1, NULL), 0);

  destroy_test_buf(frame);
}

TEST(wire, checked_read_be32) {
  struct buf *frame = create_test_buf(4);

  int expect_fail = 0;
  READ_BE32(frame, 0, NULL);
  EXPECT_FALSE(expect_fail);

  expect_fail = 1;
  READ_BE32(frame, 1, NULL);
  EXPECT_FALSE(expect_fail); // we should not be here

wire_error:
  EXPECT_TRUE(expect_fail);
  destroy_test_buf(frame);
}

TEST(wire, read_ether_addr) {
  struct buf *frame = create_test_buf(6);

  EXPECT_LT(read_ether_addr(frame, -1, NULL), 0);
  EXPECT_EQ(read_ether_addr(frame, 0, NULL), 6);
  EXPECT_LT(read_ether_addr(frame, 1, NULL), 0);

  destroy_test_buf(frame);
}

TEST(wire, checked_read_ether_addr) {
  struct buf *frame = create_test_buf(6);

  int expect_fail = 0;
  READ_ETHER_ADDR(frame, 0, NULL);
  EXPECT_FALSE(expect_fail);

  expect_fail = 1;
  READ_ETHER_ADDR(frame, 1, NULL);
  EXPECT_FALSE(expect_fail); // we should not be here

wire_error:
  EXPECT_TRUE(expect_fail);
  destroy_test_buf(frame);
}

TEST(wire, read_bytes) {
  const static int LEN = 100;
  struct buf *frame = create_test_buf(LEN);

  EXPECT_LT(read_bytes(frame, -1, NULL, LEN), 0);
  EXPECT_EQ(read_bytes(frame, 0, NULL, LEN), LEN);
  EXPECT_LT(read_bytes(frame, 1, NULL, LEN), 0);

  destroy_test_buf(frame);
}

TEST(wire, checked_read_bytes) {
  const static int LEN = 100;
  struct buf *frame = create_test_buf(LEN);

  int expect_fail = 0;
  READ_BYTES(frame, 0, NULL, LEN);
  EXPECT_FALSE(expect_fail);

  expect_fail = 1;
  READ_BYTES(frame, 1, NULL, LEN);
  EXPECT_FALSE(expect_fail); // we should not be here

wire_error:
  EXPECT_TRUE(expect_fail);
  destroy_test_buf(frame);
}

TEST(wire, read_int_string) {
  char name[] = "owl";
	char read[4];
	struct buf *frame = create_test_buf(1 + strlen(name));

	write_u8(frame, 0, strlen(name));
	write_bytes(frame, 1, (uint8_t *) name, strlen(name));

  EXPECT_EQ(read_int_string(frame, 0, read, 0), 1);
  EXPECT_STREQ(read, "");
  EXPECT_EQ(read_int_string(frame, 0, read, 1), 2);
  EXPECT_STREQ(read, "o");
  EXPECT_EQ(read_int_string(frame, 0, read, 2), 3);
  EXPECT_STREQ(read, "ow");
  EXPECT_EQ(read_int_string(frame, 0, read, 3), 4);
  EXPECT_STREQ(read, "owl");
  EXPECT_EQ(read_int_string(frame, 0, read, sizeof(read)), 4);
  EXPECT_STREQ(read, name);

  destroy_test_buf(frame);
}

TEST(wire, checked_read_int_string) {
  char name[] = "owl";
  char read[4];
	struct buf *frame = create_test_buf(1 + strlen(name));

	write_u8(frame, 0, strlen(name));
	write_bytes(frame, 1, (uint8_t *) name, strlen(name));

  int expect_fail = 0;
  READ_INT_STRING(frame, 0, NULL, sizeof(read));
  EXPECT_FALSE(expect_fail);

  expect_fail = 1;
  READ_INT_STRING(frame, buf_len(frame), NULL, sizeof(read));
  EXPECT_FALSE(expect_fail); // we should not be here

wire_error:
  EXPECT_TRUE(expect_fail);
  destroy_test_buf(frame);
}

TEST(wire, write_read_u8) {
  struct buf *frame = create_test_buf(1);
  uint8_t write = 42;
  uint8_t read = 0;

  EXPECT_EQ(write_u8((struct buf *)frame, 0, write), 1);
  EXPECT_EQ(read_u8(frame, 0, &read), 1);
  EXPECT_EQ(read, write);

  destroy_test_buf(frame);
}

TEST(wire, write_read_le16) {
  struct buf *frame = create_test_buf(2);
  uint16_t write = 42;
  uint16_t read = 0;

  EXPECT_EQ(write_le16((struct buf *)frame, 0, write), 2);
  EXPECT_EQ(read_le16(frame, 0, &read), 2);
  EXPECT_EQ(read, write);

  destroy_test_buf(frame);
}

TEST(wire, write_read_be16) {
  struct buf *frame = create_test_buf(2);
  uint16_t write = 42;
  uint16_t read = 0;

  EXPECT_EQ(write_be16((struct buf *)frame, 0, write), 2);
  EXPECT_EQ(read_be16(frame, 0, &read), 2);
  EXPECT_EQ(read, write);

  destroy_test_buf(frame);
}

TEST(wire, write_le16_read_be16) {
  struct buf *frame = create_test_buf(2);
  uint16_t write = 42;
  uint16_t read = 0;

  EXPECT_EQ(write_le16((struct buf *)frame, 0, write), 2);
  EXPECT_EQ(read_be16(frame, 0, &read), 2);
  EXPECT_NE(read, write);

  destroy_test_buf(frame);
}

TEST(wire, write_be16_read_le16) {
  struct buf *frame = create_test_buf(2);
  uint16_t write = 42;
  uint16_t read = 0;

  EXPECT_EQ(write_be16((struct buf *)frame, 0, write), 2);
  EXPECT_EQ(read_le16(frame, 0, &read), 2);
  EXPECT_NE(read, write);

  destroy_test_buf(frame);
}

TEST(wire, write_read_le32) {
  struct buf *frame = create_test_buf(4);
  uint32_t write = 42;
  uint32_t read = 0;

  EXPECT_EQ(write_le32((struct buf *)frame, 0, write), 4);
  EXPECT_EQ(read_le32(frame, 0, &read), 4);
  EXPECT_EQ(read, write);

  destroy_test_buf(frame);
}

TEST(wire, write_read_be32) {
  struct buf *frame = create_test_buf(4);
  uint32_t write = 42;
  uint32_t read = 0;

  EXPECT_EQ(write_be32((struct buf *)frame, 0, write), 4);
  EXPECT_EQ(read_be32(frame, 0, &read), 4);
  EXPECT_EQ(read, write);

  destroy_test_buf(frame);
}

TEST(wire, write_le32_read_be32) {
  struct buf *frame = create_test_buf(4);
  uint32_t write = 42;
  uint32_t read = 0;

  EXPECT_EQ(write_le32((struct buf *)frame, 0, write), 4);
  EXPECT_EQ(read_be32(frame, 0, &read), 4);
  EXPECT_NE(read, write);

  destroy_test_buf(frame);
}

TEST(wire, write_be32_read_le32) {
  struct buf *frame = create_test_buf(4);
  uint32_t write = 42;
  uint32_t read = 0;

  EXPECT_EQ(write_be32((struct buf *)frame, 0, write), 4);
  EXPECT_EQ(read_le32(frame, 0, &read), 4);
  EXPECT_NE(read, write);

  destroy_test_buf(frame);
}

TEST(wire, write_read_ether_addr) {
  struct buf *frame = create_test_buf(6);
  struct ether_addr write = {{ 0x01, 0x02, 0x03, 0x04, 0x05 }};
  struct ether_addr read = {{ 0x00, 0x00, 0x00, 0x00, 0x00 }};

  EXPECT_EQ(write_ether_addr((struct buf *)frame, 0, &write), 6);
  EXPECT_EQ(read_ether_addr(frame, 0, &read), 6);
  for (int i = 0; i < 6; i++)
    EXPECT_EQ(read.ether_addr_octet[i], write.ether_addr_octet[i]);

  destroy_test_buf(frame);
}

TEST(wire, write_read_bytes) {
  const static uint8_t LEN = 100;

  struct buf *frame = create_test_buf(LEN);
  uint8_t write[LEN];
  const uint8_t *read;

  for (uint8_t i = 0; i < LEN; i++)
    write[i] = i;

  EXPECT_EQ(write_bytes((struct buf *)frame, 0, write, LEN), LEN);
  EXPECT_EQ(read_bytes(frame, 0, &read, LEN), LEN);
  for (int i = 0; i < LEN; i++)
    EXPECT_EQ(read[i], write[i]);

  destroy_test_buf(frame);
}
