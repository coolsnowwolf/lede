/*
 * wlc - Broadcom Wireless Driver Control Utility
 *
 * Copyright (C) 2006 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <glob.h>
#include <ctype.h>

#include <typedefs.h>
#include <wlutils.h>
#include <proto/802.11.h>

#define VERSION "0.1"
#define BUFSIZE 8192
#define PTABLE_MAGIC 0xbadc0ded
#define PTABLE_SLT1 1
#define PTABLE_SLT2 2
#define PTABLE_ACKW 3
#define PTABLE_ADHM 4
#define PTABLE_END 0xffffffff

/* 
 * Copy each token in wordlist delimited by space into word 
 * Taken from Broadcom shutils.h
 */
#define foreach(word, wordlist, next) \
	for (next = &wordlist[strspn(wordlist, " ")], \
		 strncpy(word, next, sizeof(word)), \
		 word[strcspn(word, " ")] = '\0', \
		 word[sizeof(word) - 1] = '\0', \
		 next = strchr(next, ' '); \
		 strlen(word); \
		 next = next ? &next[strspn(next, " ")] : "", \
		 strncpy(word, next, sizeof(word)), \
		 word[strcspn(word, " ")] = '\0', \
		 word[sizeof(word) - 1] = '\0', \
		 next = strchr(next, ' '))

static char wlbuf[8192];
static char interface[16] = "wl0";
static unsigned long kmem_offset = 0;
static int vif = 0, debug = 1, fromstdin = 0;

typedef enum {
	NONE =   0x00,

	/* types */
	PARAM_TYPE =    0x00f,
	INT =    0x001,
	STRING = 0x002,
	MAC =    0x003,

	/* options */
	PARAM_OPTIONS = 0x0f0,
	NOARG =  0x010,

	/* modes */
	PARAM_MODE =    0xf00,
	GET =    0x100,
	SET =    0x200,
} wlc_param;

struct wlc_call {
	const char *name;
	wlc_param param;
	int (*handler)(wlc_param param, void *data, void *value);
	union {
		int num;
		char *str;
		void *ptr;
	} data;
	const char *desc;
};

/* can't use the system include because of the stupid broadcom header files */
extern struct ether_addr *ether_aton(const char *asc);
static inline int my_ether_ntoa(unsigned char *ea, char *buf)
{
	return sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x",
		ea[0], ea[1], ea[2], ea[3], ea[4], ea[5]);
}

static int wlc_ioctl(wlc_param param, void *data, void *value)
{
	unsigned int *var = ((unsigned int *) data);
	unsigned int ioc = *var;

	if (param & NOARG) {
		return wl_ioctl(interface, ioc, NULL, 0);
	}
	switch(param & PARAM_TYPE) {
		case MAC:
			return wl_ioctl(interface, ((param & SET) ? (ioc) : (ioc >> 16)) & 0xffff, value, 6);
		case INT:
			return wl_ioctl(interface, ((param & SET) ? (ioc) : (ioc >> 16)) & 0xffff, value, sizeof(int));
		case STRING:
			return wl_ioctl(interface, ((param & SET) ? (ioc) : (ioc >> 16)) & 0xffff, value, BUFSIZE);
	}
	return 0;
}

static int wlc_iovar(wlc_param param, void *data, void *value)
{
	int *val = (int *) value;
	char *iov = *((char **) data);
	int ret = 0;
	
	if (param & SET) {
		switch(param & PARAM_TYPE) {
			case INT:
				ret = wl_iovar_setint(interface, iov, *val);
				break;
			case MAC:
				ret = wl_iovar_set(interface, iov, value, 6);
				break;
		}
	}
	if (param & GET) {
		switch(param & PARAM_TYPE) {
			case INT:
				ret = wl_iovar_get(interface, iov, val, sizeof(int));
				break;
			case MAC:
				ret = wl_iovar_get(interface, iov, value, 6);
				break;
		}
	}

	return ret;
}

static int wlc_bssiovar(wlc_param param, void *data, void *value)
{
	int *val = (int *) value;
	char *iov = *((char **) data);
	int ret = 0;
	
	if (param & SET) {
		switch(param & PARAM_TYPE) {
			case INT:
				ret = wl_bssiovar_setint(interface, iov, vif, *val);
		}
	}
	if (param & GET) {
		switch(param & PARAM_TYPE) {
			case INT:
				ret = wl_bssiovar_get(interface, iov, vif, val, sizeof(int));
		}
	}

	return ret;
}

static int wlc_vif_enabled(wlc_param param, void *data, void *value)
{
	int *val = (int *) value;
	int buf[3];
	int ret = 0;
	
	sprintf((char *) buf, "bss");
	buf[1] = vif;
	if (param & SET) {
		buf[2] = (*val ? 1 : 0);
		ret = wl_ioctl(interface, WLC_SET_VAR, buf, sizeof(buf));
	} else if (param & GET) {
		ret = wl_ioctl(interface, WLC_GET_VAR, buf, sizeof(buf));
		*val = buf[0];
	}

	return ret;
}

static int wlc_ssid(wlc_param param, void *data, void *value)
{
	int ret = -1, ret2 = -1;
	char *dest = (char *) value;
	wlc_ssid_t ssid;
	
	if ((param & PARAM_MODE) == GET) {
		ret = wl_bssiovar_get(interface, "ssid", vif, &ssid, sizeof(ssid));

		if (ret)
			/* if we can't get the ssid through the bssiovar, try WLC_GET_SSID */
			ret = wl_ioctl(interface, WLC_GET_SSID, &ssid, sizeof(ssid));
		
		if (!ret) {
			memcpy(dest, ssid.SSID, ssid.SSID_len);
			dest[ssid.SSID_len] = 0;
		}
	} else if ((param & PARAM_MODE) == SET) {
		strncpy(ssid.SSID, value, 32);
		ssid.SSID_len = strlen(value);
		
		if (ssid.SSID_len > 32)
			ssid.SSID_len = 32;
		
		if (vif == 0) {
			/* for the main interface, also try the WLC_SET_SSID call */
			ret2 = wl_ioctl(interface, WLC_SET_SSID, &ssid, sizeof(ssid));
		}
		
		ret = wl_bssiovar_set(interface, "ssid", vif, &ssid, sizeof(ssid));
		ret = (!ret2 ? 0 : ret);
	}
	
	return ret;
}

static int wlc_int(wlc_param param, void *data, void *value)
{
	int *var = *((int **) data);
	int *val = (int *) value;

	if ((param & PARAM_MODE) == SET) {
		*var = *val;
	} else if ((param & PARAM_MODE) == GET) {
		*val = *var;
	}

	return 0;
}

static int wlc_flag(wlc_param param, void *data, void *value)
{
	int *var = *((int **) data);

	*var = 1;

	return 0;
}

static int wlc_string(wlc_param param, void *data, void *value)
{
	char *var = *((char **) data);
	
	if ((param & PARAM_MODE) == GET) {
		strcpy(value, var);
	}

	return 0;
}

static int wlc_afterburner(wlc_param param, void *data, void *value)
{
	int *val = (int *) value;
	int ret = 0;

	if ((param & PARAM_MODE) == GET) {
		ret = wl_iovar_get(interface, "afterburner", val, sizeof(int));
	} else {
		wl_iovar_setint(interface, "wlfeatureflag", (*val ? 3 : 0));
		ret = wl_iovar_setint(interface, "afterburner", (*val ? 1 : 0));
		wl_iovar_setint(interface, "afterburner_override", *val);
	}

	return ret;
}

static int wlc_maclist(wlc_param param, void *data, void *value)
{
	unsigned int *var = ((unsigned int *) data);
	unsigned int ioc = *var;
	int limit = (sizeof(wlbuf) - 4) / sizeof(struct ether_addr);
	struct maclist *list = (struct maclist *) wlbuf;
	char *str = (char *) value;
	char astr[30], *p;
	struct ether_addr *addr;
	int isset = 0;
	int ret;

	if ((param & PARAM_MODE) == GET) {
		list->count = limit;
		ret = wl_ioctl(interface, (ioc >> 16) & 0xffff, wlbuf, sizeof(wlbuf));
		
		if (!ret) 
			while (list->count) {
				str += sprintf(str, "%s", ((((char *) value) == str) ? "" : " "));
				str += my_ether_ntoa((unsigned char *) &list->ea[list->count-- - 1], str);
			}
		
		return ret;
	} else {
		while (*str && isspace(*str))
			*str++;
		
		if (*str == '+') {
			str++;

			list->count = limit;
			if (wl_ioctl(interface, (ioc >> 16) & 0xffff, wlbuf, sizeof(wlbuf)) == 0)
				isset = 1;

			while (*str && isspace(*str))
				str++;
		}
		
		if (!isset)
			memset(wlbuf, 0, sizeof(wlbuf));
		
		foreach(astr, str, p) {
			if (list->count >= limit)
				break;
			
			if ((addr = ether_aton(astr)) != NULL)
				memcpy(&list->ea[list->count++], addr, sizeof(struct ether_addr));
		}

		return wl_ioctl(interface, ioc & 0xffff, wlbuf, sizeof(wlbuf));
	}
}

static int wlc_radio(wlc_param param, void *data, void *value)
{
	int *val = (int *) value;
	int ret;

	if ((param & PARAM_MODE) == GET) {
		ret = wl_ioctl(interface, WLC_GET_RADIO, val, sizeof(int));
		*val = ((*val & 1) ? 0 : 1);
	} else {
		*val = (1 << 16) | (*val ? 0 : 1); 
		ret = wl_ioctl(interface, WLC_SET_RADIO, val, sizeof(int));
	}

	return ret;
}

static int wlc_wsec_key(wlc_param param, void *null, void *value)
{
	wl_wsec_key_t wsec_key;
	unsigned char *index = value;
	unsigned char *key;
	unsigned char *data;
	unsigned char hex[3];
	
	if ((param & PARAM_MODE) != SET)
		return 0;

	memset(&wsec_key, 0, sizeof(wsec_key));
	if (index[0] == '=') {
		wsec_key.flags = WL_PRIMARY_KEY;
		index++;
	}
	
	if ((index[0] < '1') || (index[0] > '4') || (index[1] != ','))
		return -1;
	
	key = index + 2;
	if (strncmp(key, "d:", 2) == 0) { /* delete key */
	} else if (strncmp(key, "s:", 2) == 0) { /* ascii key */
		key += 2;
		wsec_key.len = strlen(key);

		if ((wsec_key.len != 5) && (wsec_key.len != 13))
			return -1;
		
		strcpy(wsec_key.data, key);
	} else { /* hex key */
		wsec_key.len = strlen(key);
		if ((wsec_key.len != 10) && (wsec_key.len != 26))
			return -1;
		
		wsec_key.len /= 2;
		data = wsec_key.data;
		hex[2] = 0;
		do {
			hex[0] = *(key++);
			hex[1] = *(key++);
			*(data++) = (unsigned char) strtoul(hex, NULL, 16);
		} while (*key != 0);
	}

	return wl_bssiovar_set(interface, "wsec_key", vif, &wsec_key, sizeof(wsec_key));
}

static int wlc_cap(wlc_param param, void *data, void *value)
{
	char *iov = *((char **) data);

	if (param & GET)
		return wl_iovar_get(interface, iov, value, BUFSIZE);

	return -1;
}

static int wlc_bssmax(wlc_param param, void *data, void *value)
{
	int *val = (int *) value;
	char *iov = *((char **) data);
	int ret = -1;

	if (param & GET) {
		ret = wl_iovar_get(interface, iov, wlbuf, BUFSIZE);
		if (!ret) {
			if (strstr(wlbuf, "mbss4"))
				*val = 4;
			else if (strstr(wlbuf, "mbss16"))
				*val = 16;
			else
				*val = 1;
		}
	}

	return ret;
}

static inline int cw2ecw(int cw)
{
	int i;	
	for (cw++, i = 0; cw; i++) cw >>=1;
	return i - 1;
}

static int wlc_wme_ac(wlc_param param, void *data, void *value)
{
	char *type = *((char **) data);
	char *settings = (char *) value;
	char cmd[100], *p, *val;
	edcf_acparam_t params[AC_COUNT];
	int ret;
	int intval;
	int cur = -1;
	char *buf = wlbuf;

	if ((param & PARAM_MODE) != SET)
		return -1;
	
	memset(params, 0, sizeof(params));
	ret = wl_iovar_get(interface, type, params, sizeof(params));
	memset(buf, 0, BUFSIZE);
	strcpy(buf, type);
	buf += strlen(buf) + 1;
	
	foreach(cmd, settings, p) {
		val = strchr(cmd, '=');
		if (val == NULL) {
			if (strcmp(cmd, "be") == 0)
				cur = AC_BE;
			else if (strcmp(cmd, "bk") == 0)
				cur = AC_BK;
			else if (strcmp(cmd, "vi") == 0)
				cur = AC_VI;
			else if (strcmp(cmd, "vo") == 0)
				cur = AC_VO;
			else
				return -1;

			/* just in case */
			params[cur].ACI = (params[cur].ACI & (0x3 << 5)) | (cur << 5);
		} else {
			*(val++) = 0;
			
			intval = strtoul(val, NULL, 10);
			if (strcmp(cmd, "cwmin") == 0)
				params[cur].ECW = (params[cur].ECW & ~(0xf)) | cw2ecw(intval);
			else if (strcmp(cmd, "ecwmin") == 0)
				params[cur].ECW = (params[cur].ECW & ~(0xf)) | (intval & 0xf);
			else if (strcmp(cmd, "cwmax") == 0)
				params[cur].ECW = (params[cur].ECW & ~(0xf << 4)) | (cw2ecw(intval) << 4);
			else if (strcmp(cmd, "ecwmax") == 0)
				params[cur].ECW = (params[cur].ECW & ~(0xf << 4)) | ((intval & 0xf) << 4);
			else if (strcmp(cmd, "aifsn") == 0)
				params[cur].ACI = (params[cur].ACI & ~(0xf)) | (intval & 0xf);
			else if (strcmp(cmd, "txop") == 0)
				params[cur].TXOP = intval >> 5;
			else if (strcmp(cmd, "force") == 0)
				params[cur].ACI = (params[cur].ACI & ~(1 << 4)) | ((intval) ? (1 << 4) : 0);
			else return -1;
			
			memcpy(buf, &params[cur], sizeof(edcf_acparam_t));
			wl_ioctl(interface, WLC_SET_VAR, wlbuf, BUFSIZE);
		}
	}
	return ret;
}

static int wlc_ifname(wlc_param param, void *data, void *value)
{
	char *val = (char *) value;
	int ret = 0;
	
	if (param & SET) {
		if (strlen(val) < 16)
			strcpy(interface, val);
		else ret = -1;
	}
	if (param & GET) {
		strcpy(val, interface);
	}

	return ret;
}

static int wlc_wdsmac(wlc_param param, void *data, void *value)
{
	unsigned char mac[6];
	int ret = 0;
	
	ret = wl_ioctl(interface, WLC_WDS_GET_REMOTE_HWADDR, &mac, 6);
	if (ret == 0)
		my_ether_ntoa(mac, value);

	return ret;
}

static int wlc_pmk(wlc_param param, void *data, void *value)
{
	int ret = -1;
	char *str = (char *) value;
	wsec_pmk_t pmk;
	
	/* driver doesn't support GET */

	if ((param & PARAM_MODE) == SET) {
		strncpy(pmk.key, str, WSEC_MAX_PSK_LEN);
		pmk.key_len = strlen(str);

		if (pmk.key_len > WSEC_MAX_PSK_LEN)
			pmk.key_len = WSEC_MAX_PSK_LEN;

		pmk.flags = WSEC_PASSPHRASE;

		ret = wl_ioctl(interface, WLC_SET_WSEC_PMK, &pmk, sizeof(pmk));
	}
	
	return ret;
}

static const struct wlc_call wlc_calls[] = {
	{
		.name = "version",
		.param = STRING|NOARG,
		.handler = wlc_string,
		.data.str = VERSION,
		.desc = "Version of this program"
	},
	{
		.name = "debug",
		.param = INT,
		.handler = wlc_int,
		.data.ptr = &debug,
		.desc = "wlc debug level"
	},
	{
		.name = "stdin",
		.param = NOARG,
		.handler = wlc_flag,
		.data.ptr = &fromstdin,
		.desc = "Accept input from stdin"
	},
	{
		.name = "ifname",
		.param = STRING,
		.handler = wlc_ifname,
		.desc = "interface to send commands to"
	},
	{
		.name = "up",
		.param = NOARG,
		.handler = wlc_ioctl,
		.data.num = WLC_UP,
		.desc = "Bring the interface up"
	},
	{
		.name = "down",
		.param = NOARG,
		.handler = wlc_ioctl,
		.data.num = WLC_DOWN,
		.desc = "Bring the interface down"
	},
	{
		.name = "radio",
		.param = INT,
		.handler = wlc_radio,
		.desc = "Radio enabled flag"
	},
	{
		.name = "ap",
		.param = INT,
		.handler = wlc_ioctl,
		.data.num = ((WLC_GET_AP << 16) | WLC_SET_AP),
		.desc = "Access Point mode"
	},
	{
		.name = "mssid",
		.param = INT,
		.handler = wlc_iovar,
		.data.str = "mbss",
		.desc = "Multi-ssid mode"
	},
	{
		.name = "apsta",
		.param = INT,
		.handler = wlc_iovar,
		.data.str = "apsta",
		.desc = "AP+STA mode"
	},
	{
		.name = "infra",
		.param = INT,
		.handler = wlc_ioctl,
		.data.num = ((WLC_GET_INFRA << 16) | WLC_SET_INFRA),
		.desc = "Infrastructure mode"
	},
	{
		.name = "wet",
		.param = INT,
		.handler = wlc_ioctl,
		.data.num = ((WLC_GET_WET << 16) | WLC_SET_WET),
		.desc = "Wireless repeater mode",
	},
	{
		.name = "statimeout",
		.param = INT,
		.handler = wlc_iovar,
		.data.str = "sta_retry_time",
		.desc = "STA connection timeout"
	},
	{
		.name = "country",
		.param = STRING,
		.handler = wlc_ioctl,
		.data.num = ((WLC_GET_COUNTRY << 16) | WLC_SET_COUNTRY),
		.desc = "Country code"
	},
	{
		.name = "channel",
		.param = INT,
		.handler = wlc_ioctl,
		.data.num = ((WLC_GET_CHANNEL << 16) | WLC_SET_CHANNEL),
		.desc = "Channel",
	},
	{
		.name = "vlan_mode",
		.param = INT,
		.handler = wlc_bssiovar,
		.data.str = "vlan_mode",
		.desc = "Parse 802.1Q tags",
	},
	{
		.name = "vif",
		.param = INT,
		.handler = wlc_int,
		.data.ptr = &vif,
		.desc = "Current vif index"
	},
	{
		.name = "enabled",
		.param = INT,
		.handler = wlc_vif_enabled,
		.desc = "vif enabled flag"
	},
	{
		.name = "ssid",
		.param = STRING,
		.handler = wlc_ssid,
		.desc = "Interface ESSID"
	},
	{
		.name = "closed",
		.param = INT,
		.handler = wlc_bssiovar,
		.data.str = "closednet",
		.desc = "Hidden ESSID flag"
	},
	{
		.name = "wsec",
		.param = INT,
		.handler = wlc_bssiovar,
		.data.str = "wsec",
		.desc = "Security mode flags"
	},
	{
		.name = "wepkey",
		.param = STRING,
		.handler = wlc_wsec_key,
		.desc = "Set/Remove WEP keys"
	},
	{
		.name = "wepauth",
		.param = INT,
		.handler = wlc_ioctl,
		.data.num = ((WLC_GET_AUTH << 16) | WLC_SET_AUTH),
		.desc = "WEP authentication type. 0 = OpenSystem, 1 = SharedKey"
	},
	{
		.name = "wsec_restrict",
		.param = INT,
		.handler = wlc_bssiovar,
		.data.str = "wsec_restrict",
		.desc = "Drop unencrypted traffic"
	},
	{
		.name = "eap_restrict",
		.param = INT,
		.handler = wlc_bssiovar,
		.data.str = "eap_restrict",
		.desc = "Only allow 802.1X traffic until 802.1X authorized"
	},
	{
		.name = "wpa_auth",
		.param = INT,
		.handler = wlc_bssiovar,
		.data.str = "wpa_auth",
		.desc = "WPA authentication modes"
	},
	{
		.name = "ap_isolate",
		.param = INT,
		.handler = wlc_bssiovar,
		.data.str = "ap_isolate",
		.desc = "Isolate connected clients"
	},
	{
		.name = "supplicant",
		.param = INT,
		.handler = wlc_iovar,
		.data.str = "sup_wpa",
		.desc = "Built-in WPA supplicant"
	},
	{
		.name = "passphrase",
		.param = STRING,
		.handler = wlc_pmk,
		.desc = "Passphrase for built-in WPA supplicant",
	},
	{
		.name = "maxassoc",
		.param = INT,
		.handler = wlc_iovar,
		.data.str = "maxassoc",
		.desc = "Max. number of associated clients",
	},
	{
		.name = "wme",
		.param = INT,
		.handler = wlc_iovar,
		.data.str = "wme",
		.desc = "WME enabled"
	},
	{
		.name = "wme_ac_ap",
		.param = STRING,
		.handler = wlc_wme_ac,
		.data.str = "wme_ac_ap",
		.desc = "Set WME AC options for AP mode",
	},
	{
		.name = "wme_ac_sta",
		.param = STRING,
		.handler = wlc_wme_ac,
		.data.str = "wme_ac_sta",
		.desc = "Set WME AC options for STA mode",
	},
	{
		.name = "wme_noack",
		.param = INT,
		.handler = wlc_iovar,
		.data.str = "wme_noack",
		.desc = "WME ACK disable request",
	},
	{
		.name = "802.11d",
		.param = INT,
		.handler = wlc_ioctl,
		.data.num = ((WLC_GET_REGULATORY << 16) | WLC_SET_REGULATORY),
		.desc = "Enable/disable 802.11d regulatory management",
	},
	{
		.name = "802.11h",
		.param = INT,
		.handler = wlc_ioctl,
		.data.num = ((WLC_GET_SPECT_MANAGMENT << 16) | WLC_SET_SPECT_MANAGMENT),
		.desc = "Enable/disable 802.11h spectrum management",
	},
	{
		.name = "fragthresh",
		.param = INT,
		.handler = wlc_iovar,
		.data.str = "fragthresh",
		.desc = "Fragmentation threshold",
	},
	{
		.name = "rtsthresh",
		.param = INT,
		.handler = wlc_iovar,
		.data.str = "rtsthresh",
		.desc = "RTS threshold"
	},
	{
		.name = "slottime",
		.param = INT,
		.handler = wlc_iovar,
		.data.str = "acktiming",
		.desc = "Slot time"
	},
	{
		.name = "rxant",
		.param = INT,
		.handler = wlc_ioctl,
		.data.num = ((WLC_GET_ANTDIV << 16) | WLC_SET_ANTDIV),
		.desc = "Rx antenna selection"
	},
	{
		.name = "txant",
		.param = INT,
		.handler = wlc_ioctl,
		.data.num = ((WLC_GET_TXANT << 16) | WLC_SET_TXANT),
		.desc = "Tx antenna selection"
	},
	{
		.name = "dtim",
		.param = INT,
		.handler = wlc_ioctl,
		.data.num = ((WLC_GET_DTIMPRD << 16) | WLC_SET_DTIMPRD),
		.desc = "DTIM period",
	},
	{
		.name = "bcn",
		.param = INT,
		.handler = wlc_ioctl,
		.data.num = ((WLC_GET_BCNPRD << 16) | WLC_SET_BCNPRD),
		.desc = "Beacon interval"
	},
	{
		.name = "frameburst",
		.param = INT,
		.handler = wlc_ioctl,
		.data.num = ((WLC_GET_FAKEFRAG << 16) | WLC_SET_FAKEFRAG),
		.desc = "Framebursting"
	},
	{
		.name = "monitor",
		.param = INT,
		.handler = wlc_ioctl,
		.data.num = ((WLC_GET_MONITOR << 16) | WLC_SET_MONITOR),
		.desc = "Monitor mode"
	},
	{
		.name = "passive_scan",
		.param = INT,
		.handler = wlc_ioctl,
		.data.num = ((WLC_GET_PASSIVE_SCAN << 16) | WLC_SET_PASSIVE_SCAN),
		.desc = "Passive scan mode"
	},
	{
		.name = "macfilter",
		.param = INT,
		.handler = wlc_ioctl,
		.data.num = ((WLC_GET_MACMODE << 16) | WLC_SET_MACMODE),
		.desc = "MAC filter mode (0:disabled, 1:deny, 2:allow)"
	},
	{
		.name = "maclist",
		.param = STRING,
		.data.num = ((WLC_GET_MACLIST << 16) | WLC_SET_MACLIST),
		.handler = wlc_maclist,
		.desc = "MAC filter list"
	},
	{
		.name = "autowds",
		.param = INT,
		.handler = wlc_ioctl,
		.data.num = ((WLC_GET_LAZYWDS << 16) | WLC_SET_LAZYWDS),
		.desc = "Automatic WDS"
	},
	{
		.name = "wds",
		.param = STRING,
		.data.num = ((WLC_GET_WDSLIST << 16) | WLC_SET_WDSLIST),
		.handler = wlc_maclist,
		.desc = "WDS connection list"
	},
	{
		.name = "wdstimeout",
		.param = INT,
		.handler = wlc_iovar,
		.data.str = "wdstimeout",
		.desc = "WDS link detection timeout"
	},
	{
		.name = "wdsmac",
		.param = STRING|NOARG,
		.handler = wlc_wdsmac,
		.desc = "MAC of the remote WDS endpoint (only with wds0.* interfaces)"
	},
	{
		.name = "afterburner",
		.param = INT,
		.handler = wlc_afterburner,
		.desc = "Broadcom Afterburner"
	},
	{
		.name = "ibss_merge",
		.param = INT,
		.handler = wlc_iovar,
		.data.str = "ibss_coalesce_allowed",
		.desc = "Allow IBSS merges"
	},
	{
		.name = "bssid",
		.param = MAC,
		.handler = wlc_ioctl,
		.data.num = ((WLC_GET_BSSID << 16) | WLC_SET_BSSID),
		.desc = "BSSID"
	},
	{
		.name = "cur_etheraddr",
		.param = MAC,
		.handler = wlc_iovar,
		.data.str = "cur_etheraddr",
		.desc = "Current MAC Address"
	},
	{
		.name = "default_bssid",
		.param = MAC,
		.handler = wlc_iovar,
		.data.str = "perm_etheraddr",
		.desc = "Default BSSID (read-only)"
	},
	{
		.name = "assoclist",
		.param = STRING,
		.data.num = (WLC_GET_ASSOCLIST << 16),
		.handler = wlc_maclist,
		.desc = "MACs of associated stations"
	},
	{
		.name = "gmode",
		.param = INT,
		.data.num = ((WLC_GET_GMODE << 16) | WLC_SET_GMODE),
		.handler = wlc_ioctl,
		.desc = "G Mode"
	},
	{
		.name = "phytype",
		.param = INT,
		.data.num = (WLC_GET_PHYTYPE << 16),
		.handler = wlc_ioctl,
		.desc = "PHY Type (read-only)"
	},
	{
		.name = "nmode",
		.param = INT,
		.handler = wlc_iovar,
		.data.str = "nmode",
		.desc = "N Mode"
	},
	{
		.name = "nreqd",
		.param = INT,
		.handler = wlc_iovar,
		.data.str = "nreqd",
		.desc = "N Mode required"
	},
	{
		.name = "chanspec",
		.param = INT,
		.handler = wlc_iovar,
		.data.str = "chanspec",
		.desc = "Channel Spec (See bcmwifi.h)"
	},
	{
		.name = "band",
		.param = INT,
		.data.num = ((WLC_GET_BAND << 16) | WLC_SET_BAND),
		.handler = wlc_ioctl,
		.desc = "Band (0=auto, 1=5Ghz, 2=2.4GHz)"
	},
	{
		.name = "cap",
		.param = STRING|NOARG,
		.handler = wlc_cap,
		.data.str = "cap",
		.desc = "Capabilities"
	},
	{
		.name = "bssmax",
		.param = INT|NOARG,
		.handler = wlc_bssmax,
		.data.str = "cap",
		.desc = "Number of VIF's supported"
	},
	{
		.name = "leddc",
		.param = INT,
		.handler = wlc_iovar,
		.data.str = "leddc",
		.desc = "LED Duty Cycle"
	},
	
};
#define wlc_calls_size (sizeof(wlc_calls) / sizeof(struct wlc_call))

static void usage(char *cmd)
{
	int i;
	fprintf(stderr, "Usage: %s <command> [<argument> ...]\n"
					"\n"
					"Available commands:\n", cmd);
	for (i = 0; i < wlc_calls_size; i++) {
		fprintf(stderr, "\t%-16s\t%s\n", wlc_calls[i].name ?: "", wlc_calls[i].desc ?: "");
	}
	fprintf(stderr, "\n");
	exit(1);
}

static int do_command(const struct wlc_call *cmd, char *arg)
{
	static char buf[BUFSIZE];
	int set;
	int ret = 0;
	char *format, *end;
	int intval;
	void *ptr = (void *) buf;

	if (debug >= 10) {
		fprintf(stderr, "do_command %-16s\t'%s'\n", cmd->name, arg);
	}
	
	if ((arg == NULL) && ((cmd->param & PARAM_TYPE) != NONE)) {
		set = 0;
		ret = cmd->handler(cmd->param | GET, (void *) &cmd->data, (void *) buf);
		if (ret == 0) {
			switch(cmd->param & PARAM_TYPE) {
				case INT:
					intval = *((int *) buf);
					
					if (intval > 65535)
						format = "0x%08x\n";
					else if (intval > 255)
						format = "0x%04x\n";
					else
						format = "%d\n";
					
					fprintf(stdout, format, intval);
					break;
				case STRING:
					fprintf(stdout, "%s\n", buf);
					break;
				case MAC:
					my_ether_ntoa(buf, buf + 6);
					fprintf(stdout, "%s\n", buf + 6);
					break;
			}
		}
	} else { /* SET */
		set = 1;
		switch(cmd->param & PARAM_TYPE) {
			case INT:
				intval = strtoul(arg, &end, 0);
				if (end && !(*end)) {
					memcpy(buf, &intval, sizeof(intval));
				} else {
					fprintf(stderr, "%s: Invalid argument\n", cmd->name);
					return -1;
				}
				break;
			case STRING:
				strncpy(buf, arg, BUFSIZE);
				buf[BUFSIZE - 1] = 0;
				break;
			case MAC:
				ptr = ether_aton(arg);
				if (!ptr) {
					fprintf(stderr, "%s: Invalid mac address '%s'\n", cmd->name, arg);
					return -1;
				}
				break;
		}

		ret = cmd->handler(cmd->param | SET, (void *) &cmd->data, ptr);
	}
	
	if ((debug > 0) && (ret != 0)) 
		fprintf(stderr, "Command '%s %s' failed: %d\n", (set == 1 ? "set" : "get"), cmd->name, ret);
	
	return ret;
}

static struct wlc_call *find_cmd(char *name)
{
	int found = 0, i = 0;

	while (!found && (i < wlc_calls_size)) {
		if (strcmp(name, wlc_calls[i].name) == 0)
			found = 1;
		else
			i++;
	}

	return (struct wlc_call *) (found ? &wlc_calls[i] : NULL);
}

int main(int argc, char **argv)
{
	static char buf[BUFSIZE];
	char *s, *s2;
	char *cmd = argv[0];
	struct wlc_call *call;
	int ret = 0;

	if (argc < 2)
		usage(argv[0]);

	for(interface[2] = '0'; (interface[2] < '3') && (wl_probe(interface) != 0); interface[2]++);
	if (interface[2] == '3') {
		fprintf(stderr, "No Broadcom wl interface found!\n");
		return -1;
	}

	argv++;
	argc--;
	while ((argc > 0) && (argv[0] != NULL)) {
		if ((call = find_cmd(argv[0])) == NULL) {
			fprintf(stderr, "Invalid command: %s\n\n", argv[0]);
			usage(cmd);
		}
		if ((argc > 1) && (!(call->param & NOARG))) {
			ret = do_command(call, argv[1]);
			argv += 2;
			argc -= 2;
		} else {
			ret = do_command(call, NULL);
			argv++;
			argc--;
		}
	}

	while (fromstdin && !feof(stdin)) {
		*buf = 0;
		fgets(buf, BUFSIZE - 1, stdin);
		
		if (*buf == 0)
			continue;
		
		if ((s = strchr(buf, '\r')) != NULL)
			*s = 0;
		if ((s = strchr(buf, '\n')) != NULL)
			*s = 0;

		s = buf;
		while (isspace(*s))
			s++;

		if (!*s)
			continue;
	
		if ((s2 = strchr(s, ' ')) != NULL)
			*(s2++) = 0;
		
		while (s2 && isspace(*s2))
			s2++;
		
		if ((call = find_cmd(s)) == NULL) {
			fprintf(stderr, "Invalid command: %s\n", s);
			ret = -1;
		} else
			ret = do_command(call, ((call->param & NOARG) ? NULL : s2));
	}

	return ret;
}
