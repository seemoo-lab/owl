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

extern "C" {
#include "sync.h"
#include "ieee80211.h"
}

#include "gtest/gtest.h"

static struct awdl_sync_state *test_state(uint64_t now) {
  static struct awdl_sync_state state;
  awdl_sync_state_init(&state, now);
  return &state;
}

TEST(awdl_sync, next_aw_tu) {
  uint64_t now = 0;
  struct awdl_sync_state *state = test_state(now);
  uint64_t eaw_period = 64;

  for (uint64_t tu = 0; tu < 4 * eaw_period; tu++) {
    for (; now < ieee80211_tu_to_usec(tu + 1); now++) {
      uint16_t next_aw_tu = awdl_sync_next_aw_tu(now, state);
      EXPECT_EQ(next_aw_tu, eaw_period - (tu % eaw_period));
    }
  }
}

TEST(awdl_sync, current_aw) {
  uint64_t now = 0;
  struct awdl_sync_state *state;
  uint16_t aw, current_aw;

  aw = 0;
  state = test_state(now);
  state->aw_counter = aw;
  current_aw = awdl_sync_current_aw(now, state);
  EXPECT_EQ(current_aw, aw);

  aw = 1337;
  state = test_state(now);
  state->aw_counter = aw;
  current_aw = awdl_sync_current_aw(now, state);
  EXPECT_EQ(current_aw, aw);

  aw = 0xffff;
  state = test_state(now);
  state->aw_counter = aw;
  current_aw = awdl_sync_current_aw(now, state);
  EXPECT_EQ(current_aw, aw);
}

TEST(awdl_sync, current_aw_timedelta) {
  uint64_t now = 0;
  struct awdl_sync_state *state = test_state(now);
  uint64_t tu = 0;

  for (uint16_t aw = 0; aw < 0xffff; aw++) {
    for (; tu < (aw + 1u) * state->aw_period; tu++) {
      uint16_t current_aw = awdl_sync_current_aw(ieee80211_tu_to_usec(tu), state);
      EXPECT_EQ(current_aw, aw);
    }
  }
}
