#if HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef PACKAGE_NAME
#define PACKAGE_NAME ""
#endif

#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION ""
#endif

#ifndef LOCALEDIR
#define LOCALEDIR "/usr/local/share/locale"
#endif

#include "njit8021xclient.h"

const struct GlobalConfig g_config = {
	/*.package_name    = */ PACKAGE_NAME,
	/*.package_version = */ PACKAGE_VERSION,
	/*.locale_dir      = */ LOCALEDIR,
};

