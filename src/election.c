#include <string.h>
#include <stdio.h>

#include "election.h"
#include "peers.h"
#include "log.h"

static inline int compare(uint32_t a, uint32_t b) {
	return (a < b) ? -1 : (a > b);
}

int compare_ether_addr(const struct ether_addr *a, const struct ether_addr *b) {
	return memcmp(a, b, sizeof(struct ether_addr));
}

int awdl_election_is_sync_master(const struct awdl_election_state *state, const struct ether_addr *addr) {
	return !compare_ether_addr(&state->sync_addr, addr);
}

static void awdl_election_reset_metric(struct awdl_election_state *state) {
	state->self_counter = AWDL_ELECTION_COUNTER_INIT;
	state->self_metric = AWDL_ELECTION_METRIC_INIT;
}

static void awdl_election_reset_self(struct awdl_election_state *state) {
	state->height = 0;
	state->master_addr = state->self_addr;
	state->sync_addr = state->self_addr;
	state->master_metric = state->self_metric;
	state->master_counter = state->self_counter;
}

void awdl_election_state_init(struct awdl_election_state *state, const struct ether_addr *self) {
	state->master_addr = *self;
	state->sync_addr = *self;
	state->self_addr = *self;
	awdl_election_reset_metric(state);
	awdl_election_reset_self(state);
}

static int awdl_election_compare_master(const struct awdl_election_state *a, const struct awdl_election_state *b) {
	int result = compare(a->master_counter, b->master_counter);
	if (!result)
		result = compare(a->master_metric, b->master_metric);
	return result;
}

void awdl_election_run(struct awdl_election_state *state, const struct awdl_peer_state *peers) {
	struct awdl_peer *peer;
	struct ether_addr old_top_master = state->master_addr;
	struct ether_addr old_sync_master = state->sync_addr;
	struct awdl_election_state *master_state = state;
	awdl_peers_it_t it = awdl_peers_it_new(peers->peers);

	awdl_election_reset_self(state);

	/* FIXME probably not fully correct */
	while (awdl_peers_it_next(it, &peer) == PEERS_OK) {
		int cmp_metric;
		struct awdl_election_state *peer_state = &peer->election;
		if (peer_state->height + 1 > AWDL_ELECTION_TREE_MAX_HEIGHT) {
			log_debug("Ignore peer %s because sync tree would get too large (%u, max %u)",
			          ether_ntoa(&peer_state->self_addr), peer_state->height + 1, AWDL_ELECTION_TREE_MAX_HEIGHT);
			continue; /* reject: tree would get too large if accepted as sync master */
		}
		if (awdl_election_is_sync_master(peer_state, &state->self_addr))
			continue; /* reject: do not allow cycles in sync tree */
		cmp_metric = awdl_election_compare_master(peer_state, master_state);
		if (cmp_metric < 0) {
			continue; /* reject: lower top master metric */
		} else if (cmp_metric == 0) {
			/* if metric is same, compare distance to master */
			if (peer_state->height > master_state->height) {
				continue; /* reject: do not increase length of sync tree */
			} else if (peer_state->height == master_state->height) {
				/* if height is same, need tie breaker */
				if (compare_ether_addr(&peer_state->self_addr, &master_state->self_addr) <= 0)
					continue; /* reject: peer has smaller address */
			}
		}
		/* accept: otherwise */
		master_state = peer_state;
	}

	if (state != master_state) { /* adopt new master */
		state->master_addr = master_state->master_addr;
		state->sync_addr = master_state->self_addr;
		state->master_metric = master_state->master_metric;
		state->master_counter = master_state->master_counter;
		state->height = master_state->height + 1;
	}

	if (compare_ether_addr(&old_top_master, &state->master_addr) ||
	    compare_ether_addr(&old_sync_master, &state->sync_addr)) {
		char tree[256];
		awdl_election_tree_print(state, tree, sizeof(tree));
		log_debug("new election tree: %s", tree);
	}
}

int awdl_election_tree_print(const struct awdl_election_state *state, char *str, size_t len) {
	char *cur = str, *const end = str + len;
	cur += snprintf(cur, end - cur, "%s", ether_ntoa(&state->self_addr));
	if (state->height > 0)
		cur += snprintf(cur, end - cur, " -> %s", ether_ntoa(&state->sync_addr));
	if (state->height > 1) {
		cur += snprintf(cur, end - cur, " ");
		for (uint32_t i = 1; i < state->height; i++)
			cur += snprintf(cur, end - cur, "-"); /* one dash for every intermediate hop */
		cur += snprintf(cur, end - cur, "> %s", ether_ntoa(&state->master_addr));
	}
	cur += snprintf(cur, end - cur, " (met %u, ctr %u)", state->master_metric, state->master_counter);
	return cur - str;
}
