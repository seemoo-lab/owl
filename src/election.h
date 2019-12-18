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

int awdl_election_tree_print(const struct awdl_election_state *state, char *str, int len);

/* Util functions */
int compare_ether_addr(const struct ether_addr *a, const struct ether_addr *b);

#endif /* AWDL_ELECTION_H_ */
