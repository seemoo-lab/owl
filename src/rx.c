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

#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <radiotap.h>
#include <radiotap_iter.h>

#include "rx.h"
#include "sync.h"
#include "wire.h"
#include "log.h"

#define AWDL_SYNC_THRESHOLD 3

int awdl_handle_sync_params_tlv(struct awdl_peer *src, const struct buf *val, struct awdl_state *state, uint64_t now) {
	uint16_t aw_counter_master;
	uint16_t time_to_next_aw_master;
	int64_t sync_err_tu;

	if (!awdl_election_is_sync_master(&state->election, &src->addr))
		return RX_IGNORE; /* ignore sync params from nodes that are not our master */

	/* TODO synchronization could be more accurate */

	READ_LE16(val, 1, &time_to_next_aw_master);
	READ_LE16(val, 29, &aw_counter_master);

	state->sync.meas_total++;
	sync_err_tu = awdl_sync_error_tu(now, time_to_next_aw_master, aw_counter_master, &state->sync);
	if (sync_err_tu > AWDL_SYNC_THRESHOLD || sync_err_tu < -AWDL_SYNC_THRESHOLD) {
		state->sync.meas_err++;
		log_trace("Sync error %d TU (%.02f %%)", sync_err_tu, state->sync.meas_err * 100.0 / state->sync.meas_total);
	}
	awdl_sync_update_last(now, time_to_next_aw_master, aw_counter_master, &state->sync);

	return RX_OK;
wire_error:
	return RX_TOO_SHORT;
}

int awdl_handle_chanseq_tlv(struct awdl_peer *src, const struct buf *val,
                            struct awdl_state *state __attribute__((unused))) {
	uint8_t count;
	uint8_t encoding;
	uint8_t duplicate_count;
	uint8_t step_count;
	uint16_t fill_channel;
	struct awdl_chan list[AWDL_CHANSEQ_LENGTH];
	int size;

	/* read and sanity check, we do not support any other configuration */
	READ_U8(val, 0, &count);
	if (count + 1 != AWDL_CHANSEQ_LENGTH)
		return RX_UNEXPECTED_VALUE;
	READ_U8(val, 2, &duplicate_count);
	if (duplicate_count > 0)
		return RX_UNEXPECTED_VALUE;
	READ_U8(val, 3, &step_count);
	if (step_count + 1 != state->sync.presence_mode)
		return RX_UNEXPECTED_VALUE;
	READ_LE16(val, 4, &fill_channel);
	if (fill_channel != 0xffff)
		return RX_UNEXPECTED_VALUE;

	READ_U8(val, 1, &encoding);
	size = awdl_chan_encoding_size(encoding);
	if (size < 1)
		return RX_UNEXPECTED_VALUE;

	for (int i = 0, offset = 6; i < AWDL_CHANSEQ_LENGTH; i++, offset += size)
		READ_BYTES_COPY(val, offset, list[i].val, size);

	if (memcmp(src->sequence, list, sizeof(list))) {
		log_debug("peer %s (%s) changed channel sequence to %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
		          ether_ntoa(&src->addr), src->name,
		          awdl_chan_num(list[0], encoding), awdl_chan_num(list[1], encoding),
		          awdl_chan_num(list[2], encoding), awdl_chan_num(list[3], encoding),
		          awdl_chan_num(list[4], encoding), awdl_chan_num(list[5], encoding),
		          awdl_chan_num(list[6], encoding), awdl_chan_num(list[7], encoding),
		          awdl_chan_num(list[8], encoding), awdl_chan_num(list[9], encoding),
		          awdl_chan_num(list[10], encoding), awdl_chan_num(list[11], encoding),
		          awdl_chan_num(list[12], encoding), awdl_chan_num(list[13], encoding),
		          awdl_chan_num(list[14], encoding), awdl_chan_num(list[15], encoding));
		memcpy(src->sequence, list, sizeof(list));
	}
	return RX_OK;

wire_error:
	return RX_TOO_SHORT;
}

int awdl_handle_election_params_tlv(struct awdl_peer *src, const struct buf *val,
                                    struct awdl_state *state __attribute__((unused))) {
	uint8_t distance_to_master;

	if (src->supports_v2)
		return 0; /* if node supports election v2, we ignore v1 advertisements */

	// READ_U8(val, 0, 0); /* ignore flags */
	// READ_LE16(val, 1, 0); /* TODO ignore ID */
	READ_U8(val, 3, &distance_to_master);
	src->election.height = distance_to_master; // READ_U8(val, 3, &src->election.height);
	// READ_U8(val, 4, 0); /* ignore unknown */
	READ_ETHER_ADDR(val, 5, &src->election.master_addr);
	READ_LE32(val, 11, &src->election.master_metric);
	READ_LE32(val, 15, &src->election.self_metric);
	// READ_LE16(val, 19, 0); /* padding */

	return RX_OK;
wire_error:
	return RX_TOO_SHORT;
}

int awdl_handle_election_params_v2_tlv(struct awdl_peer *src, const struct buf *val,
                                       struct awdl_state *state __attribute__((unused))) {
	READ_LE32(val, 16, &src->election.height);
	READ_ETHER_ADDR(val, 0, &src->election.master_addr);
	READ_ETHER_ADDR(val, 6, &src->election.sync_addr);
	READ_LE32(val, 12, &src->election.master_counter);
	READ_LE32(val, 20, &src->election.master_metric);
	READ_LE32(val, 24, &src->election.self_metric);
	// READ_LE32(val, 28, &tlv->unknown); /* ignore unknown */
	// READ_LE32(val, 32, &tlv->reserved); /* ignore reserved */
	READ_LE32(val, 36, &src->election.self_counter);

	src->supports_v2 = 1;
	return RX_OK;
wire_error:
	return RX_TOO_SHORT;
}

int awdl_handle_arpa_tlv(struct awdl_peer *src, const struct buf *val,
                         struct awdl_state *state __attribute__((unused))) {
	// READ_U8(val, 0, &flags); /* semantics unclear, ignore */
	READ_INT_STRING(val, 1, src->name, HOST_NAME_LENGTH_MAX);
	return RX_OK;
wire_error:
	return RX_TOO_SHORT;
}

int awdl_handle_data_path_state_tlv(struct awdl_peer *src, const struct buf *val,
                                    struct awdl_state *state __attribute__((unused))) {
	uint16_t flags;
	int offset = 0;
	READ_LE16(val, offset, &flags);
	offset += 2;

	if (flags & AWDL_DATA_PATH_FLAG_COUNTRY_CODE) {
		READ_BYTES_COPY(val, offset, (uint8_t *) src->country_code, 3);
		offset += 3;
	}
	if (flags & AWDL_DATA_PATH_FLAG_SOCIAL_CHANNEL_MAP) {
		READ_LE16(val, offset, NULL /* supported social channels */);
		offset += 2;
	}
	if (flags & AWDL_DATA_PATH_FLAG_INFRA_INFO) {
		READ_ETHER_ADDR(val, offset, NULL /* BSSID */);
		offset += 6;
		READ_LE16(val, offset, NULL /* channel */);
		offset += 2;
	}
	if (flags & AWDL_DATA_PATH_FLAG_INFRA_ADDRESS) {
		READ_ETHER_ADDR(val, offset, &src->infra_addr);
		offset += 6;
	}
	if (flags & AWDL_DATA_PATH_FLAG_AWDL_ADDRESS) {
		READ_ETHER_ADDR(val, offset, NULL /* AWDL address */);
		offset += 6;
	}

	/* TODO complete parsing */
	return RX_OK;
wire_error:
	return RX_TOO_SHORT;
}

int awdl_handle_version_tlv(struct awdl_peer *src, const struct buf *val,
                            struct awdl_state *state __attribute__((unused))) {
	uint8_t version, devclass;
	READ_U8(val, 0, &version);
	READ_U8(val, 1, &devclass);
	src->version = version;
	src->devclass = devclass;
	return RX_OK;
wire_error:
	return RX_TOO_SHORT;
}

int awdl_handle_tlv(struct awdl_peer *src, uint8_t type, const struct buf *val,
                    struct awdl_state *state, uint64_t tsft) {
	switch (type) {
		case AWDL_SYNCHRONIZATON_PARAMETERS_TLV:
			return awdl_handle_sync_params_tlv(src, val, state, tsft);
		case AWDL_CHAN_SEQ_TLV:
			return awdl_handle_chanseq_tlv(src, val, state);
		case AWDL_ELECTION_PARAMETERS_TLV:
			return awdl_handle_election_params_tlv(src, val, state);
		case AWDL_ELECTION_PARAMETERS_V2_TLV:
			return awdl_handle_election_params_v2_tlv(src, val, state);
		case AWDL_ARPA_TLV:
			return awdl_handle_arpa_tlv(src, val, state);
		case AWDL_DATA_PATH_STATE_TLV:
			return awdl_handle_data_path_state_tlv(src, val, state);
		case AWDL_VERSION_TLV:
			return awdl_handle_version_tlv(src, val, state);
		case AWDL_SYNCTREE_TLV: /* seems to be buggy, not used in our election process */
			/* fall through */
		default:
			log_trace("awdl: not handling %s (%u)", awdl_tlv_as_str(type), type);
			return RX_IGNORE;
	}
}

int awdl_parse_action_hdr(const struct buf *frame) {
	const struct awdl_action *af;
	int valid;

	READ_BYTES(frame, 0, NULL, sizeof(struct awdl_action));

	af = (const struct awdl_action *) buf_data(frame);

	valid = af->category == IEEE80211_VENDOR_SPECIFIC &&
	        !memcmp(&af->oui, &AWDL_OUI, sizeof(AWDL_OUI)) &&
	        af->type == AWDL_TYPE &&
	        af->version == AWDL_VERSION_COMPAT;

	if (valid && (af->subtype == AWDL_ACTION_PSF || af->subtype == AWDL_ACTION_MIF))
		return af->subtype;

wire_error:
	return -1;
}

int awdl_rx_action(const struct buf *frame, signed char rssi, uint64_t tsft,
                   const struct ether_addr *src, const struct ether_addr *dst,
                   struct awdl_state *state) {
	int len;
	enum peers_status status;
	uint8_t tlv_type;
	uint16_t tlv_len;
	const uint8_t *tlv_value;
	struct awdl_peer *peer;
	int subtype;

	(void) dst; /* TODO ignore destination address for now, could be used to mitigate desynchronization attack */

	subtype = awdl_parse_action_hdr(frame);
	if (subtype < 0) {
		log_trace("awdl_action: not an action frame"); /* could be block ACK */
		return RX_IGNORE;
	}
	buf_strip(frame, sizeof(struct awdl_action));

	if (state->filter_rssi) {
		status = awdl_peer_get(state->peers.peers, src, NULL);
		if (((status == PEERS_OK) && rssi < state->rssi_threshold + state->rssi_grace) ||
		    ((status == PEERS_MISSING) && rssi < state->rssi_threshold))
			return RX_IGNORE_RSSI;
	}
	state->stats.rx_action++;

	/* Update peer table */
	status = awdl_peer_add(state->peers.peers, src, tsft, state->peer_cb, state->peer_cb_data);
	if (status < 0) {
		log_warn("awdl_action: could not add peer: %s (%d)", ether_ntoa(src), status);
		return RX_IGNORE;
	}
	status = awdl_peer_get(state->peers.peers, src, &peer);
	if (status < 0) {
		log_warn("awdl_action: could not find peer: %s (%d)", ether_ntoa(src), status);
		return RX_IGNORE; /* FIXME happens sometimes, not sure why */
	}

	log_trace("awdl_action: receive %s from %s (rssi %d)", awdl_frame_as_str(subtype), ether_ntoa(&peer->addr), rssi);

	while ((len = read_tlv(frame, 0, &tlv_type, &tlv_len, &tlv_value)) > 0) {
		const struct buf *tlv_buf = buf_new_const(tlv_value, tlv_len);
		int result = awdl_handle_tlv(peer, tlv_type, tlv_buf, state, tsft);
		if (state->tlv_cb)
			state->tlv_cb(peer, tlv_type, tlv_buf, state, state->tlv_cb_data);
		buf_free(tlv_buf);
		if (result < 0) {
			log_warn("awdl_action: parsing error %s", awdl_tlv_as_str(tlv_type));
			return RX_UNEXPECTED_FORMAT;
		}
		buf_strip(frame, len);
	}

	if (buf_len(frame) > 0) {
		log_debug("awdl_action: unexpected bytes (%d) at end of frame", buf_len(frame));
		return RX_UNEXPECTED_FORMAT;
	}

	if (subtype == AWDL_ACTION_MIF)
		peer->sent_mif = 1;

	/* update peer info after parsing all TLVs */
	awdl_peer_add(state->peers.peers, src, tsft, state->peer_cb, state->peer_cb_data);

	return RX_OK;
}

int llc_parse(const struct buf *frame, struct llc_hdr *llc) {
	uint16_t pid; /* to not take pointer of packed llc->pid */
	READ_U8(frame, 0, &llc->dsap);
	READ_U8(frame, 1, &llc->ssap);
	READ_U8(frame, 2, &llc->control);
	READ_BYTES_COPY(frame, 3, (uint8_t *) &llc->oui, 3);
	READ_BE16(frame, 6, &pid);
	llc->pid = pid;
	return RX_OK;
wire_error:
	return RX_TOO_SHORT;
}

int awdl_valid_llc_header(const struct buf *frame) {
	struct llc_hdr llc;

	if (llc_parse(frame, &llc) < 0) {
		log_warn("llc: frame to short");
		return 0;
	}

	if (llc.dsap != 0xaa || llc.ssap != 0xaa || llc.control != 0x03 ||
	    llc.pid != AWDL_LLC_PROTOCOL_ID) {
		log_warn("llc: invalid header (DSAP %#04x, SSAP %#04x, control %#04x, OUI %02x:%02x:%02x, PID %#06x",
		         llc.dsap, llc.ssap, llc.control,
		         llc.oui.byte[0], llc.oui.byte[1], llc.oui.byte[2], llc.pid);
		return 0;
	}
	return 1;
}

int awdl_rx_data(const struct buf *frame, struct buf ***out, const struct ether_addr *src,
                 const struct ether_addr *dst, struct awdl_state *state) {
	uint16_t ether_type;
	int offset = 0;

	log_trace("awdl_data: receive from %s", ether_ntoa(src));
	state->stats.rx_data++;

	if (awdl_peer_get(state->peers.peers, src, NULL) != PEERS_OK)
		return RX_IGNORE_PEER;

	if (!awdl_valid_llc_header(frame))
		return RX_UNEXPECTED_FORMAT;
	buf_strip(frame, sizeof(struct llc_hdr));

	if ((unsigned int) buf_len(frame) < sizeof(struct awdl_data)) {
		log_warn("awdl_data: frame too short");
		return RX_TOO_SHORT;
	}

	/* create ethernet frame */
	read_be16(frame, 6, &ether_type);
	buf_strip(frame, sizeof(struct awdl_data));

	**out = buf_new_owned(ETHER_MAX_LEN);

	/* TODO use checked write methods */
	offset += write_ether_addr(**out, offset, dst);
	offset += write_ether_addr(**out, offset, src);
	offset += write_be16(**out, offset, ether_type);
	/* TODO avoid copying bytes */
	offset += write_bytes(**out, offset, buf_data(frame), buf_len(frame));

	(*out)++;

	return RX_OK;
}

int awdl_rx_data_amsdu(const struct buf *frame, struct buf ***out, const struct ether_addr *src __attribute__((unused)),
                       const struct ether_addr *dst __attribute__((unused)), struct awdl_state *state) {
	/* Iterate over all subframes */
	while (buf_len(frame) > 0) {
		struct ether_addr src_a, dst_a;
		uint16_t len_a;
		int err;
		const struct buf *subframe;
		READ_ETHER_ADDR(frame, 0, &dst_a);
		READ_ETHER_ADDR(frame, 6, &src_a);
		READ_BE16(frame, 12, &len_a);
		BUF_STRIP(frame, 14); /* strip subframe header */
		if (len_a > buf_len(frame))
			return RX_TOO_SHORT;
		/* create subview of buf */
		subframe = buf_new_const(buf_data(frame), len_a);
		err = awdl_rx_data(subframe, out, &src_a, &dst_a, state);
		buf_free(subframe);
		if (err < 0)
			return err;
		BUF_STRIP(frame, len_a); /* strip frame */
		if (buf_len(frame) > 0)
			BUF_STRIP(frame, (4 - ((14 + len_a) % 4)) % 4); /* strip padding */
	}
	return RX_OK;
wire_error:
	return RX_TOO_SHORT;
}

static int radiotap_parse(const struct buf *frame, signed char *rssi, uint8_t *flags, uint64_t *tsft) {
	struct ieee80211_radiotap_iterator iter;
	int err;

	err = ieee80211_radiotap_iterator_init(&iter, (struct ieee80211_radiotap_header *) buf_data(frame),
	                                       buf_len(frame), NULL);
	if (err < 0)
		return RX_UNEXPECTED_FORMAT;

	while (!(err = ieee80211_radiotap_iterator_next(&iter))) {
		if (iter.is_radiotap_ns) {
			switch (iter.this_arg_index) {
				case IEEE80211_RADIOTAP_TSFT: /* https://www.radiotap.org/fields/TSFT.html */
					if (tsft)
						*tsft = le64toh(*(uint64_t *) iter.this_arg);
					break;
				case IEEE80211_RADIOTAP_FLAGS:
					if (flags)
						*flags = *(unsigned char *) iter.this_arg;
					break;
				case IEEE80211_RADIOTAP_DBM_ANTSIGNAL: /* https://www.radiotap.org/fields/Antenna%20signal.html */
					if (rssi)
						*rssi = *(signed char *) iter.this_arg;
				default:
					/* ignore */
					break;
			}
		}
	}

	if (err != -ENOENT)
		return RX_UNEXPECTED_FORMAT;

	return RX_OK;
}

static int check_fcs(const struct buf *frame, uint8_t radiotap_flags) {
	if (radiotap_flags & IEEE80211_RADIOTAP_F_BADFCS)
		return -1;
	if (radiotap_flags & IEEE80211_RADIOTAP_F_FCS)
		BUF_TAKE(frame, 4); /* strip FCS */
	return 0;
wire_error:
	return -1;
}

int awdl_rx(const struct buf *frame, struct buf ***data_frame, struct awdl_state *state) {
	const struct ieee80211_hdr *ieee80211;
	const struct ether_addr *from, *to;
	uint16_t fc, qosc; /* frame and QoS control */
	signed char rssi;
	uint64_t tsft;
	uint8_t flags;

	tsft = clock_time_us(); /* TODO Radiotap TSFT is more accurate but then need to access TSF in clock_time_us() */
	if (radiotap_parse(frame, &rssi, &flags, NULL /* &tsft */) < 0)
		return RX_UNEXPECTED_FORMAT;
	BUF_STRIP(frame, le16toh(((const struct ieee80211_radiotap_header *) buf_data(frame))->it_len));

	if (check_fcs(frame, flags)) /* note that if no flags are present (flags==0), frames will pass */
		return RX_IGNORE_FAILED_CRC;

	READ_BYTES(frame, 0, NULL, sizeof(struct ieee80211_hdr));

	ieee80211 = (const struct ieee80211_hdr *) (buf_data(frame));
	from = &ieee80211->addr2;
	to = &ieee80211->addr1;
	fc = le16toh(ieee80211->frame_control);

	if (!memcmp(from, &state->self_address, sizeof(struct ether_addr)))
		return RX_IGNORE_FROM_SELF; /* TODO ignore frames from self, should be filtered at pcap level */

	//if (!(to->ether_addr_octet[0] & 0x01) && memcmp(to, &state->self_address, sizeof(struct ether_addr)))
	//	return RX_IGNORE_NOPROMISC; /* neither broadcast/multicast nor unicast to me */

	BUF_STRIP(frame, sizeof(struct ieee80211_hdr));

	/* Processing based on frame type and subtype */
	switch (fc & (IEEE80211_FCTL_FTYPE | IEEE80211_FCTL_STYPE)) {
		case IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION:
			return awdl_rx_action(frame, rssi, tsft, from, to, state);
		case IEEE80211_FTYPE_DATA | IEEE80211_STYPE_DATA | IEEE80211_STYPE_QOS_DATA:
			READ_LE16(frame, 0, &qosc);
			BUF_STRIP(frame, IEEE80211_QOS_CTL_LEN);
			/* TODO should handle block acks if required (IEEE80211_QOS_CTL_ACK_POLICY_XYZ) */
			if (qosc & IEEE80211_QOS_CTL_A_MSDU_PRESENT)
				return awdl_rx_data_amsdu(frame, data_frame, from, to, state);
			/* else fall through */
		case IEEE80211_FTYPE_DATA | IEEE80211_STYPE_DATA:
			return awdl_rx_data(frame, data_frame, from, to, state);
		default:
			log_warn("ieee80211: cannot handle type %x and subtype %x of received frame from %s",
			         fc & IEEE80211_FCTL_FTYPE, fc & IEEE80211_FCTL_STYPE, ether_ntoa(from));
			return RX_UNEXPECTED_TYPE;
	}
wire_error:
	return RX_TOO_SHORT;
}
