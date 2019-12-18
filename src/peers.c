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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "peers.h"
#include "hashmap.h"
#include "log.h"

#define PEERS_DEFAULT_TIMEOUT        2000000 /* in ms */
#define PEERS_DEFAULT_CLEAN_INTERVAL 1000000 /* in ms */

void awdl_peer_state_init(struct awdl_peer_state *state) {
	state->peers = awdl_peers_init();
	state->timeout = PEERS_DEFAULT_TIMEOUT;
	state->clean_interval = PEERS_DEFAULT_CLEAN_INTERVAL;
}

awdl_peers_t awdl_peers_init() {
	return (awdl_peers_t) hashmap_new(sizeof(struct ether_addr));
}

void awdl_peers_free(awdl_peers_t peers) {
	struct awdl_peer *peer;
	map_t map = (map_t) peers;

	/* Remove all allocated peers */
	map_it_t it = hashmap_it_new(map);
	while (hashmap_it_next(it, NULL, (any_t *) &peer) == MAP_OK) {
		hashmap_it_remove(it);
		free(peer);
	}
	hashmap_it_free(it);

	/* Remove hashmap itself */
	hashmap_free(peers);
}

int awdl_peers_length(awdl_peers_t peers) {
	map_t map = (map_t) peers;
	return hashmap_length(map);
}

static int awdl_peer_is_valid(const struct awdl_peer *peer) {
	return peer->sent_mif && peer->devclass && peer->version;
}

struct awdl_peer *awdl_peer_new(const struct ether_addr *addr) {
	struct awdl_peer *peer = (struct awdl_peer *) malloc(sizeof(struct awdl_peer));
	*(struct ether_addr *) &peer->addr = *addr;
	peer->last_update = 0;
	awdl_election_state_init(&peer->election, addr);
	awdl_chanseq_init_static(peer->sequence, &CHAN_NULL);
	peer->sync_offset = 0;
	peer->devclass = 0;
	peer->version = 0;
	peer->supports_v2 = 0;
	peer->sent_mif = 0;
	strcpy(peer->name, "");
	strcpy(peer->country_code, "NA");
	peer->is_valid = 0;
	return peer;
}

enum peers_status
awdl_peer_add(awdl_peers_t peers, const struct ether_addr *_addr, uint64_t now, awdl_peer_cb cb, void *arg) {
	int status, result;
	map_t map = (map_t) peers;
	mkey_t addr = (mkey_t) _addr;
	struct awdl_peer *peer;
	status = hashmap_get(map, addr, (any_t *) &peer, 0 /* do not remove */);
	if (status == MAP_MISSING)
		peer = awdl_peer_new(_addr); /* create new entry */

	/* update */
	peer->last_update = now;

	if (status == MAP_OK) {
		result = PEERS_UPDATE;
		goto out;
	}

	status = hashmap_put(map, (mkey_t) &peer->addr, peer);
	if (status != MAP_OK) {
		free(peer);
		return PEERS_INTERNAL;
	}
	result = PEERS_OK;
out:
	if (!peer->is_valid && awdl_peer_is_valid(peer)) {
		/* peer has turned valid */
		peer->is_valid = 1;
		log_info("add peer %s (%s)", ether_ntoa(&peer->addr), peer->name);
		if (cb)
			cb(peer, arg);
	}
	return result;
}

enum peers_status awdl_peer_remove(awdl_peers_t peers, const struct ether_addr *_addr, awdl_peer_cb cb, void *arg) {
	int status;
	map_t map = (map_t) peers;
	mkey_t addr = (mkey_t) _addr;
	struct awdl_peer *peer;
	status = hashmap_get(map, addr, (any_t *) &peer, 1 /* remove */);
	if (status == MAP_MISSING)
		return PEERS_MISSING;
	if (peer->is_valid) {
		log_info("remove peer %s (%s)", ether_ntoa(&peer->addr), peer->name);
		if (cb)
			cb(peer, arg);
	}
	free(peer);
	return PEERS_OK;
}

enum peers_status awdl_peer_get(awdl_peers_t peers, const struct ether_addr *_addr, struct awdl_peer **peer) {
	int status;
	map_t map = (map_t) peers;
	mkey_t addr = (mkey_t) _addr;
	status = hashmap_get(map, addr, (any_t *) peer, 0 /* keep */);
	if (status == MAP_MISSING)
		return PEERS_MISSING;
	return PEERS_OK;
}

int awdl_peer_print(const struct awdl_peer *peer, char *str, int len) {
	char *cur = str, *const end = str + len;
	if (strlen(peer->name))
		cur += snprintf(cur, cur < end ? end - cur : 0, "%s: ", peer->name);
	else
		cur += snprintf(cur, cur < end ? end - cur : 0, "<UNNAMED>: ");
    cur += awdl_election_tree_print(&peer->election, cur, end - cur);
	return cur - str;
}

int awdl_peers_print(awdl_peers_t peers, char *str, int len) {
	char *cur = str, *const end = str + len;
	map_t map = (map_t) peers;
	map_it_t it = hashmap_it_new(map);
	struct awdl_peer *peer;

	while (hashmap_it_next(it, NULL, (any_t *) &peer) == MAP_OK) {
        cur += awdl_peer_print(peer, cur, end - cur);
        cur += snprintf(cur, cur < end ? end - cur : 0, "\n");
    }

	hashmap_it_free(it);
	return cur - str;
}

void awdl_peers_remove(awdl_peers_t peers, uint64_t before, awdl_peer_cb cb, void *arg) {
	map_t map = (map_t) peers;
	map_it_t it = hashmap_it_new(map);
	struct awdl_peer *peer;

	while (hashmap_it_next(it, NULL, (any_t *) &peer) == MAP_OK) {
		if (peer->last_update < before) {
			if (peer->is_valid) {
				log_info("remove peer %s (%s)", ether_ntoa(&peer->addr), peer->name);
				if (cb)
					cb(peer, arg);
			}
			hashmap_it_remove(it);
			free(peer);
		}
	}
	hashmap_it_free(it);
}

awdl_peers_it_t awdl_peers_it_new(awdl_peers_t in) {
	return (awdl_peers_it_t) hashmap_it_new((map_t) in);
}

enum peers_status awdl_peers_it_next(awdl_peers_it_t it, struct awdl_peer **peer) {
	int s = hashmap_it_next((map_it_t) it, 0, (any_t *) peer);
	if (s == MAP_OK)
		return PEERS_OK;
	else
		return PEERS_MISSING;
}

enum peers_status awdl_peers_it_remove(awdl_peers_it_t it) {
	int s = hashmap_it_remove((map_it_t) it);
	if (s == MAP_OK)
		return PEERS_OK;
	else
		return PEERS_MISSING;
}

void awdl_peers_it_free(awdl_peers_it_t it) {
	hashmap_it_free((map_it_t) it);
}
