#ifndef OWL_CORE_H
#define OWL_CORE_H

#include <stdbool.h>
#include <ev.h>

#include "state.h"
#include "circular_buffer.h"
#include "io.h"

struct ev_state {
	struct ev_loop *loop;
	ev_timer mif_timer, psf_timer, tx_timer, tx_mcast_timer, chan_timer, peer_timer;
	ev_io read_wlan, read_host;
	ev_signal stats;
};

struct daemon_state {
	struct io_state io;
	struct awdl_state awdl_state;
	struct ieee80211_state ieee80211_state;
	struct ev_state ev_state;
	struct buf *next;
	cbuf_handle_t tx_queue_multicast;
	const char *dump;
};

int awdl_init(struct daemon_state *state, const char *wlan, const char *host, struct awdl_chan chan, const char *dump);

void awdl_free(struct daemon_state *state);

void awdl_schedule(struct ev_loop *loop, struct daemon_state *state);

void wlan_device_ready(struct ev_loop *loop, ev_io *handle, int revents);

void host_device_ready(struct ev_loop *loop, ev_io *handle, int revents);

void awdl_receive_frame(uint8_t *user, const struct pcap_pkthdr *hdr, const uint8_t *buf);

void awdl_send_action(struct daemon_state *state, enum awdl_action_type type);

void awdl_send_psf(struct ev_loop *loop, ev_timer *handle, int revents);

void awdl_send_mif(struct ev_loop *loop, ev_timer *handle, int revents);

void awdl_send_unicast(struct ev_loop *loop, ev_timer *timer, int revents);

void awdl_send_multicast(struct ev_loop *loop, ev_timer *timer, int revents);

int awdl_send_data(const struct buf *buf, const struct io_state *io_state,
                   struct awdl_state *awdl_state, struct ieee80211_state *ieee80211_state);

void awdl_switch_channel(struct ev_loop *loop, ev_timer *handle, int revents);

void awdl_clean_peers(struct ev_loop *loop, ev_timer *timer, int revents);

void awdl_print_stats(struct ev_loop *loop, ev_signal *handle, int revents);

#endif /* OWL_CORE_H */
