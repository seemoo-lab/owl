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

#ifndef AWDL_PEERS_H
#define AWDL_PEERS_H

#include <stdint.h>
#include <net/ethernet.h>

#include "election.h"
#include "channel.h"

#define HOST_NAME_LENGTH_MAX 64

enum peers_status {
	PEERS_UPDATE = 1, /* Peer updated */
	PEERS_OK = 0, /* New peer added */
	PEERS_MISSING = -1, /* Peer does not exist */
	PEERS_INTERNAL = -2, /* Internal error */
};

struct awdl_peer {
	const struct ether_addr addr;
	uint64_t last_update;
	struct awdl_election_state election;
	struct awdl_chan sequence[AWDL_CHANSEQ_LENGTH];
	uint64_t sync_offset;
	char name[HOST_NAME_LENGTH_MAX + 1]; /* space for trailing zero */
	char country_code[2 + 1];
	struct ether_addr infra_addr;
	uint8_t version;
	uint8_t devclass;
	uint8_t supports_v2 : 1;
	uint8_t sent_mif : 1;
	uint8_t is_valid : 1;
};

typedef void (*awdl_peer_cb)(struct awdl_peer *, void *arg);

typedef void *awdl_peers_t;

struct awdl_peer_state {
	awdl_peers_t peers;
	uint64_t timeout;
	uint64_t clean_interval;
};

void awdl_peer_state_init(struct awdl_peer_state *state);

awdl_peers_t awdl_peers_init();

void awdl_peers_free(awdl_peers_t peers);

int awdl_peers_length(awdl_peers_t peers);

enum peers_status
awdl_peer_add(awdl_peers_t peers, const struct ether_addr *addr, uint64_t now, awdl_peer_cb cb, void *arg);

enum peers_status awdl_peer_remove(awdl_peers_t peers, const struct ether_addr *addr, awdl_peer_cb cb, void *arg);

enum peers_status awdl_peer_get(awdl_peers_t peers, const struct ether_addr *addr, struct awdl_peer **peer);

int awdl_peer_print(const struct awdl_peer *peer, char *str, int len);

/**
 * Apply callback to and then remove all peers matching a filter
 * @param peers the awdl_peers_t instance
 * @param before remove all entries with an last_update timestamp {@code before}
 * @param cb the callback function, can be NULL
 * @param arg will be passed to callback function
 */
void awdl_peers_remove(awdl_peers_t peers, uint64_t before, awdl_peer_cb cb, void *arg);

int awdl_peers_print(awdl_peers_t peers, char *str, int len);

/* Iterator functions */
typedef void *awdl_peers_it_t;

awdl_peers_it_t awdl_peers_it_new(awdl_peers_t in);

enum peers_status awdl_peers_it_next(awdl_peers_it_t it, struct awdl_peer **peer);

enum peers_status awdl_peers_it_remove(awdl_peers_it_t it);

void awdl_peers_it_free(awdl_peers_it_t it);

#endif /* ADWL_PEERS_H */
