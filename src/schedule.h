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

#ifndef AWDL_SCHEDULE_H_
#define AWDL_SCHEDULE_H_

#include <stdbool.h>

#include "state.h"
#include "peers.h"

#define AWDL_UNICAST_GUARD_TU 3
#define AWDL_MULTICAST_GUARD_TU 16

double usec_to_sec(uint64_t usec);

uint64_t sec_to_usec(double sec);

/**
 * @brief Determine whether we are on the same non-zero channel as {@code peer}.
 * @param state our state
 * @param peer the other peer
 * @return true if we and {@code peer} are on the same non-zero channel, false otherwise
 */
bool awdl_same_channel_as_peer(const struct awdl_state *state, uint64_t now, const struct awdl_peer *peer);

/**
 * @brief Determine whether we are
 * @param state
 * @return
 */
int awdl_is_multicast_eaw(const struct awdl_state *state, uint64_t now);

/**
 * @brief Determine whether we are outside a certain guard interval.
 *
 *          time
 * ----------------------->
 * +---+--------------+---+
 * | G |  ok to send  | G |
 * +---+--------------+---+
 *   ^     ^            ^
 *   |     |            |
 *  pos    0           neg
 *
 *  return when (^) is now
 *
 * @param state AWDL state
 * @param guard guard interval in TU
 * @return 0 if we are outside guard interval, or a positive or negative time in seconds
 */
double awdl_can_send_in(const struct awdl_state *state, uint64_t now, int guard);

double awdl_can_send_unicast_in(const struct awdl_state *state, const struct awdl_peer *peer, uint64_t now, int guard);

#endif /* AWDL_SCHEDULE_H_ */
