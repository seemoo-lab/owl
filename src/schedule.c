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

#include "schedule.h"

double usec_to_sec(uint64_t usec) {
	return usec / 1000000.;
}

uint64_t sec_to_usec(double sec) {
	return sec * 1000000;
}

bool awdl_same_channel_as_peer(const struct awdl_state *state, uint64_t now, const struct awdl_peer *peer) {
	int own_slot, peer_slot;
	int own_chan, peer_chan;

	own_slot = awdl_sync_current_eaw(now, &state->sync) % AWDL_CHANSEQ_LENGTH;
	peer_slot = awdl_sync_current_eaw(now + peer->sync_offset, &state->sync) % AWDL_CHANSEQ_LENGTH;

	own_chan = awdl_chan_num(state->channel.sequence[own_slot], state->channel.enc);
	peer_chan = awdl_chan_num(peer->sequence[peer_slot], state->channel.enc);

	return own_chan && (own_chan == peer_chan);
}

int awdl_is_multicast_eaw(const struct awdl_state *state, uint64_t now) {
	uint16_t slot = awdl_sync_current_eaw(now, &state->sync) % AWDL_CHANSEQ_LENGTH;
	return slot == 0 || slot == 10;
}

double awdl_can_send_in(const struct awdl_state *state, uint64_t now, int guard) {
	uint64_t next_aw = awdl_sync_next_aw_us(now, &state->sync);
	uint64_t _guard = ieee80211_tu_to_usec(guard);
	uint64_t eaw = ieee80211_tu_to_usec(64);

	return (next_aw < _guard) ? -usec_to_sec(_guard - next_aw) : ((eaw - next_aw < _guard) ? usec_to_sec(
		(_guard - (eaw - next_aw))) : 0);
}

double awdl_can_send_unicast_in(const struct awdl_state *state, const struct awdl_peer *peer, uint64_t now, int guard) {
	uint64_t next_aw = awdl_sync_next_aw_us(now, &state->sync);
	uint64_t _guard = ieee80211_tu_to_usec(guard);
	uint64_t eaw = ieee80211_tu_to_usec(64);

	if (!awdl_same_channel_as_peer(state, now, peer))
		return usec_to_sec(next_aw); /* try again in the next slot */

	if (next_aw < _guard) { /* we are at the end of slot */
		if (awdl_same_channel_as_peer(state, now + eaw, peer)) {
			return 0; /* we are on the same channel in next slot, ignore guard */
		} else {
			return -usec_to_sec(_guard - next_aw);
		}
	} else if (eaw - next_aw < _guard) {
		if (awdl_same_channel_as_peer(state, now - eaw, peer)) {
			return 0; /* we were on the same channel last slot, ignore guard */
		} else {
			return usec_to_sec(_guard - (eaw - next_aw));
		}
	} else {
		return 0; /* we are inside guard interval */
	}
}
