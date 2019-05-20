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

#ifndef AWDL_STATE_H_
#define AWDL_STATE_H_

#include <stdint.h>
#include <stdbool.h>

#include "wire.h"
#include "frame.h"
#include "peers.h"
#include "sync.h"
#include "channel.h"

#define RSSI_THRESHOLD_DEFAULT -65
#define RSSI_GRACE_DEFAULT      -5

struct awdl_state; /* forward declaration for tlv_cb */

typedef void (*awdl_tlv_cb)(struct awdl_peer *, uint8_t, const struct buf *, struct awdl_state *, void *);

struct awdl_stats {
	uint64_t tx_action;
	uint64_t tx_data;
	uint64_t tx_data_unicast;
	uint64_t tx_data_multicast;
	uint64_t rx_action;
	uint64_t rx_data;
	uint64_t rx_unknown;
};

/* Complete node state */
struct awdl_state {
	struct ether_addr self_address;
	char name[HOST_NAME_LENGTH_MAX + 1];

	uint8_t version;
	uint8_t dev_class;

	/* sequence number for data frames */
	uint16_t sequence_number;
	/* PSF interval (in TU) */
	uint16_t psf_interval;
	/* destination address for action frames */
	struct ether_addr dst;

	/* Allows to hook TLV reception */
	awdl_tlv_cb tlv_cb;
	void *tlv_cb_data;

	/* Allows to hook adding of new neighbor */
	awdl_peer_cb peer_cb;
	void *peer_cb_data;

	/* Allows to hook removing of new neighbor */
	awdl_peer_cb peer_remove_cb;
	void *peer_remove_cb_data;

	int filter_rssi; /* whether or not to filter (default: true) */
	signed char rssi_threshold; /* peers exceeding this threshold are discovered */
	signed char rssi_grace; /* once discovered accept lower RSSI */

	struct awdl_election_state election;
	struct awdl_sync_state sync;
	struct awdl_channel_state channel;
	struct awdl_peer_state peers;
	struct awdl_stats stats;
};

void awdl_stats_init(struct awdl_stats *stats);

void awdl_init_state(struct awdl_state *state, const char *hostname, const struct ether_addr *self,
                     struct awdl_chan chan, uint64_t now);

uint16_t awdl_state_next_sequence_number(struct awdl_state *);

struct ieee80211_state {
	/* IEEE 802.11 sequence number */
	uint16_t sequence_number;
	/* whether we need to add an fcs */
	bool fcs;
};

void ieee80211_init_state(struct ieee80211_state *);

unsigned int ieee80211_state_next_sequence_number(struct ieee80211_state *);

uint64_t clock_time_us();

#endif /* AWDL_STATE_H_ */
