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
