#include "QMIThread.h"
//2021-03-15 zhangkaibo@fibocom.com changed begin for oa 20210311037
#include <net/if.h>
//2021-03-15 zhangkaibo@fibocom.com changed end for oa 20210311037

static size_t fibo_fread(const char *filename, void *buf, size_t size)
{
    FILE *fp = fopen(filename, "r");
    size_t n = 0;

    memset(buf, 0x00, size);

    if (fp) {
	n = fread(buf, 1, size, fp);
	if (n <= 0 || n == size) {
	    dbg_time(
		"warnning: fail to fread(%s), fread=%zd, buf_size=%zd, "
		"errno: %d (%s)",
		__func__, filename, n, size, errno, strerror(errno));
	}
	fclose(fp);
    }

    return n > 0 ? n : 0;
}

static size_t fibo_fwrite(const char *filename, const void *buf, size_t size)
{
    FILE *fp = fopen(filename, "w");
    size_t n = 0;

    if (fp) {
	n = fwrite(buf, 1, size, fp);
	if (n != size) {
	    dbg_time(
		"warnning: fail to fwrite(%s), fwrite=%zd, buf_size=%zd, "
		"errno: %d (%s)",
		__func__, filename, n, size, errno, strerror(errno));
	}
	fclose(fp);
    }

    return n > 0 ? n : 0;
}

static int fibo_iface_is_in_bridge(const char *iface)
{
    char filename[256];

    snprintf(filename, sizeof(filename), "/sys/class/net/%s/brport", iface);
    return (access(filename, F_OK) == 0 || errno != ENOENT);
}

int fibo_bridge_mode_detect(PROFILE_T *profile)
{
    const char *ifname = profile->qmapnet_adapter ? profile->qmapnet_adapter
						  : profile->usbnet_adapter;
    const char *driver;
    char bridge_mode[128];
    char bridge_ipv4[128];
    char ipv4[128];
    char buf[64];
    size_t n;
    int in_bridge;

    driver = profile->driver_name;
    snprintf(bridge_mode, sizeof(bridge_mode), "/sys/class/net/%s/bridge_mode",
	     ifname);
    snprintf(bridge_ipv4, sizeof(bridge_ipv4), "/sys/class/net/%s/bridge_ipv4",
	     ifname);

    if (access(bridge_mode, F_OK) && errno == ENOENT) {
	snprintf(bridge_mode, sizeof(bridge_mode),
		 "/sys/module/%s/parameters/bridge_mode", driver);
	snprintf(bridge_ipv4, sizeof(bridge_ipv4),
		 "/sys/module/%s/parameters/bridge_ipv4", driver);

	if (access(bridge_mode, F_OK) && errno == ENOENT) {
	    bridge_mode[0] = '\0';
	}
    }

    in_bridge = fibo_iface_is_in_bridge(ifname);
    if (in_bridge) {
	dbg_time("notice: iface %s had add to bridge\n", ifname);
    }

    if (in_bridge && bridge_mode[0] == '\0') {
	dbg_time("warnning: can not find bride_mode file for %s\n", ifname);
	return 1;
    }

    n = fibo_fread(bridge_mode, buf, sizeof(buf));

    if (in_bridge) {
	if (n <= 0 || buf[0] == '0') {
	    dbg_time("warnning: should set 1 to bride_mode file for %s\n",
		     ifname);
	    return 1;
	}
    } else {
	if (n <= 0 || buf[0] == '0') {
	    return 0;
	}
    }

    memset(ipv4, 0, sizeof(ipv4));

    if (strstr(bridge_ipv4, "/sys/class/net/") || profile->qmap_mode == 0 ||
	profile->qmap_mode == 1) {
	snprintf(ipv4, sizeof(ipv4), "0x%x", profile->ipv4.Address);
	dbg_time("echo '%s' > %s", ipv4, bridge_ipv4);
	fibo_fwrite(bridge_ipv4, ipv4, strlen(ipv4));
    } else {
	snprintf(ipv4, sizeof(ipv4), "0x%x:%d", profile->ipv4.Address,
		 profile->muxid);
	dbg_time("echo '%s' > %s", ipv4, bridge_ipv4);
	fibo_fwrite(bridge_ipv4, ipv4, strlen(ipv4));
    }

    return 1;
}
//2021-03-15 zhangkaibo@fibocom.com changed begin for oa 20210311037
void fibo_get_driver_info(PROFILE_T *profile, RMNET_INFO *rmnet_info) {
    int ifc_ctl_sock;
    struct ifreq ifr;
    int rc;
    int request = 0x89F3;
    unsigned char data[512];

    memset(rmnet_info, 0x00, sizeof(*rmnet_info));

    ifc_ctl_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (ifc_ctl_sock <= 0) {
        dbg_time("socket() failed: %s\n", strerror(errno));
        return;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, profile->usbnet_adapter, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;
    ifr.ifr_ifru.ifru_data = (void *)data;
    rc = ioctl(ifc_ctl_sock, request, &ifr);
    if (rc < 0) {
        dbg_time("ioctl(0x%x, qmap_settings) failed: %s, rc=%d", request, strerror(errno), rc);
    }
    else {
        memcpy(rmnet_info, data, sizeof(*rmnet_info));
    }

    close(ifc_ctl_sock);
}
//2021-03-15 zhangkaibo@fibocom.com changed end for oa 20210311037
int fibo_qmap_mode_detect(PROFILE_T *profile)
{
    int n;
    char buf[128];
    char qmap_netcard[128];
    struct {
	char filename[255 * 2];
	char linkname[255 * 2];
    } * pl;

    pl = (typeof(pl))malloc(sizeof(*pl));

    snprintf(pl->linkname, sizeof(pl->linkname),
	     "/sys/class/net/%s/device/driver", profile->usbnet_adapter);
    n = readlink(pl->linkname, pl->filename, sizeof(pl->filename));
    pl->filename[n] = '\0';
    while (pl->filename[n] != '/') n--;
    profile->driver_name = strdup(&pl->filename[n + 1]);

//2021-03-15 zhangkaibo@fibocom.com changed begin for oa 20210311037
    fibo_get_driver_info(profile, &profile->rmnet_info);
    if (profile->rmnet_info.size) {
        profile->qmap_mode = profile->rmnet_info.qmap_mode;
        if (profile->qmap_mode) {
            int offset_id = profile->pdp - 1;

            if (profile->qmap_mode == 1)
                offset_id = 0;
            profile->muxid = profile->rmnet_info.mux_id[offset_id];
            profile->qmapnet_adapter = strdup( profile->rmnet_info.ifname[offset_id]);
            profile->qmap_size = profile->rmnet_info.rx_urb_size;
            profile->qmap_version = profile->rmnet_info.qmap_version;
        }

        goto _out;
    }
//2021-03-15 zhangkaibo@fibocom.com changed end for oa 20210311037

    if (qmidev_is_pciemhi(profile->qmichannel)) {
	profile->qmap_mode = 1;
    
        if (profile->muxid == 0 || profile->muxid  == 0x81)
        {
            profile->muxid = 0x81;
        }
        else
        {
            if (profile->muxid  < 0x80)
                profile->muxid += 0x81;
            profile->qmap_mode = 2;
        }
        
	profile->qmapnet_adapter = strdup(profile->usbnet_adapter);
       goto _final_process;
    }

    snprintf(pl->filename, sizeof(pl->filename), "/sys/class/net/%s/qmap_num",
	     profile->usbnet_adapter);

//2021-01-27 willa.liu@fibocom.com changed begin for support mantis 0068849
    if (access(pl->filename, F_OK) == 0) {
        dbg_time("access %s", pl->filename);
	if (errno != ENOENT) {
	    dbg_time("fail to access %s, errno: %d (%s)", pl->filename, errno,
		     strerror(errno));
	    goto _out;
	}
//2021-02-01 willa.liu@fibocom.com changed begin for support mantis 0069837
/*
	snprintf(pl->filename, sizeof(pl->filename),
		 "/sys/module/%s/parameters/qmap_num", profile->driver_name);
	if (access(pl->filename, R_OK)) {
	    if (errno != ENOENT) {
		dbg_time("fail to access %s, errno: %d (%s)", pl->filename,
			 errno, strerror(errno));
		goto _out;
	    }

	    snprintf(
		pl->filename, sizeof(pl->filename),
		"/sys/class/net/%s/device/driver/module/parameters/qmap_num",
		profile->usbnet_adapter);
	    if (access(pl->filename, R_OK)) {
		if (errno != ENOENT) {
		    dbg_time("fail to access %s, errno: %d (%s)", pl->filename,
			     errno, strerror(errno));
		    goto _out;
		}
	    }
	}
*/
//2021-02-01 willa.liu@fibocom.com changed end for support mantis 0069837
    }
    else
    {
    snprintf(pl->filename, sizeof(pl->filename), "/sys/class/net/%s/qmap_mode",
        profile->usbnet_adapter);

        if (access(pl->filename, F_OK) == 0) {
        dbg_time("access %s", pl->filename);
            if (errno != ENOENT) {
                dbg_time("fail to access %s, errno: %d (%s)", pl->filename, errno,
                    strerror(errno));
                goto _out;
            }
        }
    }

    if(!access(pl->filename, R_OK))
    {
        n = fibo_fread(pl->filename, buf, sizeof(buf));
        if(n > 0)
        {
            profile->qmap_mode = atoi(buf);
            if(profile->qmap_mode >= 1 && qmidev_is_pciemhi(profile->qmichannel))
            {
                profile->muxid =
                    profile->pdp + 0x80;
                sprintf(qmap_netcard, "%s.%d", profile->usbnet_adapter,profile->pdp);
                profile->qmapnet_adapter = strdup(qmap_netcard);
            }
            if(qmidev_is_gobinet(profile->qmichannel) || qmidev_is_qmiwwan(profile->qmichannel))
            {
                if(profile->qmap_mode > 1)
                {
                    profile->muxid =
                        profile->pdp + 0x80;
                    sprintf(qmap_netcard, "%s.%d", profile->usbnet_adapter,profile->pdp);
                    profile->qmapnet_adapter = strdup(qmap_netcard);
                }
                if(profile->qmap_mode == 1)
                {
                    profile->muxid = 0x81;
                    profile->qmapnet_adapter = strdup(profile->usbnet_adapter);
                }
            }
        }


    if (0) {
        profile->qmap_mode = atoi(buf);

        if (profile->qmap_mode > 1 && qmidev_is_gobinet(profile->qmichannel)) {
        profile->muxid =
            profile->pdp + 0x80;  // muxis is 0x8X for PDN-X
        sprintf(qmap_netcard, "%s.%d", profile->usbnet_adapter,profile->pdp);
        profile->qmapnet_adapter = strdup(qmap_netcard);
        }
        if (profile->qmap_mode >= 1 && !qmidev_is_gobinet(profile->qmichannel)) {
        profile->muxid =
            profile->pdp + 0x80;  // muxis is 0x8X for PDN-X
        sprintf(qmap_netcard, "%s.%d", profile->usbnet_adapter,profile->pdp);
        profile->qmapnet_adapter = strdup(qmap_netcard);
        }
        if (profile->qmap_mode == 1 && qmidev_is_gobinet(profile->qmichannel)) {
            profile->muxid = 0x81;
            profile->qmapnet_adapter = strdup(profile->usbnet_adapter);
//2021-01-27 willa.liu@fibocom.com changed end for support mantis 0068849

	    }
	}
    } else if (qmidev_is_qmiwwan(profile->qmichannel)) {
	snprintf(pl->filename, sizeof(pl->filename), "/sys/class/net/qmimux%d",
		 profile->pdp - 1);
	if (access(pl->filename, R_OK)) {
	    if (errno != ENOENT) {
		dbg_time("fail to access %s, errno: %d (%s)", pl->filename,
			 errno, strerror(errno));
	    }
	    goto _out;
	}

	// upstream Kernel Style QMAP qmi_wwan.c
	snprintf(pl->filename, sizeof(pl->filename),
		 "/sys/class/net/%s/qmi/add_mux", profile->usbnet_adapter);
	n = fibo_fread(pl->filename, buf, sizeof(buf));
	if (n >= 5) {
	    profile->qmap_mode = n / 5;	 // 0x81\n0x82\n0x83\n
	    if (profile->qmap_mode > 1) {
		// PDN-X map to qmimux-X
		profile->muxid = (buf[5 * (profile->pdp - 1) + 2] - '0') * 16 +
				 (buf[5 * (profile->pdp - 1) + 3] - '0');
		sprintf(qmap_netcard, "qmimux%d", profile->pdp - 1);
		profile->qmapnet_adapter = strdup(qmap_netcard);
	    } else if (profile->qmap_mode == 1) {
		profile->muxid =
		    (buf[5 * 0 + 2] - '0') * 16 + (buf[5 * 0 + 3] - '0');
		sprintf(qmap_netcard, "qmimux%d", 0);
		profile->qmapnet_adapter = strdup(qmap_netcard);
	    }
	}
    }

_out:
    if (profile->qmap_mode) {
	profile->qmap_size = 16 * 1024;
	snprintf(pl->filename, sizeof(pl->filename),
		 "/sys/class/net/%s/qmap_size", profile->usbnet_adapter);
	if (!access(pl->filename, R_OK)) {
	    size_t n;
	    char buf[32];
	    n = fibo_fread(pl->filename, buf, sizeof(buf));
	    if (n > 0) {
		profile->qmap_size = atoi(buf);
	    }
	}
    }

_final_process:
    if (profile->qmap_mode) 
        dbg_time("qmap_mode = %d, muxid = 0x%02x, qmap_netcard = %s",
        	 profile->qmap_mode, profile->muxid, profile->qmapnet_adapter);

    free(pl);

    return 0;
}

int fibo_qmap_mode_set(PROFILE_T *profile)
{
    int n;
    char buf[128];
    struct {
	char filename[255 * 2];
	char linkname[255 * 2];
    } * pl;

    if (qmidev_is_pciemhi(profile->qmichannel))
    {
        dbg_time("pcie mode exit fibo_qmap_mode_set ");
        return 0;
    }

    pl = (typeof(pl))malloc(sizeof(*pl));

    snprintf(pl->linkname, sizeof(pl->linkname),
	     "/sys/class/net/%s/device/driver", profile->usbnet_adapter);
    n = readlink(pl->linkname, pl->filename, sizeof(pl->filename));
    pl->filename[n] = '\0';
    while (pl->filename[n] != '/') n--;
    profile->driver_name = strdup(&pl->filename[n + 1]);

    snprintf(pl->filename, sizeof(pl->filename), "/sys/class/net/%s/qmap_num",
	     profile->usbnet_adapter);
    if (access(pl->filename, R_OK)) {
	if (errno != ENOENT) {
	    dbg_time("fail to access %s, errno: %d (%s)", pl->filename, errno,
		     strerror(errno));
	    goto _out;
	}

	snprintf(pl->filename, sizeof(pl->filename),
		 "/sys/module/%s/parameters/qmap_mnum", profile->driver_name);
	if (access(pl->filename, R_OK)) {
	    if (errno != ENOENT) {
		dbg_time("fail to access %s, errno: %d (%s)", pl->filename,
			 errno, strerror(errno));
		goto _out;
	    }

	    snprintf(
		pl->filename, sizeof(pl->filename),
		"/sys/class/net/%s/device/driver/module/parameters/qmap_num",
		profile->usbnet_adapter);
	    if (access(pl->filename, R_OK)) {
		if (errno != ENOENT) {
		    dbg_time("fail to access %s, errno: %d (%s)", pl->filename,
			     errno, strerror(errno));
		    goto _out;
		}
	    }
	}
    }

    if (!access(pl->filename, R_OK)) {
	snprintf(buf, sizeof(buf), "%d", profile->pdpnum);
	n = fibo_fwrite(pl->filename, buf, strlen(buf));
    }

_out:

    free(pl);

    return 0;
}
