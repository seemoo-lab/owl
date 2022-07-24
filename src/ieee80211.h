#ifndef IEEE80211_H_
#define IEEE80211_H_

#include <stdint.h>

#ifdef __APPLE__

#include <net/ethernet.h>
#elif __BIONIC__
#include <net/ethernet.h>
#include <netinet/ether.h>

#else
#include <netinet/ether.h>
#endif

/* Some relevant Ethernet Protocol IDs */
#define ETH_P_IP	0x0800		/* Internet Protocol packet	*/
#define ETH_P_IPV6	0x86DD		/* IPv6 over bluebook		*/

#define OUI_LEN 3

struct oui {
	uint8_t byte[OUI_LEN];
} __attribute__((__packed__));

#define FCS_LEN 4

#define IEEE80211_FCTL_VERS		0x0003
#define IEEE80211_FCTL_FTYPE		0x000c
#define IEEE80211_FCTL_STYPE		0x00f0
#define IEEE80211_FCTL_TODS		0x0100
#define IEEE80211_FCTL_FROMDS		0x0200
#define IEEE80211_FCTL_MOREFRAGS	0x0400
#define IEEE80211_FCTL_RETRY		0x0800
#define IEEE80211_FCTL_PM		0x1000
#define IEEE80211_FCTL_MOREDATA		0x2000
#define IEEE80211_FCTL_PROTECTED	0x4000
#define IEEE80211_FCTL_ORDER		0x8000
#define IEEE80211_FCTL_CTL_EXT		0x0f00

#define IEEE80211_SCTL_FRAG		0x000F
#define IEEE80211_SCTL_SEQ		0xFFF0

#define IEEE80211_FTYPE_MGMT		0x0000
#define IEEE80211_FTYPE_CTL		0x0004
#define IEEE80211_FTYPE_DATA		0x0008
#define IEEE80211_FTYPE_EXT		0x000c

/* management */
#define IEEE80211_STYPE_ASSOC_REQ	0x0000
#define IEEE80211_STYPE_ASSOC_RESP	0x0010
#define IEEE80211_STYPE_REASSOC_REQ	0x0020
#define IEEE80211_STYPE_REASSOC_RESP	0x0030
#define IEEE80211_STYPE_PROBE_REQ	0x0040
#define IEEE80211_STYPE_PROBE_RESP	0x0050
#define IEEE80211_STYPE_BEACON		0x0080
#define IEEE80211_STYPE_ATIM		0x0090
#define IEEE80211_STYPE_DISASSOC	0x00A0
#define IEEE80211_STYPE_AUTH		0x00B0
#define IEEE80211_STYPE_DEAUTH		0x00C0
#define IEEE80211_STYPE_ACTION		0x00D0

/* control */
#define IEEE80211_STYPE_CTL_EXT		0x0060
#define IEEE80211_STYPE_BACK_REQ	0x0080
#define IEEE80211_STYPE_BACK		0x0090
#define IEEE80211_STYPE_PSPOLL		0x00A0
#define IEEE80211_STYPE_RTS		0x00B0
#define IEEE80211_STYPE_CTS		0x00C0
#define IEEE80211_STYPE_ACK		0x00D0
#define IEEE80211_STYPE_CFEND		0x00E0
#define IEEE80211_STYPE_CFENDACK	0x00F0

/* data */
#define IEEE80211_STYPE_DATA			0x0000
#define IEEE80211_STYPE_DATA_CFACK		0x0010
#define IEEE80211_STYPE_DATA_CFPOLL		0x0020
#define IEEE80211_STYPE_DATA_CFACKPOLL		0x0030
#define IEEE80211_STYPE_NULLFUNC		0x0040
#define IEEE80211_STYPE_CFACK			0x0050
#define IEEE80211_STYPE_CFPOLL			0x0060
#define IEEE80211_STYPE_CFACKPOLL		0x0070
#define IEEE80211_STYPE_QOS_DATA		0x0080
#define IEEE80211_STYPE_QOS_DATA_CFACK		0x0090
#define IEEE80211_STYPE_QOS_DATA_CFPOLL		0x00A0
#define IEEE80211_STYPE_QOS_DATA_CFACKPOLL	0x00B0
#define IEEE80211_STYPE_QOS_NULLFUNC		0x00C0
#define IEEE80211_STYPE_QOS_CFACK		0x00D0
#define IEEE80211_STYPE_QOS_CFPOLL		0x00E0
#define IEEE80211_STYPE_QOS_CFACKPOLL		0x00F0

/* extension, added by 802.11ad */
#define IEEE80211_STYPE_DMG_BEACON		0x0000

/* control extension - for IEEE80211_FTYPE_CTL | IEEE80211_STYPE_CTL_EXT */
#define IEEE80211_CTL_EXT_POLL		0x2000
#define IEEE80211_CTL_EXT_SPR		0x3000
#define IEEE80211_CTL_EXT_GRANT	0x4000
#define IEEE80211_CTL_EXT_DMG_CTS	0x5000
#define IEEE80211_CTL_EXT_DMG_DTS	0x6000
#define IEEE80211_CTL_EXT_SSW		0x8000
#define IEEE80211_CTL_EXT_SSW_FBACK	0x9000
#define IEEE80211_CTL_EXT_SSW_ACK	0xa000

/* Maximum size for the MA-UNITDATA primitive, 802.11 standard section
   6.2.1.1.2.
   802.11e clarifies the figure in section 7.1.2. The frame body is
   up to 2304 octets long (maximum MSDU size) plus any crypt overhead. */
#define IEEE80211_MAX_DATA_LEN		2304
/* 30 byte 4 addr hdr, 2 byte QoS, 2304 byte MSDU, 12 byte crypt, 4 byte FCS */
#define IEEE80211_MAX_FRAME_LEN		2352

#define IEEE80211_QOS_CTL_LEN		2
/* 1d tag mask */
#define IEEE80211_QOS_CTL_TAG1D_MASK		0x0007
/* TID mask */
#define IEEE80211_QOS_CTL_TID_MASK		0x000f
/* EOSP */
#define IEEE80211_QOS_CTL_EOSP			0x0010
/* ACK policy */
#define IEEE80211_QOS_CTL_ACK_POLICY_NORMAL	0x0000
#define IEEE80211_QOS_CTL_ACK_POLICY_NOACK	0x0020
#define IEEE80211_QOS_CTL_ACK_POLICY_NO_EXPL	0x0040
#define IEEE80211_QOS_CTL_ACK_POLICY_BLOCKACK	0x0060
#define IEEE80211_QOS_CTL_ACK_POLICY_MASK	0x0060
/* A-MSDU 802.11n */
#define IEEE80211_QOS_CTL_A_MSDU_PRESENT	0x0080
/* Mesh Control 802.11s */
#define IEEE80211_QOS_CTL_MESH_CONTROL_PRESENT  0x0100

/* Mesh Power Save Level */
#define IEEE80211_QOS_CTL_MESH_PS_LEVEL		0x0200
/* Mesh Receiver Service Period Initiated */
#define IEEE80211_QOS_CTL_RSPI			0x0400

/*
 * 802.11n Management Action Frames
 *
 * Adapted from 'ieee80211_hdr_3addr' in <linux/ieee80211.h>
 */
struct ieee80211_hdr {
	uint16_t frame_control;
	uint16_t duration_id;
	struct ether_addr addr1; /* dst */
	struct ether_addr addr2; /* src */
	struct ether_addr addr3; /* bssid */
	uint16_t seq_ctrl;
} __attribute__((__packed__));

struct llc_hdr {
	uint8_t dsap;
	uint8_t ssap;
	uint8_t control;
	/* SNAP extension */
	struct oui oui;
	uint16_t pid;
} __attribute__((__packed__));

/**
 * ieee80211_tu_to_usec - convert time units (TU) to microseconds
 * @tu: the TUs
 */
static inline unsigned long ieee80211_tu_to_usec(unsigned long tu) {
	return 1024 * tu;
}

static inline unsigned long ieee80211_usec_to_tu(unsigned long usec) {
	return usec / 1024;
}

int ieee80211_channel_to_frequency(int chan);
int ieee80211_frequency_to_channel(int freq);

static inline int ieee80211_radiotap_type_to_mask(int type) {
    return 1 << type;
}

static inline int ieee80211_radiotap_rate_to_val(int rate) {
    return 2 * rate;
}

#endif /* IEEE80211_H_ */
