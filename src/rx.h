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

#ifndef AWDL_RX_H_
#define AWDL_RX_H_

#include "frame.h"
#include "version.h"
#include "state.h"

enum RX_RESULT {
	RX_IGNORE_PEER = 6,
	RX_IGNORE_RSSI = 5,
	RX_IGNORE_FAILED_CRC = 4,
	RX_IGNORE_NOPROMISC = 3,
	RX_IGNORE_FROM_SELF = 2,
	RX_IGNORE = 1,
	RX_OK = 0,
	RX_UNEXPECTED_FORMAT = -1,
	RX_UNEXPECTED_TYPE = -2,
	RX_UNEXPECTED_VALUE = -3,
	RX_TOO_SHORT = -3,
};

int awdl_handle_sync_params_tlv(struct awdl_peer *src, const struct buf *val, struct awdl_state *state, uint64_t tsft);

int awdl_handle_chanseq_tlv(struct awdl_peer *src, const struct buf *val, struct awdl_state *state);

int awdl_handle_election_params_tlv(struct awdl_peer *src, const struct buf *val, struct awdl_state *state);

int awdl_handle_election_params_v2_tlv(struct awdl_peer *src, const struct buf *val,
                                       struct awdl_state *state);

int awdl_handle_arpa_tlv(struct awdl_peer *src, const struct buf *val, struct awdl_state *state);

int awdl_handle_data_path_state_tlv(struct awdl_peer *src, const struct buf *val, struct awdl_state *state);

int awdl_handle_version_tlv(struct awdl_peer *src, const struct buf *val, struct awdl_state *state);

int awdl_handle_tlv(struct awdl_peer *src, uint8_t type, const struct buf *val,
                    struct awdl_state *state, uint64_t tsft);

int awdl_parse_action_hdr(const struct buf *frame);

int awdl_rx_action(const struct buf *frame, signed char rssi, uint64_t tsft,
                   const struct ether_addr *src, const struct ether_addr *dst, struct awdl_state *state);

int llc_parse(const struct buf *frame, struct llc_hdr *llc);
int awdl_valid_llc_header(const struct buf *frame);

int awdl_rx_data(const struct buf *frame, struct buf ***out,
                 const struct ether_addr *src, const struct ether_addr *dst, struct awdl_state *state);

int awdl_rx_data_amsdu(const struct buf *frame, struct buf ***out,
                       const struct ether_addr *src, const struct ether_addr *dst, struct awdl_state *state);

/** @brief Receive and process AWDL action and data frame
 *
 * @param frame input frame
 * @param data if {@code frame} points to a valid AWDL data frame, will point to converted Ethernet frame after function returns
 * @param state
 * @return RX_OK
 */
int awdl_rx(const struct buf *frame, struct buf ***data_frames, struct awdl_state *state);

#endif /* AWDL_RX_H_ */
