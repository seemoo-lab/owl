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
