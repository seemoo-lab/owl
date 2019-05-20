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

#ifndef AWDL_VERSION_H_
#define AWDL_VERSION_H_

#include <stdint.h>

uint8_t awdl_version(int major, int minor);

int awdl_version_major(uint8_t version);

int awdl_version_minor(uint8_t version);

const char *awdl_version_to_str(uint8_t);

enum awdl_devclass {
	AWDL_DEVCLASS_MACOS = 1,
	AWDL_DEVCLASS_IOS = 2,
	AWDL_DEVCLASS_TVOS = 8,
};

const char *awdl_devclass_to_str(uint8_t devclass);

#endif /* AWDL_VERSION_H_ */
