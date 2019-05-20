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

#ifndef AWDL_FRAME_H_
#define AWDL_FRAME_H_

#include <stdint.h>

#include "ieee80211.h"

#define AWDL_LLC_PROTOCOL_ID  0x0800

#define AWDL_OUI (struct oui){{ 0x00, 0x17, 0xf2 }}
#define AWDL_BSSID (struct ether_addr){{ 0x00, 0x25, 0x00, 0xff, 0x94, 0x73 }}

#define IEEE80211_VENDOR_SPECIFIC 127
#define AWDL_VERSION_COMPAT awdl_version(1, 0)
#define AWDL_TYPE 8

struct awdl_data {
	uint16_t head; /* AWDL_DATA_HEAD */
	uint16_t seq;
	uint16_t pad; /* AWDL_DATA_PAD */
	uint16_t ethertype;
} __attribute__((__packed__));

struct awdl_action {
	uint8_t category; /* 127 - vendor-specific */
	struct oui oui;
	uint8_t type;
	uint8_t version;
	uint8_t subtype; /* awdl_frame_types */
	uint8_t reserved;
	uint32_t phy_tx;
	uint32_t target_tx;
} __attribute__((__packed__));

enum awdl_action_type {
	AWDL_ACTION_PSF = 0,
	AWDL_ACTION_MIF = 3,
};

const char *awdl_frame_as_str(uint8_t type);

/* AWDL TLV type values */
enum awdl_tlvs {
	AWDL_SSTH_REQUEST_TLV = 0, /* assumed deprecated */
	AWDL_SERVICE_REQUEST_TLV = 1, /* assumed deprecated */
	AWDL_SERVICE_RESPONSE_TLV = 2,
	AWDL_SYNCHRONIZATON_PARAMETERS_TLV = 4,
	AWDL_ELECTION_PARAMETERS_TLV = 5,
	AWDL_SERVICE_PARAMETERS_TLV = 6,
	AWDL_ENHANCED_DATA_RATE_CAPABILITIES_TLV = 7,
	AWDL_ENHANCED_DATA_RATE_OPERATION_TLV = 8, /* assumed deprecated */
	AWDL_INFRA_TLV = 9, /* assumed deprecated */
	AWDL_INVITE_TLV = 10, /* assumed deprecated */
	AWDL_DBG_STRING_TLV = 11, /* assumed deprecated */
	AWDL_DATA_PATH_STATE_TLV = 12,
	AWDL_ENCAPSULATED_IP_TLV = 13, /* assumed deprecated */
	AWDL_DATAPATH_DEBUG_PACKET_LIVE_TLV = 14, /* assumed deprecated */
	AWDL_DATAPATH_DEBUG_AF_LIVE_TLV = 15, /* assumed deprecated */
	AWDL_ARPA_TLV = 16,
	AWDL_IEEE80211_CNTNR_TLV = 17,
	AWDL_CHAN_SEQ_TLV = 18,
	AWDL_SYNCTREE_TLV = 20,
	AWDL_VERSION_TLV = 21,
	AWDL_BLOOM_FILTER_TLV = 22,
	AWDL_NAN_SYNC_TLV = 23,
	AWDL_ELECTION_PARAMETERS_V2_TLV = 24,
};

struct tl {
	uint8_t type;
	uint16_t length;
} __attribute__((__packed__));

struct awdl_chanseq {
	uint8_t count;
	uint8_t encoding;
	uint8_t duplicate_count;
	uint8_t step_count;
	uint16_t fill_channel;
} __attribute__((__packed__));


struct awdl_sync_params_tlv {
	uint8_t type;
	uint16_t length;
	uint8_t next_aw_channel;
	uint16_t tx_down_counter;
	uint8_t master_channel;
	uint8_t guard_time;
	uint16_t aw_period; /* AW period in TUs */
	uint16_t af_period; /* how often action frames are sent out in TUs */
	uint16_t flags;
	uint16_t aw_ext_length; /* length of an extended AW in TUs */
	uint16_t aw_com_length; /* length of a regular AW in TUs */
	uint16_t remaining_aw_length;
	uint8_t min_ext;
	uint8_t max_ext_multicast;
	uint8_t max_ext_unicast;
	uint8_t max_ext_af;
	struct ether_addr master_addr;
	uint8_t presence_mode;
	uint8_t reserved;
	uint16_t next_aw_seq;
	uint16_t ap_alignment;
	/* struct awdl_chanseq chanseq; */
	/* uint8_t pad[2]; */
} __attribute__((__packed__));

struct awdl_chanseq_tlv {
	uint8_t type;
	uint16_t length;
	/* struct awdl_chanseq chanseq; */
	/* uint8_t pad[3]; */
} __attribute__((__packed__));

/* AWDL election parameters */
struct awdl_election_params_tlv {
	uint8_t type;
	uint16_t length;
	uint8_t flags;
	uint16_t id;
	uint8_t distancetop;
	uint8_t unknown;
	struct ether_addr top_master_addr;
	uint32_t top_master_metric;
	uint32_t self_metric;
	uint8_t pad[2];
} __attribute__((__packed__));

struct awdl_election_params_v2_tlv {
	uint8_t type;
	uint16_t length;
	struct ether_addr master_addr; /* currently elected master */
	struct ether_addr sync_addr; /* this is _probably_ the node we currently sync to */
	uint32_t master_counter; /* 'self_counter' of currently elected master */
	uint32_t distance_to_master;
	uint32_t master_metric;
	uint32_t self_metric;
	uint32_t unknown; /* 0 */
	uint32_t reserved; /* 0 */
	uint32_t self_counter; /* is incremented by 1 while node is selected as master every PI (3.14) seconds */
} __attribute__((__packed__));

struct awdl_sync_tree_tlv {
	uint8_t type;
	uint16_t length;
	struct ether_addr tree[1];
} __attribute__((__packed__));

struct awdl_service_params_tlv {
	uint8_t type;
	uint16_t length;
	unsigned char unknown[3];
	uint16_t sui;
	uint32_t bitmask;
} __attribute__((__packed__));

struct awdl_ht_capabilities_tlv {
	uint8_t type;
	uint16_t length;
	uint16_t unknown;
	uint16_t ht_capabilities;
	uint8_t ampdu_params;
	uint8_t rx_mcs;
	uint16_t unknown2;
} __attribute__((__packed__));

enum awdl_data_path_state_flags {
	AWDL_DATA_PATH_FLAG_COUNTRY_CODE = 0x0100,
	AWDL_DATA_PATH_FLAG_SOCIAL_CHANNEL_MAP = 0x0200,
	AWDL_DATA_PATH_FLAG_INFRA_INFO = 0x0001,
	AWDL_DATA_PATH_FLAG_INFRA_ADDRESS = 0x0002,
	AWDL_DATA_PATH_FLAG_AWDL_ADDRESS = 0x0004,
	AWDL_DATA_PATH_FLAG_UMI = 0x0010,
	/* TODO complete */
};

struct awdl_data_path_state_tlv {
	uint8_t type;
	uint16_t length;
	uint16_t flags;
	char country_code[3];
	uint16_t social_channels;
	struct ether_addr awdl_addr;
	uint16_t ext_flags;
	/* uint32_t logtrigger_id; if (ext_flags | 0x4) */
} __attribute__((__packed__));

struct awdl_arpa_tlv {
	uint8_t type;
	uint16_t length;
	uint8_t flags; /* 3 is only valid flag
	                * found in IO80211Family.kext
	                * method: IO80211ServiceRequestDescriptor::initWithArpaTLV(apple80211_awdl_arpa_tlv *,ether_addr *,userPrintCtx *) */
	uint8_t name_length;
	char name[1];
	/* uint16_t suffix */
} __attribute__((__packed__));

struct awdl_version_tlv {
	uint8_t type;
	uint16_t length;
	uint8_t version;
	uint8_t devclass;
} __attribute__((__packed__));

const char *awdl_tlv_as_str(uint8_t type);

#endif /* AWDL_FRAME_H_ */
