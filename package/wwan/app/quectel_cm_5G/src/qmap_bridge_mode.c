/******************************************************************************
  @file    qmap_bridge_mode.c
  @brief   Connectivity bridge manager.

  DESCRIPTION
  Connectivity Management Tool for USB network adapter of Quectel wireless cellular modules.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2016 - 2023 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
#include "QMIThread.h"

static size_t ql_fread(const char *filename, void *buf, size_t size) {
    FILE *fp = fopen(filename , "r");
    size_t n = 0;

    memset(buf, 0x00, size);

    if (fp) {
        n = fread(buf, 1, size, fp);
        if (n <= 0 || n == size) {
            dbg_time("warnning: fail to fread(%s), fread=%zu, buf_size=%zu: (%s)", filename, n, size, strerror(errno));
        }
        fclose(fp);
    }

    return n > 0 ? n : 0;
}

static size_t ql_fwrite(const char *filename, const void *buf, size_t size) {
    FILE *fp = fopen(filename , "w");
    size_t n = 0;

    if (fp) {
        n = fwrite(buf, 1, size, fp);
        if (n != size) {
            dbg_time("warnning: fail to fwrite(%s), fwrite=%zu, buf_size=%zu: (%s)", filename, n, size, strerror(errno));
        }
        fclose(fp);
    }

    return n > 0 ? n : 0;
}

int ql_bridge_mode_detect(PROFILE_T *profile) {
    const char *ifname = profile->qmapnet_adapter[0] ? profile->qmapnet_adapter : profile->usbnet_adapter;
    const char *driver;
    char bridge_mode[128];
    char bridge_ipv4[128];
    char ipv4[128];
    char buf[64];
    size_t n;
    int in_bridge = 0;

    driver = profile->driver_name;
    snprintf(bridge_mode, sizeof(bridge_mode), "/sys/class/net/%s/bridge_mode", ifname);
    snprintf(bridge_ipv4, sizeof(bridge_ipv4), "/sys/class/net/%s/bridge_ipv4", ifname);

    if (access(bridge_ipv4, R_OK)) {
        if (errno != ENOENT) {
            dbg_time("fail to access %s, errno: %d (%s)", bridge_mode, errno, strerror(errno));
            return 0;
        }

        snprintf(bridge_mode, sizeof(bridge_mode), "/sys/module/%s/parameters/bridge_mode", driver);
        snprintf(bridge_ipv4, sizeof(bridge_ipv4), "/sys/module/%s/parameters/bridge_ipv4", driver);

        if (access(bridge_mode, R_OK)) {
            if (errno != ENOENT) {
                dbg_time("fail to access %s, errno: %d (%s)", bridge_mode, errno, strerror(errno));
            }
            return 0;
        }
    }

    n = ql_fread(bridge_mode, buf, sizeof(buf));
    if (n > 0) {
        in_bridge = (buf[0] != '0');
    }
    if (!in_bridge)
        return 0;
   
    memset(ipv4, 0, sizeof(ipv4));

    if (strstr(bridge_ipv4, "/sys/class/net/") || profile->qmap_mode == 0 || profile->qmap_mode == 1) {
        snprintf(ipv4, sizeof(ipv4), "0x%x", profile->ipv4.Address);
        dbg_time("echo '%s' > %s", ipv4, bridge_ipv4);
        ql_fwrite(bridge_ipv4, ipv4, strlen(ipv4));
    }
    else {
        snprintf(ipv4, sizeof(ipv4), "0x%x:%d", profile->ipv4.Address, profile->muxid);
        dbg_time("echo '%s' > %s", ipv4, bridge_ipv4);
        ql_fwrite(bridge_ipv4, ipv4, strlen(ipv4));
    }

    return in_bridge;
}

int ql_enable_qmi_wwan_rawip_mode(PROFILE_T *profile) {
    char filename[256];
    char buf[4];
    size_t n;
    FILE *fp;

    if (!qmidev_is_qmiwwan(profile->qmichannel))
        return 0;

    snprintf(filename, sizeof(filename), "/sys/class/net/%s/qmi/rawip", profile->usbnet_adapter);
    n = ql_fread(filename, buf, sizeof(buf));

    if (n == 0)
        return 0;

    if (buf[0] == '1' || buf[0] == 'Y')
        return 0;

    fp = fopen(filename , "w");
    if (fp == NULL) {
        dbg_time("Fail to fopen(%s, \"w\"), errno: %d (%s)", filename, errno, strerror(errno));
        return 1;
    }

    buf[0] = 'Y';
    n = fwrite(buf, 1, 1, fp);
    if (n != 1) {
        dbg_time("Fail to fwrite(%s), errno: %d (%s)", filename, errno, strerror(errno));
        fclose(fp);
        return 1;
    }
    fclose(fp);

    return 0;
}

int ql_driver_type_detect(PROFILE_T *profile) {
    if (qmidev_is_gobinet(profile->qmichannel)) {
        profile->qmi_ops = &gobi_qmidev_ops;
    }
    else {
        profile->qmi_ops = &qmiwwan_qmidev_ops;
    }
    qmidev_send = profile->qmi_ops->send;

    return 0;
}

void ql_set_driver_bridge_mode(PROFILE_T *profile) {
    char enable[16];
    char filename[256];

    if(profile->qmap_mode)
        snprintf(filename, sizeof(filename), "/sys/class/net/%s/bridge_mode", profile->qmapnet_adapter);
    else
        snprintf(filename, sizeof(filename), "/sys/class/net/%s/bridge_mode", profile->usbnet_adapter);
    snprintf(enable, sizeof(enable), "%02d\n", profile->enable_bridge);
    ql_fwrite(filename, enable, sizeof(enable));
}

static int ql_qmi_qmap_mode_detect(PROFILE_T *profile) {
    char buf[128];
    int n;
    struct {
        char filename[255 * 2];
        char linkname[255 * 2];
    } *pl;

    pl = (typeof(pl)) malloc(sizeof(*pl));

    snprintf(pl->linkname, sizeof(pl->linkname), "/sys/class/net/%s/device/driver", profile->usbnet_adapter);
    n = readlink(pl->linkname, pl->filename, sizeof(pl->filename));
    pl->filename[n] = '\0';
    while (pl->filename[n] != '/')
        n--;
    strncpy(profile->driver_name, &pl->filename[n+1], sizeof(profile->driver_name) - 1);

    ql_get_driver_rmnet_info(profile, &profile->rmnet_info);
    if (profile->rmnet_info.size) {
        profile->qmap_mode = profile->rmnet_info.qmap_mode;
        if (profile->qmap_mode) {
            int offset_id = (profile->muxid == 0)? profile->pdp - 1 : profile->muxid - 0x81;

            if (profile->qmap_mode == 1)
                offset_id = 0;
            profile->muxid = profile->rmnet_info.mux_id[offset_id];
            strncpy(profile->qmapnet_adapter, profile->rmnet_info.ifname[offset_id], sizeof(profile->qmapnet_adapter) - 1);
            profile->qmap_size = profile->rmnet_info.rx_urb_size;
            profile->qmap_version = profile->rmnet_info.qmap_version;
        }

        goto _out;
    }

    snprintf(pl->filename, sizeof(pl->filename), "/sys/class/net/%s/qmap_mode", profile->usbnet_adapter);
    if (access(pl->filename, R_OK)) {
        if (errno != ENOENT) {
            dbg_time("fail to access %s, errno: %d (%s)", pl->filename, errno, strerror(errno));
            goto _out;
        }
        
        snprintf(pl->filename, sizeof(pl->filename), "/sys/module/%s/parameters/qmap_mode", profile->driver_name);
        if (access(pl->filename, R_OK)) {
            if (errno != ENOENT) {
                dbg_time("fail to access %s, errno: %d (%s)", pl->filename, errno, strerror(errno));
                goto _out;
            }
            
            snprintf(pl->filename, sizeof(pl->filename), "/sys/class/net/%s/device/driver/module/parameters/qmap_mode", profile->usbnet_adapter);
            if (access(pl->filename, R_OK)) {
                if (errno != ENOENT) {
                    dbg_time("fail to access %s, errno: %d (%s)", pl->filename, errno, strerror(errno));
                    goto _out;
                }
            }
        }
    }

    if (!access(pl->filename, R_OK)) {
        n = ql_fread(pl->filename, buf, sizeof(buf));
        if (n > 0) {
            profile->qmap_mode = atoi(buf);
            
            if (profile->qmap_mode > 1) {
                if(!profile->muxid)
                	profile->muxid = profile->pdp + 0x80; //muxis is 0x8X for PDN-X
                snprintf(profile->qmapnet_adapter, sizeof(profile->qmapnet_adapter),
                    "%.16s.%d", profile->usbnet_adapter, profile->muxid - 0x80);
           } if (profile->qmap_mode == 1) {
                profile->muxid = 0x81;
                strncpy(profile->qmapnet_adapter, profile->usbnet_adapter, sizeof(profile->qmapnet_adapter));
           }
        }
    }
    else if (qmidev_is_qmiwwan(profile->qmichannel)) {
        snprintf(pl->filename, sizeof(pl->filename), "/sys/class/net/qmimux%d", profile->pdp - 1);
        if (access(pl->filename, R_OK)) {
            if (errno != ENOENT) {
                dbg_time("fail to access %s, errno: %d (%s)", pl->filename, errno, strerror(errno));
            }
            goto _out;
        }

        //upstream Kernel Style QMAP qmi_wwan.c
        snprintf(pl->filename, sizeof(pl->filename), "/sys/class/net/%s/qmi/add_mux", profile->usbnet_adapter);
        n = ql_fread(pl->filename, buf, sizeof(buf));
        if (n >= 5) {
            dbg_time("If use QMAP by /sys/class/net/%s/qmi/add_mux", profile->usbnet_adapter);
            #if 1
            dbg_time("Please set mtu of wwan0 >= max dl qmap packet size");
            #else
            dbg_time("File:%s Line:%d Please make sure add next patch to qmi_wwan.c", __func__, __LINE__);
            /*
            diff --git a/drivers/net/usb/qmi_wwan.c b/drivers/net/usb/qmi_wwan.c
            index 74bebbd..db8a777 100644
            --- a/drivers/net/usb/qmi_wwan.c
            +++ b/drivers/net/usb/qmi_wwan.c
            @@ -379,6 +379,24 @@ static ssize_t add_mux_store(struct device *d,  struct device_attribute *attr, c
                if (!ret) {
                        info->flags |= QMI_WWAN_FLAG_MUX;
                        ret = len;
            +#if 1 //Add by Quectel
            +               if (le16_to_cpu(dev->udev->descriptor.idVendor) == 0x2c7c) {
            +                       int idProduct = le16_to_cpu(dev->udev->descriptor.idProduct);
            +
            +                       if (idProduct == 0x0121 || idProduct == 0x0125 || idProduct == 0x0435) //MDM9x07
            +                               dev->rx_urb_size = 4*1024;
            +                       else if (idProduct == 0x0306) //MDM9x40
            +                               dev->rx_urb_size = 16*1024;
            +                       else if (idProduct == 0x0512) //SDX20
            +                               dev->rx_urb_size = 32*1024;
            +                       else if (idProduct == 0x0620) //SDX24
            +                               dev->rx_urb_size = 32*1024;
            +                       else if (idProduct == 0x0800) //SDX55
            +                               dev->rx_urb_size = 32*1024;
            +                       else
            +                               dev->rx_urb_size = 32*1024;
            +               }
            +#endif
                }
            err:
                rtnl_unlock();
            */
            #endif
            profile->qmap_mode = n/5; //0x11\n0x22\n0x33\n
            if (profile->qmap_mode > 1) {
                //PDN-X map to qmimux-X
                if(!profile->muxid) {
                    profile->muxid = (buf[5*(profile->pdp - 1) + 2] - '0')*16 + (buf[5*(profile->pdp - 1) + 3] - '0');
                    snprintf(profile->qmapnet_adapter, sizeof(profile->qmapnet_adapter), "qmimux%d", profile->pdp - 1);
                } else {
                    profile->muxid = (buf[5*(profile->muxid - 0x81) + 2] - '0')*16 + (buf[5*(profile->muxid - 0x81) + 3] - '0');
                    snprintf(profile->qmapnet_adapter, sizeof(profile->qmapnet_adapter), "qmimux%d", profile->muxid - 0x81);
                }
            } else if (profile->qmap_mode == 1) {
                profile->muxid = (buf[5*0 + 2] - '0')*16 + (buf[5*0 + 3] - '0');
                snprintf(profile->qmapnet_adapter, sizeof(profile->qmapnet_adapter),
                    "qmimux%d", 0);
            }
        }
    } 

_out:
    if (profile->qmap_mode) {
        if (profile->qmap_size == 0) {
            profile->qmap_size = 16*1024;
            snprintf(pl->filename, sizeof(pl->filename), "/sys/class/net/%s/qmap_size", profile->usbnet_adapter);
            if (!access(pl->filename, R_OK)) {
                size_t n;
                char buf[32];
                n = ql_fread(pl->filename, buf, sizeof(buf));
                if (n > 0) {
                    profile->qmap_size = atoi(buf);
                }
            }
        }

        if (profile->qmap_version == 0) {
            profile->qmap_version = WDA_DL_DATA_AGG_QMAP_ENABLED;
        }

        dbg_time("qmap_mode = %d, qmap_version = %d, qmap_size = %d, muxid = 0x%02x, qmap_netcard = %s",
            profile->qmap_mode, profile->qmap_version, profile->qmap_size, profile->muxid, profile->qmapnet_adapter);
    }
    ql_set_driver_bridge_mode(profile);
    free(pl);

    return 0;
}

static int ql_mbim_usb_vlan_mode_detect(PROFILE_T *profile) {
    char tmp[128];

    snprintf(tmp, sizeof(tmp), "/sys/class/net/%s.%d", profile->usbnet_adapter, profile->pdp);
    if (!access(tmp, F_OK)) {
        profile->qmap_mode = 4;
        profile->muxid = profile->pdp;
        no_trunc_strncpy(profile->qmapnet_adapter, tmp + strlen("/sys/class/net/"), sizeof(profile->qmapnet_adapter) - 1);

        dbg_time("mbim_qmap_mode = %d, vlan_id = 0x%02x, qmap_netcard = %s",
            profile->qmap_mode, profile->muxid, profile->qmapnet_adapter);
    }

    return 0;
}

static int ql_mbim_mhi_qmap_mode_detect(PROFILE_T *profile) {
    ql_get_driver_rmnet_info(profile, &profile->rmnet_info);
    if (profile->rmnet_info.size) {
        profile->qmap_mode = profile->rmnet_info.qmap_mode;
        if (profile->qmap_mode) {
            int offset_id = profile->pdp - 1;

            if (profile->qmap_mode == 1)
                offset_id = 0;
            profile->muxid = profile->pdp;
            strcpy(profile->qmapnet_adapter, profile->rmnet_info.ifname[offset_id]);
            profile->qmap_size = profile->rmnet_info.rx_urb_size;
            profile->qmap_version = profile->rmnet_info.qmap_version;
        
            dbg_time("mbim_qmap_mode = %d, vlan_id = 0x%02x, qmap_netcard = %s",
                profile->qmap_mode, profile->muxid, profile->qmapnet_adapter);
        }

        goto _out;
    }
    
_out:
    return 0;
}

int ql_qmap_mode_detect(PROFILE_T *profile) {
    if (profile->software_interface == SOFTWARE_MBIM) {
        if (profile->hardware_interface == HARDWARE_USB)
            return ql_mbim_usb_vlan_mode_detect(profile);
        else if (profile->hardware_interface == HARDWARE_PCIE)
            return ql_mbim_mhi_qmap_mode_detect(profile);
    } else if (profile->software_interface == SOFTWARE_QMI) {
        return ql_qmi_qmap_mode_detect(profile);
    }
#ifdef CONFIG_QRTR
    else if(profile->software_interface == SOFTWARE_QRTR) {
        char tmp[128];

        profile->qmap_mode = 4;
        profile->qmap_version = WDA_DL_DATA_AGG_QMAP_V5_ENABLED;
        profile->qmap_size = 31*1024;
        profile->muxid = 0x80 | profile->pdp;
        snprintf(profile->qmapnet_adapter, sizeof(profile->qmapnet_adapter), "rmnet_data%d", profile->muxid&0xF);

        snprintf(tmp, sizeof(tmp), "/sys/class/net/%s", profile->qmapnet_adapter);
        if (access(tmp, F_OK)) {
            rtrmnet_ctl_create_vnd(profile->usbnet_adapter, profile->qmapnet_adapter,
                profile->muxid, profile->qmap_version, 11, 4096);
        }
    }
#endif
    return 0;
}
