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

#import <Foundation/Foundation.h>
#import <SystemConfiguration/SystemConfiguration.h>
#import <CoreWLAN/CWInterface.h>
#import <CoreWLAN/CWChannel.h>
#import <CoreWLAN/CWWiFiClient.h>
#import <net/if.h>

static struct wlan_state {
	CWWiFiClient *client;
	CWInterface *iface;
	NSSet<CWChannel *> *supportedChannels;
} state;

int corewlan_init() {
	state.iface = NULL;
	state.supportedChannels = NULL;
	return 0;
}

void corewlan_free() {
}

int corewlan_disassociate(int ifindex) {
	if (!state.client) {
		state.client = [CWWiFiClient sharedWiFiClient];
	}
	if (!state.iface) {
		char iface_cstr[IFNAMSIZ];
		NSString *iface_str =[NSString stringWithUTF8String:if_indextoname(ifindex, iface_cstr)];
		state.iface = [state.client interfaceWithName:iface_str];
	}
	[state.iface disassociate];
	return 0;
}

int corewlan_set_channel(int ifindex, int channel) {
	if (!state.client) {
		state.client = [CWWiFiClient sharedWiFiClient];
	}
	if (!state.iface) {
		char iface_cstr[IFNAMSIZ];
		NSString *iface_str = [NSString stringWithUTF8String:if_indextoname(ifindex, iface_cstr)];
		state.iface = [state.client interfaceWithName:iface_str];
	}
	if (!state.supportedChannels)
		state.supportedChannels = [state.iface supportedWLANChannels];
	CWChannel* currentCandidate = NULL;
	for (CWChannel* chan in state.supportedChannels) {
		if ([chan channelNumber] != channel)
			continue;
		if (!currentCandidate || [chan channelWidth] > [currentCandidate channelWidth])
			currentCandidate = chan;
	}
	if (!currentCandidate)
		return -1;
	NSError* err;
	[state.iface setWLANChannel:currentCandidate error:&err];
	return !err;
}

int corewlan_get_hostname(char *name, size_t len) {
	NSString *computerName = [(NSString *) SCDynamicStoreCopyComputerName(NULL, NULL) autorelease];
	strlcpy(name, [computerName UTF8String], len);
	return 0;
}
