extern "C" {
#include "peers.h"
}

#include "gtest/gtest.h"

static const struct ether_addr TEST_ADDR = {{ 0xc0, 0xff, 0xee,  0xc0, 0xff, 0xee }};
static const struct ether_addr TEST_ADDR2 = {{ 0xc0, 0xff, 0xff, 0xc0, 0xff, 0xff }};

TEST(awdl_peers, init_free) {
	awdl_peers_t p = awdl_peers_init();
	EXPECT_TRUE(p);
	EXPECT_EQ(awdl_peers_length(p), 0);
	awdl_peers_free(p);
}

TEST(awdl_peers, add) {
	int s;
	awdl_peers_t p = awdl_peers_init();
	s = awdl_peer_add(p, &TEST_ADDR, 0, NULL, NULL);
	EXPECT_EQ(s, PEERS_OK);
	EXPECT_EQ(awdl_peers_length(p), 1);
	awdl_peers_free(p);
}

TEST(awdl_peers, add_two) {
	int s;
	awdl_peers_t p = awdl_peers_init();
	s = awdl_peer_add(p, &TEST_ADDR, 0, NULL, NULL);
	EXPECT_EQ(s, PEERS_OK);
	EXPECT_EQ(awdl_peers_length(p), 1);
	s = awdl_peer_add(p, &TEST_ADDR2, 0, NULL, NULL);
	EXPECT_EQ(s, PEERS_OK);
	EXPECT_EQ(awdl_peers_length(p), 2);
	awdl_peers_free(p);
}

TEST(awdl_peers, add_same) {
	int s;
	awdl_peers_t p = awdl_peers_init();
	s = awdl_peer_add(p, &TEST_ADDR, 0, NULL, NULL);
	EXPECT_EQ(s, PEERS_OK);
	EXPECT_EQ(awdl_peers_length(p), 1);
	s = awdl_peer_add(p, &TEST_ADDR, 0, NULL, NULL);
	EXPECT_EQ(s, PEERS_UPDATE);
	EXPECT_EQ(awdl_peers_length(p), 1);
	awdl_peers_free(p);
}

TEST(awdl_peers, remove) {
	int s;
	awdl_peers_t p = awdl_peers_init();
	awdl_peer_add(p, &TEST_ADDR, 0, NULL, NULL);
	s = awdl_peer_remove(p, &TEST_ADDR, NULL, NULL);
	EXPECT_EQ(s, PEERS_OK);
	EXPECT_EQ(awdl_peers_length(p), 0);
	awdl_peers_free(p);
}

TEST(awdl_peers, remove_empty) {
	int s;
	awdl_peers_t p = awdl_peers_init();
	s = awdl_peer_remove(p, &TEST_ADDR, NULL, NULL);
	EXPECT_EQ(s, PEERS_MISSING);
	EXPECT_EQ(awdl_peers_length(p), 0);
	awdl_peers_free(p);
}

TEST(awdl_peers, remove_twice) {
	int s;
	awdl_peers_t p = awdl_peers_init();
	awdl_peer_add(p, &TEST_ADDR, 0, NULL, NULL);
	awdl_peer_remove(p, &TEST_ADDR, NULL, NULL);
	s = awdl_peer_remove(p, &TEST_ADDR, NULL, NULL);
	EXPECT_EQ(s, PEERS_MISSING);
	EXPECT_EQ(awdl_peers_length(p), 0);
	awdl_peers_free(p);
}

static int count_cb = 0;

static void test_cb(struct awdl_peer *p, void *ignore) {
	(void) ignore;
	count_cb++;
	EXPECT_TRUE(!memcmp(&TEST_ADDR, &p->addr, sizeof(struct ether_addr)));
}

TEST(awdl_peers, remove_timedout) {
	struct awdl_peer *peer = NULL;
	uint64_t now = 0;
	awdl_peers_t p = awdl_peers_init();
	awdl_peer_add(p, &TEST_ADDR, now, NULL, NULL);
	awdl_peer_get(p, &TEST_ADDR, &peer);
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
