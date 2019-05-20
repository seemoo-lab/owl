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

#include <stdio.h>

#include "version.h"

uint8_t awdl_version(int major, int minor) {
	return ((uint8_t) (((major) << 4) & 0xf0) | ((minor) & 0x0f));
}

int awdl_version_major(uint8_t version) {
	return (version >> 4) & 0xf;
}

int awdl_version_minor(uint8_t version) {
	return (version & 0xf);
}

const char *awdl_version_to_str(uint8_t version) {
	static char str[64];
	char *cur = str, *const end = str + sizeof(str);
	cur += snprintf(cur, end - cur, "%d", awdl_version_major(version));
	cur += snprintf(cur, end - cur, ".");
	cur += snprintf(cur, end - cur, "%d", awdl_version_minor(version));
	return str;
}

const char *awdl_devclass_to_str(uint8_t devclass) {
	switch (devclass) {
		case AWDL_DEVCLASS_MACOS:
			return "macOS";
		case AWDL_DEVCLASS_IOS:
			return "iOS";
		case AWDL_DEVCLASS_TVOS:
			return "tvOS";
		default:
			return "Unknown";
	}
}
