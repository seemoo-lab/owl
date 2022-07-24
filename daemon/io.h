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

#ifndef OWL_IO_H
#define OWL_IO_H

#include <stdint.h>
#include <pcap/pcap.h>
#include <net/if.h>
#include <limits.h>

#ifdef __APPLE__
#include <net/ethernet.h>
#elif __BIONIC__
#include <net/ethernet.h>
#include <netinet/ether.h>
#else
#include <netinet/ether.h>
#endif

struct io_state {
	pcap_t *wlan_handle;
	char wlan_ifname[PATH_MAX]; /* name of WLAN iface */
	int wlan_ifindex;     /* index of WLAN iface */
	char host_ifname[IFNAMSIZ]; /* name of host iface */
	int host_ifindex;     /* index of host iface */
	struct ether_addr if_ether_addr; /* MAC address of WLAN and host iface */
	int wlan_fd;
	int host_fd;
	char *dumpfile;
	char wlan_no_monitor_mode;
	int wlan_is_file;
};

int io_state_init(struct io_state *state, const char *wlan, const char *host, const struct ether_addr *bssid_filter);

void io_state_free(struct io_state *state);

int wlan_send(const struct io_state *state, const uint8_t *buf, int len);

int host_send(const struct io_state *state, const uint8_t *buf, int len);

int host_recv(const struct io_state *state, uint8_t *buf, int *len);

#endif /* OWL_IO_H */
