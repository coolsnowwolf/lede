#include "charmaps/iso-8859-2.h"
#include "charmaps/iso-8859-10.h"
#include "charmaps/windows-874.h"
#include "charmaps/windows-1250.h"
#include "charmaps/koi8-r.h"

#ifdef ALL_CHARSETS
#include "charmaps/iso-8859-3.h"
#include "charmaps/iso-8859-4.h"
#include "charmaps/iso-8859-5.h"
#include "charmaps/iso-8859-6.h"
#include "charmaps/iso-8859-7.h"
#include "charmaps/iso-8859-8.h"
#include "charmaps/iso-8859-9.h"
#include "charmaps/iso-8859-13.h"
#include "charmaps/iso-8859-14.h"
#include "charmaps/iso-8859-16.h"
#include "charmaps/windows-1251.h"
#include "charmaps/windows-1252.h"
#include "charmaps/windows-1253.h"
#include "charmaps/windows-1254.h"
#include "charmaps/windows-1255.h"
#include "charmaps/windows-1256.h"
#include "charmaps/windows-1257.h"
#include "charmaps/windows-1258.h"
#endif


struct charmap {
	const char name[13];
	const unsigned char *map;
};

static struct charmap charmaps[] = {
	{ "ISO-8859-2",   map_iso_8859_2   },
	{ "ISO-8859-10",  map_iso_8859_10  },

#ifdef ALL_CHARSETS
	{ "ISO-8859-3",   map_iso_8859_3   },
	{ "ISO-8859-4",   map_iso_8859_4   },
	{ "ISO-8859-5",   map_iso_8859_5   },
	{ "ISO-8859-6",   map_iso_8859_6   },
	{ "ISO-8859-7",   map_iso_8859_7   },
	{ "ISO-8859-8",   map_iso_8859_8   },
	{ "ISO-8859-9",   map_iso_8859_9   },
	{ "ISO-8859-13",  map_iso_8859_13  },
	{ "ISO-8859-14",  map_iso_8859_14  },
	{ "ISO-8859-16",  map_iso_8859_16  },
#endif

	{ "WINDOWS-874",  map_windows_874  },
	{ "WINDOWS-1250", map_windows_1250 },

#ifdef ALL_CHARSETS
	{ "WINDOWS-1251", map_windows_1251 },
	{ "WINDOWS-1252", map_windows_1252 },
	{ "WINDOWS-1253", map_windows_1253 },
	{ "WINDOWS-1254", map_windows_1254 },
	{ "WINDOWS-1255", map_windows_1255 },
	{ "WINDOWS-1256", map_windows_1256 },
	{ "WINDOWS-1257", map_windows_1257 },
	{ "WINDOWS-1258", map_windows_1258 },
#endif

	{ "KOI8-R",       map_koi8_r       },

	/* Aliases */
	{ "LATIN2",       map_iso_8859_2   },
	{ "LATIN6",       map_iso_8859_10  },

#ifdef ALL_CHARSETS
	{ "ARABIC",       map_iso_8859_6   },
	{ "CYRILLIC",     map_iso_8859_5   },
	{ "GREEK",        map_iso_8859_7   },
	{ "HEBREW",       map_iso_8859_8   },
	{ "LATIN3",       map_iso_8859_3   },
	{ "LATIN4",       map_iso_8859_4   },
	{ "LATIN5",       map_iso_8859_9   },
#endif
};
