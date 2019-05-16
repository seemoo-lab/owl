#ifndef OWL_NETUTILS_H_
#define OWL_NETUTILS_H_

#include <netinet/in.h>
#ifdef __APPLE__
#include <net/ethernet.h>
#else
#include <netinet/ether.h>
#endif

/**
 * Needs to be run once before netutils can be used (platform-dependent).
 *
 * For example, on Linux, sets up nl80211 socket and state.
 *
 * @return 0 on success, a negative value on failure
 */
int netutils_init();

/**
 * Clean up methods if netutils are no longer needed (platform-dependent)
 */
void netutils_cleanup();

int set_monitor_mode(int ifindex);

int set_channel(int ifindex, int channel);

int link_up(int ifindex);

int link_down(int ifindex);

int link_ether_addr_get(const char *ifname, struct ether_addr *addr);

int get_hostname(char *name, size_t len);

int neighbor_add(int ifindex, const struct ether_addr *, const struct in6_addr *);

int neighbor_remove(int ifindex, const struct in6_addr *);

int neighbor_add_rfc4291(int ifindex, const struct ether_addr *);

int neighbor_remove_rfc4291(int ifindex, const struct ether_addr *);

void rfc4291_addr(const struct ether_addr *eth, struct in6_addr *in6);

#endif /* OWL_NETUTILS_H_ */
