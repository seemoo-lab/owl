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

#include "frame.h"

const char *awdl_frame_as_str(uint8_t type) {
	switch (type) {
		case AWDL_ACTION_PSF:
			return "PSF";
		case AWDL_ACTION_MIF:
			return "MIF";
		default:
			return "Unknown";
	}
}

const char *awdl_tlv_as_str(uint8_t type) {
	switch (type) {
		case AWDL_SSTH_REQUEST_TLV:
			return "SSTH Request";
		case AWDL_SERVICE_REQUEST_TLV:
			return "Service Request";
		case AWDL_SERVICE_RESPONSE_TLV:
			return "Service Response";
		case AWDL_SYNCHRONIZATON_PARAMETERS_TLV:
			return "Synchronization Parameters";
		case AWDL_ELECTION_PARAMETERS_TLV:
			return "Election Parameters";
		case AWDL_SERVICE_PARAMETERS_TLV:
			return "Service Parameters";
		case AWDL_ENHANCED_DATA_RATE_CAPABILITIES_TLV:
			return "HT Capabilities";
		case AWDL_ENHANCED_DATA_RATE_OPERATION_TLV:
			return "HT Operation";
		case AWDL_INFRA_TLV:
			return "Infra";
		case AWDL_INVITE_TLV:
			return "Invite";
		case AWDL_DBG_STRING_TLV:
			return "Debug String";
		case AWDL_DATA_PATH_STATE_TLV:
			return "Data Path State";
		case AWDL_ENCAPSULATED_IP_TLV:
			return "Encapsulated IP";
		case AWDL_DATAPATH_DEBUG_PACKET_LIVE_TLV:
			return "Datapath Debug Packet Live";
		case AWDL_DATAPATH_DEBUG_AF_LIVE_TLV:
			return "Datapath Debug AF Live";
		case AWDL_ARPA_TLV:
			return "Arpa";
		case AWDL_IEEE80211_CNTNR_TLV:
			return "VHT Capabilities";
		case AWDL_CHAN_SEQ_TLV:
			return "Channel Sequence";
		case AWDL_SYNCTREE_TLV:
			return "Synchronization Tree";
		case AWDL_VERSION_TLV:
			return "Version";
		case AWDL_BLOOM_FILTER_TLV:
			return "Bloom Filter";
		case AWDL_NAN_SYNC_TLV:
			return "NAN Sync";
		case AWDL_ELECTION_PARAMETERS_V2_TLV:
			return "Election Parameters v2";
		default:
			return "Unknown";
	}
}
