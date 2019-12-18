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
#include "peers.h"
}

#include "gtest/gtest.h"

#define TEST_ADDR(i) static const struct ether_addr TEST_ADDR##i = {{ i, i, i, i, i, i }}

TEST_ADDR(0);
TEST_ADDR(1);

TEST(awdl_peers, init_free) {
	awdl_peers_t p = awdl_peers_init();
	EXPECT_TRUE(p);
	EXPECT_EQ(awdl_peers_length(p), 0);
	awdl_peers_free(p);
}

TEST(awdl_peers, add) {
	int s;
	awdl_peers_t p = awdl_peers_init();
	s = awdl_peer_add(p, &TEST_ADDR0, 0, NULL, NULL);
	EXPECT_EQ(s, PEERS_OK);
	EXPECT_EQ(awdl_peers_length(p), 1);
	awdl_peers_free(p);
}

TEST(awdl_peers, add_two) {
	int s;
	awdl_peers_t p = awdl_peers_init();
	s = awdl_peer_add(p, &TEST_ADDR0, 0, NULL, NULL);
	EXPECT_EQ(s, PEERS_OK);
	EXPECT_EQ(awdl_peers_length(p), 1);
	s = awdl_peer_add(p, &TEST_ADDR1, 0, NULL, NULL);
	EXPECT_EQ(s, PEERS_OK);
	EXPECT_EQ(awdl_peers_length(p), 2);
	awdl_peers_free(p);
}

TEST(awdl_peers, add_same) {
	int s;
	awdl_peers_t p = awdl_peers_init();
	s = awdl_peer_add(p, &TEST_ADDR0, 0, NULL, NULL);
	EXPECT_EQ(s, PEERS_OK);
	EXPECT_EQ(awdl_peers_length(p), 1);
	s = awdl_peer_add(p, &TEST_ADDR0, 0, NULL, NULL);
	EXPECT_EQ(s, PEERS_UPDATE);
	EXPECT_EQ(awdl_peers_length(p), 1);
	awdl_peers_free(p);
}

TEST(awdl_peers, remove) {
	int s;
	awdl_peers_t p = awdl_peers_init();
	awdl_peer_add(p, &TEST_ADDR0, 0, NULL, NULL);
	s = awdl_peer_remove(p, &TEST_ADDR0, NULL, NULL);
	EXPECT_EQ(s, PEERS_OK);
	EXPECT_EQ(awdl_peers_length(p), 0);
	awdl_peers_free(p);
}

TEST(awdl_peers, remove_empty) {
	int s;
	awdl_peers_t p = awdl_peers_init();
	s = awdl_peer_remove(p, &TEST_ADDR0, NULL, NULL);
	EXPECT_EQ(s, PEERS_MISSING);
	EXPECT_EQ(awdl_peers_length(p), 0);
	awdl_peers_free(p);
}

TEST(awdl_peers, remove_twice) {
	int s;
	awdl_peers_t p = awdl_peers_init();
	awdl_peer_add(p, &TEST_ADDR0, 0, NULL, NULL);
	awdl_peer_remove(p, &TEST_ADDR0, NULL, NULL);
	s = awdl_peer_remove(p, &TEST_ADDR0, NULL, NULL);
	EXPECT_EQ(s, PEERS_MISSING);
	EXPECT_EQ(awdl_peers_length(p), 0);
	awdl_peers_free(p);
}

static int count_cb = 0;

static void test_cb(struct awdl_peer *p, void *ignore) {
	(void) ignore;
	count_cb++;
	EXPECT_TRUE(!memcmp(&TEST_ADDR0, &p->addr, sizeof(struct ether_addr)));
}

TEST(awdl_peers, remove_timedout) {
	struct awdl_peer *peer = NULL;
	uint64_t now = 0;
	awdl_peers_t p = awdl_peers_init();
	awdl_peer_add(p, &TEST_ADDR0, now, NULL, NULL);
	awdl_peer_get(p, &TEST_ADDR0, &peer);
	peer->is_valid = 1;
	EXPECT_EQ(awdl_peers_length(p), 1);
	awdl_peers_remove(p, now, test_cb, NULL);
	EXPECT_EQ(awdl_peers_length(p), 1);
	EXPECT_EQ(count_cb, 0);
	now++;
	awdl_peers_remove(p, now, test_cb, NULL);
	EXPECT_EQ(awdl_peers_length(p), 0);
	EXPECT_EQ(count_cb, 1);
	awdl_peers_free(p);
}

TEST(awdl_peers, print) {
	char buf[1000]; // Buffer is large enough
	awdl_peers_t p = awdl_peers_init();
	awdl_peer_add(p, &TEST_ADDR0, 0, NULL, NULL);
	awdl_peer_add(p, &TEST_ADDR1, 0, NULL, NULL);
	int len = awdl_peers_print(p, buf, sizeof(buf));
	EXPECT_EQ(len, strlen(buf));
	EXPECT_STREQ(buf, "<UNNAMED>: 0:0:0:0:0:0 (met 60, ctr 0)\n"
	                  "<UNNAMED>: 1:1:1:1:1:1 (met 60, ctr 0)\n");
}

TEST(awdl_peers, print_overflow) {
	char buf[30]; // Buffer is too small
	awdl_peers_t p = awdl_peers_init();
	awdl_peer_add(p, &TEST_ADDR0, 0, NULL, NULL);
	awdl_peer_add(p, &TEST_ADDR1, 0, NULL, NULL);
	int len = awdl_peers_print(p, buf, sizeof(buf));
	EXPECT_GT(len, strlen(buf)); // would have written more
	EXPECT_EQ(strlen(buf), sizeof(buf) - 1); // buffer is full
    EXPECT_STREQ(buf, "<UNNAMED>: 0:0:0:0:0:0 (met 6");
}
