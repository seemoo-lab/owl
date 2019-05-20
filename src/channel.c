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

#include "channel.h"
#include "ieee80211.h"

void awdl_chanseq_init(struct awdl_chan *seq) {
	for (int i = 0; i < AWDL_CHANSEQ_LENGTH; i++, seq++) {
		if (i < 8)
			*seq = CHAN_OPCLASS_149;
		else
			*seq = CHAN_OPCLASS_6;
	}
}

void awdl_chanseq_init_idle(struct awdl_chan *seq) {
	for (int i = 0; i < AWDL_CHANSEQ_LENGTH; i++, seq++) {
		switch (i) {
			case 8:
				*seq = CHAN_OPCLASS_6;
				break;
			case 0:
			case 9:
			case 10:
				*seq = CHAN_OPCLASS_149;
				break;
			default:
				*seq = CHAN_NULL;
				break;
		}
	}
}

void awdl_chanseq_init_static(struct awdl_chan *seq, const struct awdl_chan *chan) {
	for (int i = 0; i < AWDL_CHANSEQ_LENGTH; i++, seq++) {
		*seq = *chan;
	}
}

uint8_t awdl_chan_num(struct awdl_chan chan, enum awdl_chan_encoding enc) {
	switch (enc) {
		case AWDL_CHAN_ENC_SIMPLE:
			return chan.simple.chan_num;
		case AWDL_CHAN_ENC_LEGACY:
			return chan.legacy.chan_num;
		case AWDL_CHAN_ENC_OPCLASS:
			return chan.opclass.chan_num;
		default:
			return 0; /* unknown encoding */
	}
}

int awdl_chan_encoding_size(enum awdl_chan_encoding enc) {
	switch (enc) {
		case AWDL_CHAN_ENC_SIMPLE:
			return 1;
		case AWDL_CHAN_ENC_LEGACY:
		case AWDL_CHAN_ENC_OPCLASS:
			return 2;
		default:
			return -1; /* unknown encoding */
	}
}

/* adapted from iw/util.c */
int ieee80211_channel_to_frequency(int chan) {
	/* see 802.11 17.3.8.3.2 and Annex J
	 * there are overlapping channel numbers in 5GHz and 2GHz bands */
	if (chan <= 0)
		return 0; /* not supported */

	/* 2 GHz band */
	if (chan == 14)
		return 2484;
	else if (chan < 14)
		return 2407 + chan * 5;

	/* 5 GHz band */
	if (chan < 32)
		return 0; /* not supported */

	if (chan >= 182 && chan <= 196)
		return 4000 + chan * 5;
	else
		return 5000 + chan * 5;

}

/* from iw/util.c */
int ieee80211_frequency_to_channel(int freq) {
	/* see 802.11-2007 17.3.8.3.2 and Annex J */
	if (freq == 2484)
		return 14;
	else if (freq < 2484)
		return (freq - 2407) / 5;
	else if (freq >= 4910 && freq <= 4980)
		return (freq - 4000) / 5;
	else if (freq <= 45000) /* DMG band lower limit */
		return (freq - 5000) / 5;
	else if (freq >= 58320 && freq <= 64800)
		return (freq - 56160) / 2160;
	else
		return 0;
}
