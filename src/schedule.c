#include "schedule.h"

double usec_to_sec(uint64_t usec) {
	return usec / 1000000.;
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
		return next_aw; /* try again in the next slot */

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
