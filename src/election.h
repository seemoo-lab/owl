#ifndef AWDL_ELECTION_H_
#define AWDL_ELECTION_H_

#include "ieee80211.h"

#define AWDL_ELECTION_TREE_MAX_HEIGHT 10 /* arbitrary limit */
#define AWDL_ELECTION_METRIC_INIT 60
#define AWDL_ELECTION_COUNTER_INIT 0

struct awdl_peer_state;

struct awdl_election_state {
	struct ether_addr master_addr;
	struct ether_addr sync_addr;
	struct ether_addr self_addr;
	uint32_t height;
	uint32_t master_metric;
	uint32_t self_metric;
	uint32_t master_counter;
	uint32_t self_counter;
};

int awdl_election_is_sync_master(const struct awdl_election_state *state, const struct ether_addr *addr);

void awdl_election_state_init(struct awdl_election_state *state, const struct ether_addr *self);

void awdl_election_run(struct awdl_election_state *state, const struct awdl_peer_state *peers);

int awdl_election_tree_print(const struct awdl_election_state *state, char *str, size_t len);

/* Util functions */
int compare_ether_addr(const struct ether_addr *a, const struct ether_addr *b);

#endif /* AWDL_ELECTION_H_ */
