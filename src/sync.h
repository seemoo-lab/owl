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

#ifndef AWDL_SYNC_H_
#define AWDL_SYNC_H_

#include <stdint.h>

struct awdl_sync_state {
	uint16_t aw_counter;
	uint64_t last_update; /* in us */
	uint16_t aw_period; /* in TU */
	uint8_t presence_mode;

	/* statistics */
	uint64_t meas_err;
	uint64_t meas_total;
};

void awdl_sync_state_init(struct awdl_sync_state *state, uint64_t now);

uint16_t awdl_sync_next_aw_tu(uint64_t now_usec, const struct awdl_sync_state *state);

uint64_t awdl_sync_next_aw_us(uint64_t now_usec, const struct awdl_sync_state *state);

uint16_t awdl_sync_current_aw(uint64_t now_usec, const struct awdl_sync_state *state);

uint16_t awdl_sync_current_eaw(uint64_t now_usec, const struct awdl_sync_state *state);

int64_t awdl_sync_error_tu(uint64_t now_usec, uint16_t time_to_next_aw, uint16_t aw_counter,
                           const struct awdl_sync_state *state);

void awdl_sync_update_last(uint64_t now_usec, uint16_t time_to_next_aw, uint16_t aw_counter,
                           struct awdl_sync_state *state);

#endif /* AWDL_SYNC_H_ */
