#ifndef AWDL_SCHEDULE_H_
#define AWDL_SCHEDULE_H_

#include <stdbool.h>

#include "state.h"
#include "peers.h"

#define AWDL_UNICAST_GUARD_TU 3
#define AWDL_MULTICAST_GUARD_TU 16

double usec_to_sec(uint64_t usec);

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
