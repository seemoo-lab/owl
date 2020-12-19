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

#include <stdint.h>
#include <string.h>

#include <radiotap.h>

#include "tx.h"
#include "sync.h"
#include "ieee80211.h"

#include "crc32.h"

#define AWDL_DNS_SHORT_LOCAL  0xc00c  /* .local */

#define AWDL_DATA_HEAD        0x0403
#define AWDL_DATA_PAD         0x0000

#define AWDL_SOCIAL_CHANNEL_6_BIT   0x0001
#define AWDL_SOCIAL_CHANNEL_44_BIT  0x0002
#define AWDL_SOCIAL_CHANNEL_149_BIT 0x0004

int awdl_init_data(uint8_t *buf, struct awdl_state *state) {
	struct awdl_data *header = (struct awdl_data *) buf;

	header->head = htole16(AWDL_DATA_HEAD);
	header->seq = htole16(awdl_state_next_sequence_number(state));
	header->pad = htole16(AWDL_DATA_PAD);
	/* TODO we should generally to able to send any kind of data */
	header->ethertype = htobe16(ETH_P_IPV6);

	return sizeof(struct awdl_data);
}

int awdl_init_action(uint8_t *buf, enum awdl_action_type type) {
	struct awdl_action *af = (struct awdl_action *) buf;

	uint32_t steady_time = (uint32_t) clock_time_us(); /* TODO awdl_init_* methods should not call clock_time_us() themselves but rather be given as parameter to avoid inconsistent calculation of stuff, e.g., fields in sync parameters should be calculated based on the timestamp indicated in af->target_tx */
	af->category = IEEE80211_VENDOR_SPECIFIC; /* vendor specific */
	af->oui = AWDL_OUI;
	af->type = AWDL_TYPE;
	af->version = AWDL_VERSION_COMPAT;
	af->subtype = type;
	af->reserved = 0;
	af->phy_tx = htole32(steady_time); /* TODO arbitrary offset */
	af->target_tx = htole32(steady_time);

	return sizeof(struct awdl_action);
}

int awdl_chan_encoding_length(enum awdl_chan_encoding enc) {
	switch (enc) {
		case AWDL_CHAN_ENC_SIMPLE:
			return 1;
		case AWDL_CHAN_ENC_LEGACY:
		case AWDL_CHAN_ENC_OPCLASS:
			return 2;
		default:
			return 0; /* unknown encoding */
	}
}

int awdl_init_chanseq(uint8_t *buf, const struct awdl_state *state) {
	struct awdl_chanseq *chanseq = (struct awdl_chanseq *) buf;
	int enc_len = awdl_chan_encoding_length(state->channel.enc);
	int offset = sizeof(struct awdl_chanseq);

	chanseq->count = AWDL_CHANSEQ_LENGTH - 1; /* +1 */
	chanseq->encoding = state->channel.enc;
	chanseq->duplicate_count = 0;
	chanseq->step_count = 3;
	chanseq->fill_channel = htole16(0xffff); /* repeat current channel */
	for (int i = 0; i < AWDL_CHANSEQ_LENGTH; i++) {
		/* copy based on length */
		memcpy(buf + offset, &(state->channel.sequence[i]), enc_len);
		offset += enc_len;
	}
	return offset;
}

int awdl_init_sync_params_tlv(uint8_t *buf, const struct awdl_state *state) {
	struct awdl_sync_params_tlv *tlv = (struct awdl_sync_params_tlv *) buf;
	int len;
	uint64_t now;

	now = clock_time_us();

	tlv->type = AWDL_SYNCHRONIZATON_PARAMETERS_TLV;

	/* static info: appears to be fixed in current AWDL implementation */
	tlv->guard_time = 0;
	tlv->aw_period = htole16(state->sync.aw_period);
	tlv->aw_ext_length = htole16(state->sync.aw_period);
	tlv->aw_com_length = htole16(state->sync.aw_period);
	tlv->min_ext = state->sync.presence_mode - 1;
	tlv->max_ext_unicast = state->sync.presence_mode - 1;
	tlv->max_ext_multicast = state->sync.presence_mode - 1;
	tlv->max_ext_af = state->sync.presence_mode - 1;

	tlv->flags = htole16(0x1800); /* TODO: not sure what they do */

	tlv->reserved = 0;

	/* TODO: dynamic info needs to be adjusted during runtime */
	tlv->next_aw_channel = awdl_chan_num(state->channel.current,
	                                     state->channel.enc); /* TODO need to calculate this from current seq */
	tlv->next_aw_seq = htole16(awdl_sync_current_aw(now, &state->sync));
	tlv->ap_alignment = tlv->next_aw_seq; /* awdl_fill_sync_params(..) */
	tlv->tx_down_counter = htole16(awdl_sync_next_aw_tu(now, &state->sync));
	tlv->af_period = htole16(state->psf_interval);
	tlv->presence_mode = state->sync.presence_mode; /* always available, usually 4 */
	tlv->master_addr = state->election.master_addr;
	tlv->master_channel = awdl_chan_num(state->channel.master, state->channel.enc);
	/* lower bound is 0 */
	if (le16toh(tlv->aw_com_length) < (le16toh(tlv->aw_period) * tlv->presence_mode - le16toh(tlv->tx_down_counter)))
		tlv->remaining_aw_length = htole16(0);
	else
		tlv->remaining_aw_length = htole16(le16toh(tlv->aw_com_length) -
		                                   (le16toh(tlv->aw_period) * tlv->presence_mode - le16toh(tlv->tx_down_counter)));

	len = sizeof(struct awdl_sync_params_tlv);
	len += awdl_init_chanseq(buf + sizeof(struct awdl_sync_params_tlv), state);
	/* padding */
	buf[len++] = 0;
	buf[len++] = 0;

	tlv->length = htole16(len - sizeof(struct tl));

	return len;
}

int awdl_init_chanseq_tlv(uint8_t *buf, const struct awdl_state *state) {
	struct awdl_chanseq_tlv *tlv = (struct awdl_chanseq_tlv *) buf;
	int len;

	tlv->type = AWDL_CHAN_SEQ_TLV;

	len = sizeof(struct awdl_chanseq_tlv);
	len += awdl_init_chanseq(buf + sizeof(struct awdl_chanseq_tlv), state);
	/* padding */
	buf[len++] = 0;
	buf[len++] = 0;
	buf[len++] = 0;
	tlv->length = htole16(len - sizeof(struct tl));

	return len;
}

int awdl_init_election_params_tlv(uint8_t *buf, const struct awdl_state *state) {
	struct awdl_election_params_tlv *tlv = (struct awdl_election_params_tlv *) buf;

	tlv->type = AWDL_ELECTION_PARAMETERS_TLV;
	tlv->length = htole16(sizeof(struct awdl_election_params_tlv) - sizeof(struct tl));

	tlv->distancetop = state->election.height;
	tlv->top_master_addr = state->election.master_addr;
	tlv->top_master_metric = htole32(state->election.master_metric);
	tlv->self_metric = htole32(state->election.self_metric);

	/* TODO unsure what flags do */
	tlv->flags = 0;
	tlv->id = htole16(0);

	tlv->unknown = 0;
	tlv->pad[0] = 0;
	tlv->pad[1] = 0;

	return sizeof(struct awdl_election_params_tlv);
}

int awdl_init_election_params_v2_tlv(uint8_t *buf, const struct awdl_state *state) {
	struct awdl_election_params_v2_tlv *tlv = (struct awdl_election_params_v2_tlv *) buf;

	tlv->type = AWDL_ELECTION_PARAMETERS_V2_TLV;
	tlv->length = htole16(sizeof(struct awdl_election_params_v2_tlv) - sizeof(struct tl));

	tlv->master_addr = state->election.master_addr;
	tlv->sync_addr = state->election.sync_addr;
	tlv->distance_to_master = htole32(state->election.height);
	tlv->master_metric = htole32(state->election.master_metric);
	tlv->self_metric = htole32(state->election.self_metric);
	tlv->unknown = htole32(0);
	tlv->reserved = htole32(0);

	tlv->master_counter = htole32(state->election.master_counter);
	tlv->self_counter = htole32(state->election.self_counter);

	return sizeof(struct awdl_election_params_v2_tlv);
}

int awdl_init_service_params_tlv(uint8_t *buf, const struct awdl_state *state __attribute__((unused))) {
	struct awdl_service_params_tlv *tlv = (struct awdl_service_params_tlv *) buf;

	tlv->type = AWDL_SERVICE_PARAMETERS_TLV;
	tlv->length = htole16(sizeof(struct awdl_service_params_tlv) - sizeof(struct tl));

	/* changing the SUI causes receiving nodes to flush their mDNS cache,
	 * other than that, this TLV seems to be deprecated in AWDL v2+ */

	tlv->unknown[0] = 0;
	tlv->unknown[1] = 0;
	tlv->unknown[2] = 0;
	tlv->sui = htole16(0);
	tlv->bitmask = htole32(0);

	return sizeof(struct awdl_service_params_tlv);
}

int awdl_init_ht_capabilities_tlv(uint8_t *buf, const struct awdl_state *state __attribute__((unused))) {
	struct awdl_ht_capabilities_tlv *tlv = (struct awdl_ht_capabilities_tlv *) buf;

	tlv->type = AWDL_ENHANCED_DATA_RATE_CAPABILITIES_TLV;
	tlv->length = htole16(sizeof(struct awdl_ht_capabilities_tlv) - sizeof(struct tl));

	/* TODO extract capabilities from nl80211 (nl80211_band_attr) */
	tlv->ht_capabilities = htole16(0x11ce);
	tlv->ampdu_params = 0x1b;
	tlv->rx_mcs = 0xff; /* one stream, all MCS */

	tlv->unknown = htole16(0);
	tlv->unknown2 = htole16(0);

	return sizeof(struct awdl_ht_capabilities_tlv);
}

int awdl_init_data_path_state_tlv(uint8_t *buf, const struct awdl_state *state) {
	struct awdl_data_path_state_tlv *tlv = (struct awdl_data_path_state_tlv *) buf;

	tlv->type = AWDL_DATA_PATH_STATE_TLV;
	tlv->length = htole16(sizeof(struct awdl_data_path_state_tlv) - sizeof(struct tl));

	/* TODO this is very ugly currently */
	tlv->flags = htole16(0x8f24);

	tlv->awdl_addr = state->self_address;

	tlv->country_code[0] = 'X';
	tlv->country_code[1] = '0';
	tlv->country_code[2] = 0;

	if (awdl_chan_num(state->channel.master, AWDL_CHAN_ENC_OPCLASS) == 6)
		tlv->social_channels = htole16(AWDL_SOCIAL_CHANNEL_6_BIT);
	else if (awdl_chan_num(state->channel.master, AWDL_CHAN_ENC_OPCLASS) == 44) {
		tlv->social_channels = htole16(AWDL_SOCIAL_CHANNEL_44_BIT);
	} else { // 149
		tlv->social_channels = htole16(AWDL_SOCIAL_CHANNEL_149_BIT);
	}

	tlv->ext_flags = htole16(0x0000);

	return sizeof(struct awdl_data_path_state_tlv);
}

int awdl_init_arpa_tlv(uint8_t *buf, const struct awdl_state *state) {
	struct awdl_arpa_tlv *tlv = (struct awdl_arpa_tlv *) buf;
	size_t len = strlen(state->name);

	tlv->type = AWDL_ARPA_TLV;
	tlv->length = htole16((uint16_t) (sizeof(struct awdl_arpa_tlv) - sizeof(struct tl) + len + 1));

	tlv->flags = 3;
	tlv->name_length = (uint8_t) len;
	memcpy(tlv->name, state->name, len);
	*(uint16_t *) (tlv->name + len) = htobe16(AWDL_DNS_SHORT_LOCAL);

	return sizeof(struct tl) + le16toh(tlv->length);
}

int awdl_init_version_tlv(uint8_t *buf, const struct awdl_state *state) {
	struct awdl_version_tlv *tlv = (struct awdl_version_tlv *) buf;

	tlv->type = AWDL_VERSION_TLV;
	tlv->length = htole16(sizeof(struct awdl_version_tlv) - sizeof(struct tl));
	tlv->version = state->version;
	tlv->devclass = state->dev_class;

	return sizeof(struct awdl_version_tlv);
}

int ieee80211_init_radiotap_header(uint8_t *buf) {
	/*
	 * TX radiotap headers and mac80211
	 * https://www.kernel.org/doc/Documentation/networking/mac80211-injection.txt
	 */
	struct ieee80211_radiotap_header *hdr = (struct ieee80211_radiotap_header *) buf;
	uint8_t *ptr = buf + sizeof(struct ieee80211_radiotap_header);
	uint32_t present = 0;

	hdr->it_version = 0;
	hdr->it_pad = 0;

	/* TODO Adjust PHY parameters based on receiver capabilities */

	present |= ieee80211_radiotap_type_to_mask(IEEE80211_RADIOTAP_RATE);
	*ptr = htole16(ieee80211_radiotap_rate_to_val(12));
	ptr += sizeof(uint8_t);

	hdr->it_len = htole16((uint16_t) (ptr - buf));
	hdr->it_present = htole32(present);

	return ptr - buf;
}

int ieee80211_init_awdl_hdr(uint8_t *buf, const struct ether_addr *src, const struct ether_addr *dst,
                            struct ieee80211_state *state, uint16_t type) {
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *) buf;

	hdr->frame_control = htole16(type);
	hdr->duration_id = htole16(0);
	hdr->addr1 = *dst;
	hdr->addr2 = *src;
	hdr->addr3 = AWDL_BSSID;
	hdr->seq_ctrl = htole16(ieee80211_state_next_sequence_number(state) << 4);

	return sizeof(struct ieee80211_hdr);
}

int ieee80211_init_awdl_action_hdr(uint8_t *buf, const struct ether_addr *src, const struct ether_addr *dst,
                                   struct ieee80211_state *state) {
	return ieee80211_init_awdl_hdr(buf, src, dst, state, IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);
}

int ieee80211_init_awdl_data_hdr(uint8_t *buf, const struct ether_addr *src, const struct ether_addr *dst,
                                 struct ieee80211_state *state) {
	return ieee80211_init_awdl_hdr(buf, src, dst, state, IEEE80211_FTYPE_DATA | IEEE80211_STYPE_DATA);
}

int llc_init_awdl_hdr(uint8_t *buf) {
	struct llc_hdr *hdr = (struct llc_hdr *) buf;

	hdr->dsap = 0xaa; /* 0xaa - SNAP Extension Used */
	hdr->ssap = 0xaa; /* 0xaa - SNAP Extension Used */
	hdr->control = 0x03;
	hdr->oui = AWDL_OUI;
	hdr->pid = htobe16(AWDL_LLC_PROTOCOL_ID);

	return sizeof(struct llc_hdr);
}

int ieee80211_add_fcs(const uint8_t *start, uint8_t *end) {
	uint32_t crc = crc32(start, end - start);
	*(uint32_t *) end = htole32(crc);
	return sizeof(uint32_t);
}

int awdl_init_full_action_frame(uint8_t *buf, struct awdl_state *state, struct ieee80211_state *ieee80211_state,
                                enum awdl_action_type type) {
	uint8_t *ptr = buf;

	ptr += ieee80211_init_radiotap_header(ptr);
	ptr += ieee80211_init_awdl_action_hdr(ptr, &state->self_address, &state->dst, ieee80211_state);
	ptr += awdl_init_action(ptr, type);
	ptr += awdl_init_sync_params_tlv(ptr, state);
	ptr += awdl_init_election_params_tlv(ptr, state);
	ptr += awdl_init_chanseq_tlv(ptr, state);
	ptr += awdl_init_election_params_v2_tlv(ptr, state);
	ptr += awdl_init_service_params_tlv(ptr, state);
	if (type == AWDL_ACTION_MIF)
		ptr += awdl_init_ht_capabilities_tlv(ptr, state);
	if (type == AWDL_ACTION_MIF)
		ptr += awdl_init_arpa_tlv(ptr, state);
	ptr += awdl_init_data_path_state_tlv(ptr, state);
	ptr += awdl_init_version_tlv(ptr, state);
	if (ieee80211_state->fcs)
		ptr += ieee80211_add_fcs(buf, ptr);

	return ptr - buf;
}

int awdl_init_full_data_frame(uint8_t *buf, const struct ether_addr *src, const struct ether_addr *dst,
                              const uint8_t *payload, unsigned int plen,
                              struct awdl_state *state, struct ieee80211_state *ieee80211_state) {
	uint8_t *ptr = buf;

	ptr += ieee80211_init_radiotap_header(ptr);
	ptr += ieee80211_init_awdl_data_hdr(ptr, src, dst, ieee80211_state);
	ptr += llc_init_awdl_hdr(ptr);
	ptr += awdl_init_data(ptr, state);
	memcpy(ptr, payload, plen);
	ptr += plen;
	if (ieee80211_state->fcs)
		ptr += ieee80211_add_fcs(buf, ptr);

	return ptr - buf;
}
