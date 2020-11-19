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

#include "netutils.h"

#include <errno.h>
#include <stdbool.h>
#include <ieee80211.h>
#include <log.h>
#include <string.h>

#ifndef __APPLE__

#include <unistd.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/route/link.h>
#include <netlink/route/neighbour.h>
#include <netlink/errno.h>

#include <linux/nl80211.h>

struct nlroute_state {
	struct nl_sock *socket;
};

static struct nlroute_state nlroute_state;

static int nlroute_init(struct nlroute_state *state) {
	state->socket = nl_socket_alloc();
	if (!state->socket) {
		log_error("Failed to allocate netlink socket.");
		return -ENOMEM;
	}

	if (nl_connect(state->socket, NETLINK_ROUTE)) {
		log_error("Failed to connect to generic netlink.");
		nl_socket_free(state->socket);
		return -ENOLINK;
	}

	return 0;
}

static void nlroute_free(struct nlroute_state *state) {
	nl_socket_free(state->socket);
}

struct nl80211_state {
	struct nl_sock *socket;
	int nl80211_id;
};

static struct nl80211_state nl80211_state;

static int nl80211_init(struct nl80211_state *state) {
	state->socket = nl_socket_alloc();
	if (!state->socket) {
		log_error("Failed to allocate netlink socket.");
		return -ENOMEM;
	}

	nl_socket_set_buffer_size(state->socket, 8192, 8192);

	if (genl_connect(state->socket)) {
		log_error("Failed to connect to generic netlink.");
		nl_socket_free(state->socket);
		return -ENOLINK;
	}

	state->nl80211_id = genl_ctrl_resolve(state->socket, "nl80211");
	if (state->nl80211_id < 0) {
		log_error("nl80211 not found.");
		nl_socket_free(state->socket);
		return -ENOENT;
	}

	return 0;
}

static void nl80211_free(struct nl80211_state *state) {
	nl_socket_free(state->socket);
}

int netutils_init() {
	int err;
	err = nlroute_init(&nlroute_state);
	if (err < 0)
		return err;
	err = nl80211_init(&nl80211_state);
	if (err < 0)
		return err;
	return 0;
}

void netutils_cleanup() {
	nlroute_free(&nlroute_state);
	nl80211_free(&nl80211_state);
}

int neighbor_add(int ifindex, const struct ether_addr *eth, const struct in6_addr *in6) {
	int err;
	struct rtnl_neigh *neigh;
	struct nl_addr *nl_ipaddr, *nl_llcaddr;

	neigh = rtnl_neigh_alloc();
	if (!neigh) {
		log_error("Could not allocate netlink message");
		err = -ENOMEM;
		goto out;
	}

	rtnl_neigh_set_ifindex(neigh, ifindex);

	nl_ipaddr = nl_addr_build(AF_INET6, (void *) in6, sizeof(struct in6_addr));
	if (!nl_ipaddr) {
		log_error("Could not allocate netlink message");
		err = -ENOMEM;
		goto out;
	}
	rtnl_neigh_set_dst(neigh, nl_ipaddr);

	nl_llcaddr = nl_addr_build(AF_LLC, (void *) eth, sizeof(struct ether_addr));
	if (!nl_llcaddr) {
		log_error("Could not allocate netlink message");
		err = -ENOMEM;
		goto out;
	}
	rtnl_neigh_set_lladdr(neigh, nl_llcaddr);

	rtnl_neigh_set_state(neigh, NUD_PERMANENT);

	err = rtnl_neigh_add(nlroute_state.socket, neigh, NLM_F_CREATE);
	if (err < 0) {
		log_error("Could not add neighbor: %s", nl_geterror(err));
	}

out:
	if (neigh)
		rtnl_neigh_put(neigh);
	if (nl_ipaddr)
		nl_addr_put(nl_ipaddr);
	if (nl_llcaddr)
		nl_addr_put(nl_llcaddr);

	return err;
}

int neighbor_remove(int ifindex, const struct in6_addr *in6) {
	int err;
	struct rtnl_neigh *neigh;
	struct nl_addr *nl_ipaddr;

	neigh = rtnl_neigh_alloc();
	if (!neigh) {
		log_error("Could not allocate netlink message");
		err = -ENOMEM;
		goto out;
	}

	rtnl_neigh_set_ifindex(neigh, ifindex);

	nl_ipaddr = nl_addr_build(AF_INET6, (void *) in6, sizeof(struct in6_addr));
	if (!nl_ipaddr) {
		log_error("Could not allocate netlink message");
		err = -ENOMEM;
		goto out;
	}
	rtnl_neigh_set_dst(neigh, nl_ipaddr);

	err = rtnl_neigh_delete(nlroute_state.socket, neigh, 0);
	if (err < 0) {
		log_error("Could not delete neighbor: %s", nl_geterror(err));
	}

out:
	if (neigh)
		rtnl_neigh_put(neigh);
	if (nl_ipaddr)
		nl_addr_put(nl_ipaddr);

	return err;
}

int set_monitor_mode(int ifindex) {
	int err = 0;
	struct nl_msg *m = NULL;
	struct nl_msg *flags = NULL;

	m = nlmsg_alloc();
	if (!m) {
		log_error("Could not allocate netlink message");
		err = -ENOMEM;
		goto out;
	}

	genlmsg_put(m, 0, 0, nl80211_state.nl80211_id, 0, 0, NL80211_CMD_SET_INTERFACE, 0);

	NLA_PUT_U32(m, NL80211_ATTR_IFINDEX, ifindex);
	NLA_PUT_U32(m, NL80211_ATTR_IFTYPE, NL80211_IFTYPE_MONITOR);

	flags = nlmsg_alloc();
	if (!flags) {
		log_error("Could not allocate netlink message");
		err = -ENOMEM;
		goto out;
	}
	NLA_PUT_FLAG(flags, NL80211_MNTR_FLAG_ACTIVE);
	nla_put_nested(m, NL80211_ATTR_MNTR_FLAGS, flags);
	nlmsg_free(flags);
	flags = 0;

	err = nl_send_auto(nl80211_state.socket, m);
	if (err < 0) {
		log_error("Error while sending via netlink: %s", nl_geterror(err));
		goto out;
	}

	err = nl_recvmsgs_default(nl80211_state.socket);
	if (err < 0) {
		log_error("Error while receiving via netlink: %s", nl_geterror(err));
		goto out;
	}

	goto out;

nla_put_failure:
	log_error("building message failed");
	err = -ENOBUFS;
out:
	if (m)
		nlmsg_free(m);
	if (flags)
		nlmsg_free(flags);
	return err;
}

#define BIT(x) (1ULL<<(x))

struct channels_ctx {
	int last_band;
	bool width_40;
	bool width_80;
	bool width_160;
};

struct channel_args {
	int freq;
	bool is_available;
	struct channels_ctx ctx;
};

/* Inspired by iw/phy.c */
static int parse_freq_support(struct nl_msg *msg, void *arg)
{
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct channel_args *cargs = (struct channel_args *)arg;
	int test_freq = cargs->freq;
	struct channels_ctx *ctx = &cargs->ctx;
	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
	struct nlattr *tb_band[NL80211_BAND_ATTR_MAX + 1];
	struct nlattr *tb_freq[NL80211_FREQUENCY_ATTR_MAX + 1];
	struct nlattr *nl_band;
	struct nlattr *nl_freq;
	int rem_band, rem_freq;

	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

	if (tb_msg[NL80211_ATTR_WIPHY_BANDS]) {
		nla_for_each_nested(nl_band, tb_msg[NL80211_ATTR_WIPHY_BANDS], rem_band) {
			if (ctx->last_band != nl_band->nla_type) {
				ctx->width_40 = false;
				ctx->width_80 = false;
				ctx->width_160 = false;
				ctx->last_band = nl_band->nla_type;
			}

			nla_parse(tb_band, NL80211_BAND_ATTR_MAX, nla_data(nl_band), nla_len(nl_band), NULL);

			if (tb_band[NL80211_BAND_ATTR_FREQS]) {
				nla_for_each_nested(nl_freq, tb_band[NL80211_BAND_ATTR_FREQS], rem_freq) {
					uint32_t freq;

					nla_parse(tb_freq, NL80211_FREQUENCY_ATTR_MAX, nla_data(nl_freq), nla_len(nl_freq), NULL);

					if (!tb_freq[NL80211_FREQUENCY_ATTR_FREQ])
						continue;
					freq = nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_FREQ]);

					if ((int) freq != test_freq)
						continue;

					if (tb_freq[NL80211_FREQUENCY_ATTR_DISABLED] || tb_freq[NL80211_FREQUENCY_ATTR_NO_IR]) {
						if (tb_freq[NL80211_FREQUENCY_ATTR_DISABLED])
							log_debug("Channel %d [%d MHz] is disabled", ieee80211_frequency_to_channel(freq), freq);
						if (tb_freq[NL80211_FREQUENCY_ATTR_NO_IR])
							log_debug("Channel %d [%d MHz] does not allow to initiate radiation first (no IR)", ieee80211_frequency_to_channel(freq), freq);
						log_warn("Cannot inject frames on channel %d [%d MHz], try setting a regulatory domain (`iw reg set <CC>`) or using a different channel (6, 49, or 149)", ieee80211_frequency_to_channel(freq), freq);
						continue;
					}

					log_debug("Channel %d [%d MHz] is available for frame injection", ieee80211_frequency_to_channel(freq), freq);
					cargs->is_available = true;
				}
			}
		}
	}

	return NL_SKIP;
}

int is_channel_available(int ifindex, const int channel, bool *is_available) {
	int err;
	struct nl_msg *m;
	struct nl_cb *cb;
	int freq;
	static struct channel_args args = {
		.is_available = false,
		.ctx = {
			.last_band = -1,
		},
	};
	
	freq = ieee80211_channel_to_frequency(channel);
	if (!freq) {
		log_error("Invalid channel number %d", channel);
		err = -EINVAL;
		goto out;
	}
	args.freq = freq;

	m = nlmsg_alloc();
	if (!m) {
		log_error("Could not allocate netlink message");
		err = -ENOMEM;
		goto out;
	}

	cb = nl_cb_alloc(NL_CB_DEFAULT);

	if (genlmsg_put(m, 0, 0, nl80211_state.nl80211_id, 0, 0, NL80211_CMD_GET_WIPHY, 0) == NULL) {
		err = -ENOBUFS;
		goto out;
	}

	NLA_PUT_U32(m, NL80211_ATTR_IFINDEX, ifindex);

	nla_put_flag(m, NL80211_ATTR_SPLIT_WIPHY_DUMP);
	nlmsg_hdr(m)->nlmsg_flags |= NLM_F_DUMP;

	err = nl_send_auto(nl80211_state.socket, m);
	if (err < 0) {
		log_error("error while sending via netlink");
		goto out;
	}

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, parse_freq_support, &args);

	err = nl_recvmsgs(nl80211_state.socket, cb);
	goto out;

nla_put_failure:
	log_error("building message failed");
	err = -ENOBUFS;
out:
	if (cb)
		nl_cb_put(cb);
	if (m)
		nlmsg_free(m);
	*is_available = args.is_available;
	return err;
}

int set_channel(int ifindex, int channel) {
	int err;
	struct nl_msg *m;
	int freq;

	freq = ieee80211_channel_to_frequency(channel);
	if (!freq) {
		log_error("Invalid channel number %d", channel);
		err = -EINVAL;
		goto out;
	}

	m = nlmsg_alloc();
	if (!m) {
		log_error("Could not allocate netlink message");
		err = -ENOMEM;
		goto out;
	}

	if (genlmsg_put(m, 0, 0, nl80211_state.nl80211_id, 0, 0, NL80211_CMD_SET_CHANNEL, 0) == NULL) {
		err = -ENOBUFS;
		goto out;
	}

	NLA_PUT_U32(m, NL80211_ATTR_IFINDEX, ifindex);
	NLA_PUT_U32(m, NL80211_ATTR_WIPHY_FREQ, freq);
	NLA_PUT_U32(m, NL80211_ATTR_WIPHY_CHANNEL_TYPE, NL80211_CHAN_HT40PLUS);

	err = nl_send_auto(nl80211_state.socket, m);
	if (err < 0) {
		log_error("error while sending via netlink");
		goto out;
	}

	err = nl_recvmsgs_default(nl80211_state.socket);
	goto out;

nla_put_failure:
	log_error("building message failed");
	err = -ENOBUFS;
out:
	if (m)
		nlmsg_free(m);
	return err;
}

static int link_updown(int ifindex, int up) {
	int err;
	struct rtnl_link *old, *req;

	err = rtnl_link_get_kernel(nlroute_state.socket, ifindex, NULL, &old);
	if (err < 0) {
		log_error("Could not get link: %s", nl_geterror(err));
		return err;
	}

	req = rtnl_link_alloc();

	if (up)
		rtnl_link_set_flags(req, rtnl_link_str2flags("up"));
	else
		rtnl_link_set_flags(req, rtnl_link_str2flags("down"));

	rtnl_link_change(nlroute_state.socket, old, req, 0);

	rtnl_link_put(old);

	return 0;
}

int link_up(int ifindex) {
	return link_updown(ifindex, 1);
}

int link_down(int ifindex) {
	return link_updown(ifindex, 0);
}

int link_ether_addr_get(const char *ifname, struct ether_addr *addr) {
	int err;
	struct rtnl_link *link;
	struct nl_addr *nladdr;

	err = rtnl_link_get_kernel(nlroute_state.socket, 0, ifname, &link);
	if (err < 0) {
		log_error("Could not get link: %s", nl_geterror(err));
		return err;
	}

	nladdr = rtnl_link_get_addr(link);

	*addr = *(struct ether_addr *) nl_addr_get_binary_addr(nladdr);

	return 0;
}

int get_hostname(char *name, size_t len) {
	if (gethostname(name, len) < 0)
		return -errno;
	return 0;
}

#else /* __APPLE__ */

#include <unistd.h>
#include <fcntl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if_dl.h>

#include "corewlan.h"

int netutils_init() {
	return corewlan_init();
}

void netutils_cleanup() {
	corewlan_free();
}

int set_monitor_mode(int ifindex) {
	corewlan_disassociate(ifindex);
	/* TODO implement here instead of using libpcap */
	return 0;
}

int is_channel_available(int ifindex, int channel, bool *is_available) {
	/* we assume that channel is always available on macOS */
	(void) ifindex;
	(void) channel;
	*is_available = true;
	return 0;
}

int set_channel(int ifindex, int channel) {
	return corewlan_set_channel(ifindex, channel);
}

int link_up(int ifindex) {
	(void) ifindex;
	return 0; /* TODO implement */
}

int link_down(int ifindex) {
	(void) ifindex;
	return 0; /* TODO implement */
}

int link_ether_addr_get(const char *ifname, struct ether_addr *addr) {
	struct ifaddrs *ifap, *ifa;
	if (getifaddrs(&ifap) < 0)
		return -1;
	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr && (ifa->ifa_addr->sa_family == AF_LINK) && !strcmp(ifname, ifa->ifa_name)) {
			struct sockaddr_dl *sdl = (struct sockaddr_dl *) ifa->ifa_addr;
			memcpy(addr, LLADDR(sdl), sdl->sdl_alen);
			return 0;
		}
	}
	return -1;
}

int get_hostname(char *name, size_t len) {
	return corewlan_get_hostname(name, len);
}

/* TODO would be nicer to use PF_ROUTE socket instead of calling ndp */
int neighbor_add(int ifindex, const struct ether_addr *eth, const struct in6_addr *in6) {
	if (fork() <= 0) {
		char eth_str[18];
		char in6_str[INET6_ADDRSTRLEN];
		char iface[IFNAMSIZ];
		char in6scope_str[INET6_ADDRSTRLEN + 1 + IFNAMSIZ];
		strlcpy(eth_str, ether_ntoa(eth), sizeof(eth_str));;
		sprintf(in6scope_str, "%s%%%s", inet_ntop(AF_INET6, (void *) in6, in6_str, sizeof(in6_str)),
		        if_indextoname(ifindex, iface));
		int null = open("/dev/null", O_WRONLY);
		dup2(null, 1 /* stdout */);
		dup2(null, 2 /* stderr */);
		execlp("/usr/sbin/ndp", "/usr/sbin/ndp", "-sn", in6scope_str, eth_str, (char *) NULL);
		close(null);
	}
	return 0;
}

int neighbor_remove(int ifindex, const struct in6_addr *in6) {
	if (fork() <= 0) {
		char in6_str[INET6_ADDRSTRLEN];
		char iface[IFNAMSIZ];
		char in6scope_str[INET6_ADDRSTRLEN + 1 + IFNAMSIZ];
		sprintf(in6scope_str, "%s%%%s", inet_ntop(AF_INET6, (void *) in6, in6_str, sizeof(in6_str)),
		        if_indextoname(ifindex, iface));
		int null = open("/dev/null", O_WRONLY);
		dup2(null, 1 /* stdout */);
		dup2(null, 2 /* stderr */);
		execlp("/usr/sbin/ndp", "/usr/sbin/ndp", "-dn", in6scope_str, (char *) NULL);
		close(null);
	}
	return 0;
}

#endif /* __APPLE__ */

int neighbor_add_rfc4291(int ifindex, const struct ether_addr *addr) {
	struct in6_addr in6;
	rfc4291_addr(addr, &in6);
	return neighbor_add(ifindex, addr, &in6);
}

int neighbor_remove_rfc4291(int ifindex, const struct ether_addr *addr) {
	struct in6_addr in6;
	rfc4291_addr(addr, &in6);
	return neighbor_remove(ifindex, &in6);
}

void rfc4291_addr(const struct ether_addr *eth, struct in6_addr *in6) {
	memset(in6, 0, sizeof(struct in6_addr));
	in6->s6_addr[0] = 0xfe;
	in6->s6_addr[1] = 0x80;
	in6->s6_addr[8] = eth->ether_addr_octet[0] ^ 0x02;
	in6->s6_addr[9] = eth->ether_addr_octet[1];
	in6->s6_addr[10] = eth->ether_addr_octet[2];
	in6->s6_addr[11] = 0xff;
	in6->s6_addr[12] = 0xfe;
	in6->s6_addr[13] = eth->ether_addr_octet[3];
	in6->s6_addr[14] = eth->ether_addr_octet[4];
	in6->s6_addr[15] = eth->ether_addr_octet[5];
}
