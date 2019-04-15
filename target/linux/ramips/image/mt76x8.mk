#
# MT76x8 Profiles
#

DEVICE_VARS += SERCOMM_HWID SERCOMM_HWVER SERCOMM_SWVER

define Build/sercom-seal
	$(STAGING_DIR_HOST)/bin/mksercommfw \
		-i $@ \
		-b $(SERCOMM_HWID) \
		-r $(SERCOMM_HWVER) \
		-v $(SERCOMM_SWVER) \
		$(1)
endef

define Build/sercom-footer
	$(call Build/sercom-seal,-f)
endef


define Device/tplink
  TPLINK_FLASHLAYOUT :=
  TPLINK_HWID :=
  TPLINK_HWREV :=
  TPLINK_HWREVADD :=
  TPLINK_HVERSION :=
  KERNEL := $(KERNEL_DTB)
  KERNEL_INITRAMFS := $(KERNEL_DTB) | tplink-v2-header -e
  IMAGES += tftp-recovery.bin
  IMAGE/factory.bin := tplink-v2-image -e
  IMAGE/tftp-recovery.bin := pad-extra 128k | $$(IMAGE/factory.bin)
  IMAGE/sysupgrade.bin := tplink-v2-image -s -e | append-metadata | \
	check-size $$$$(IMAGE_SIZE)
endef
DEVICE_VARS += TPLINK_FLASHLAYOUT TPLINK_HWID TPLINK_HWREV TPLINK_HWREVADD TPLINK_HVERSION


define Device/alfa-network_awusfree1
  DTS := AWUSFREE1
  IMAGE_SIZE := $(ralink_default_fw_size_8M)
  DEVICE_TITLE := ALFA Network AWUSFREE1
  DEVICE_PACKAGES := uboot-envtools
endef
TARGET_DEVICES += alfa-network_awusfree1

define Device/tama_w06
  DTS := W06
  IMAGE_SIZE := 15040k
  DEVICE_TITLE := Tama W06
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += tama_w06

define Device/duzun-dm06
  DTS := DUZUN-DM06
  DEVICE_TITLE := DuZun DM06
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += duzun-dm06

define Device/gl-mt300n-v2
  DTS := GL-MT300N-V2
  IMAGE_SIZE := 16064k
  DEVICE_TITLE := GL-iNet GL-MT300N-V2
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += gl-mt300n-v2

define Device/glinet_vixmini
  DTS := VIXMINI
  IMAGE_SIZE := 7872k
  DEVICE_TITLE := GL.iNet VIXMINI
  SUPPORTED_DEVICES += vixmini
endef
TARGET_DEVICES += glinet_vixmini

define Device/hc5661a
  DTS := HC5661A
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := HiWiFi HC5661A
endef
TARGET_DEVICES += hc5661a

define Device/hiwifi_hc5861b
  DTS := HC5861B
  IMAGE_SIZE := 15808k
  DEVICE_TITLE := HiWiFi HC5861B
  DEVICE_PACKAGES := kmod-mt76x2
endef
TARGET_DEVICES += hiwifi_hc5861b

define Device/LinkIt7688
  DTS := LINKIT7688
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
  SUPPORTED_DEVICES := linkits7688 linkits7688d
  DEVICE_TITLE := MediaTek LinkIt Smart 7688
  DEVICE_PACKAGES:= kmod-usb2 kmod-usb-ohci uboot-envtools
endef
TARGET_DEVICES += LinkIt7688

define Device/mac1200r-v2
  DTS := MAC1200RV2
  DEVICE_TITLE := Mercury MAC1200R v2.0
  SUPPORTED_DEVICES := mac1200rv2
  DEVICE_PACKAGES := kmod-mt76x2
endef
TARGET_DEVICES += mac1200r-v2

define Device/miwifi-nano
  DTS := MIWIFI-NANO
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Xiaomi MiWiFi Nano
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += miwifi-nano

define Device/mt7628
  DTS := MT7628
  BLOCKSIZE := 64k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := MediaTek MT7628 EVB
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += mt7628

define Device/netgear_r6120
  DTS := R6120
  BLOCKSIZE := 64k
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Netgear R6120
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci
  SERCOMM_HWID := CGQ
  SERCOMM_HWVER := A001
  SERCOMM_SWVER := 0x0040
  IMAGES += factory.img
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE)| append-rootfs | pad-rootfs
  IMAGE/sysupgrade.bin := $$(IMAGE/default) | append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.img := pad-extra 576k | $$(IMAGE/default) | \
	sercom-footer | pad-to 128 | zip R6120.bin | sercom-seal
endef
TARGET_DEVICES += netgear_r6120

define Device/omega2
  DTS := OMEGA2
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Onion Omega2
  DEVICE_PACKAGES:= kmod-usb2 kmod-usb-ohci uboot-envtools
endef
TARGET_DEVICES += omega2

define Device/omega2p
  DTS := OMEGA2P
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
  DEVICE_TITLE := Onion Omega2+
  DEVICE_PACKAGES:= kmod-usb2 kmod-usb-ohci uboot-envtools kmod-sdhci-mt7620
endef
TARGET_DEVICES += omega2p

define Device/pbr-d1
  DTS := PBR-D1
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := PBR-D1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += pbr-d1

define Device/rakwireless_rak633
  DTS := RAK633
  DEVICE_TITLE := Rakwireless RAK633
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += rakwireless_rak633

define Device/skylab_skw92a
  DTS := SKW92A
  IMAGE_SIZE := 16064k
  DEVICE_TITLE := Skylab SKW92A
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += skylab_skw92a

define Device/tplink_tl-wa801nd-v5
  $(Device/tplink)
  DTS := TL-WA801NDV5
  IMAGE_SIZE := 7808k
  DEVICE_TITLE := TP-Link TL-WA801ND v5
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0x08010005
  TPLINK_HWREV := 0x1
  TPLINK_HWREVADD := 0x5
  TPLINK_HVERSION := 3
endef
TARGET_DEVICES += tplink_tl-wa801nd-v5

define Device/tplink_tl-wr802n-v4
  $(Device/tplink)
  DTS := TL-WR802NV4
  IMAGE_SIZE := 7808k
  DEVICE_TITLE := TP-Link TL-WR802N v4
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0x08020004
  TPLINK_HWREV := 0x1
  TPLINK_HWREVADD := 0x4
  TPLINK_HVERSION := 3
endef
TARGET_DEVICES += tplink_tl-wr802n-v4

define Device/tl-wr840n-v4
  $(Device/tplink)
  DTS := TL-WR840NV4
  IMAGE_SIZE := 7808k
  DEVICE_TITLE := TP-Link TL-WR840N v4
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0x08400004
  TPLINK_HWREV := 0x1
  TPLINK_HWREVADD := 0x4
  TPLINK_HVERSION := 3
endef
TARGET_DEVICES += tl-wr840n-v4

define Device/tl-wr840n-v5
  DTS := TL-WR840NV5
  IMAGE_SIZE := 3904k
  DEVICE_TITLE := TP-Link TL-WR840N v5
  TPLINK_FLASHLAYOUT := 4Mmtk
  TPLINK_HWID := 0x08400005
  TPLINK_HWREV := 0x1
  TPLINK_HWREVADD := 0x5
  TPLINK_HVERSION := 3
  KERNEL := $(KERNEL_DTB)
  KERNEL_INITRAMFS := $(KERNEL_DTB) | tplink-v2-header -e
  IMAGE/sysupgrade.bin := tplink-v2-image -s -e | append-metadata | \
	check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += tl-wr840n-v5

define Device/tl-wr841n-v13
  $(Device/tplink)
  DTS := TL-WR841NV13
  IMAGE_SIZE := 7808k
  DEVICE_TITLE := TP-Link TL-WR841N v13
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0x08410013
  TPLINK_HWREV := 0x268
  TPLINK_HWREVADD := 0x13
  TPLINK_HVERSION := 3
endef
TARGET_DEVICES += tl-wr841n-v13

define Device/tplink_c20-v4
  $(Device/tplink)
  DTS := ArcherC20v4
  IMAGE_SIZE := 7808k
  DEVICE_TITLE := TP-Link ArcherC20 v4
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0xc200004
  TPLINK_HWREV := 0x1
  TPLINK_HWREVADD := 0x4
  TPLINK_HVERSION := 3
  DEVICE_PACKAGES := kmod-mt76x0e
endef
TARGET_DEVICES += tplink_c20-v4

define Device/tplink_c50-v3
  $(Device/tplink)
  DTS := ArcherC50V3
  IMAGE_SIZE := 7808k
  DEVICE_TITLE := TP-Link ArcherC50 v3
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0x001D9BA4
  TPLINK_HWREV := 0x79
  TPLINK_HWREVADD := 0x1
  TPLINK_HVERSION := 3
  DEVICE_PACKAGES := kmod-mt76x2
endef
TARGET_DEVICES += tplink_c50-v3

define Device/tplink_c50-v4
  $(Device/tplink)
  DTS := ArcherC50V4
  IMAGE_SIZE := 7616k
  DEVICE_TITLE := TP-Link ArcherC50 v4
  TPLINK_FLASHLAYOUT := 8MSUmtk
  TPLINK_HWID := 0x001D589B
  TPLINK_HWREV := 0x93
  TPLINK_HWREVADD := 0x2
  TPLINK_HVERSION := 3
  DEVICE_PACKAGES := kmod-mt76x2
  IMAGES := sysupgrade.bin
endef
TARGET_DEVICES += tplink_c50-v4

define Device/tplink_tl-mr3020-v3
  $(Device/tplink)
  DTS := TL-MR3020V3
  IMAGE_SIZE := 7808k
  DEVICE_TITLE := TP-Link TL-MR3020 v3
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0x30200003
  TPLINK_HWREV := 0x3
  TPLINK_HWREVADD := 0x3
  TPLINK_HVERSION := 3
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += tplink_tl-mr3020-v3

define Device/tplink_tl-mr3420-v5
  $(Device/tplink)
  DTS := TL-MR3420V5
  IMAGE_SIZE := 7808k
  DEVICE_TITLE := TP-Link TL-MR3420 v5
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0x34200005
  TPLINK_HWREV := 0x5
  TPLINK_HWREVADD := 0x5
  TPLINK_HVERSION := 3
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += tplink_tl-mr3420-v5

define Device/tplink_tl-wr842n-v5
  $(Device/tplink)
  DTS := TL-WR842NV5
  IMAGE_SIZE := 7808k
  DEVICE_TITLE := TP-Link TL-WR842N v5
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0x08420005
  TPLINK_HWREV := 0x5
  TPLINK_HWREVADD := 0x5
  TPLINK_HVERSION := 3
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += tplink_tl-wr842n-v5

define Device/tplink_tl-wr902ac-v3
  $(Device/tplink)
  DTS := TL-WR902ACV3
  IMAGE_SIZE := 7808k
  DEVICE_TITLE := TP-Link TL-WR902AC v3
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0x000dc88f
  TPLINK_HWREV := 0x89
  TPLINK_HWREVADD := 0x1
  TPLINK_HVERSION := 3
  DEVICE_PACKAGES := kmod-mt76x0e kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += tplink_tl-wr902ac-v3

define Device/u7628-01-128M-16M
  DTS := U7628-01-128M-16M
  IMAGE_SIZE := 16064k
  DEVICE_TITLE := UniElec U7628-01 (128M RAM/16M flash)
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += u7628-01-128M-16M

define Device/vocore2
  DTS := VOCORE2
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := VoCore VoCore2
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport \
    kmod-sdhci-mt7620
endef
TARGET_DEVICES += vocore2

define Device/vocore2lite
  DTS := VOCORE2LITE
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := VoCore VoCore2-Lite
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport \
    kmod-sdhci-mt7620
endef
TARGET_DEVICES += vocore2lite

define Device/wavlink_wl-wn570ha1
  DTS := WL-WN570HA1
  IMAGE_SIZE := $(ralink_default_fw_size_8M)
  DEVICE_TITLE := Wavlink WL-WN570HA1
  DEVICE_PACKAGES := kmod-mt76x0e
endef
TARGET_DEVICES += wavlink_wl-wn570ha1

define Device/wavlink_wl-wn575a3
  DTS := WL-WN575A3
  IMAGE_SIZE := $(ralink_default_fw_size_8M)
  DEVICE_TITLE := Wavlink WL-WN575A3
  DEVICE_PACKAGES := kmod-mt76x2
  SUPPORTED_DEVICES += wl-wn575a3
endef
TARGET_DEVICES += wavlink_wl-wn575a3

define Device/wcr-1166ds
  DTS := WCR-1166DS
  BUFFALO_TAG_PLATFORM := MTK
  BUFFALO_TAG_VERSION := 9.99
  BUFFALO_TAG_MINOR := 9.99
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := trx | pad-rootfs | append-metadata
  IMAGE/factory.bin := \
	trx -M 0x746f435c | pad-rootfs | append-metadata | \
	buffalo-enc WCR-1166DS $$(BUFFALO_TAG_VERSION) -l | \
	buffalo-tag-dhp WCR-1166DS JP JP | buffalo-enc-tag -l | \
	buffalo-dhp-image
  DEVICE_TITLE := Buffalo WCR-1166DS
  DEVICE_PACKAGES := kmod-mt76x2
endef
TARGET_DEVICES += wcr-1166ds

define Device/widora_neo-16m
  DTS := WIDORA-NEO-16M
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Widora-NEO (16M)
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += widora-neo
endef
TARGET_DEVICES += widora_neo-16m

define Device/widora_neo-32m
  DTS := WIDORA-NEO-32M
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
  DEVICE_TITLE := Widora-NEO (32M)
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += widora_neo-32m

define Device/wiznet_wizfi630s
  DTS := WIZFI630S
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
  DEVICE_TITLE := WIZnet WizFi630S
endef
TARGET_DEVICES += wiznet_wizfi630s

define Device/wrtnode2p
  DTS := WRTNODE2P
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := WRTnode 2P
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += wrtnode2p

define Device/wrtnode2r
  DTS := WRTNODE2R
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := WRTnode 2R
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += wrtnode2r

define Device/zbtlink_zbt-we1226
  DTS := ZBT-WE1226
  IMAGE_SIZE := $(ralink_default_fw_size_8M)
  DEVICE_TITLE := ZBTlink ZBT-WE1226
endef
TARGET_DEVICES += zbtlink_zbt-we1226

define Device/zyxel_keenetic-extra-ii
  DTS := ki_rb
  IMAGE_SIZE := 14912k
  BLOCKSIZE := 64k
  DEVICE_TITLE := ZyXEL Keenetic Extra II
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | pad-to $$$$(BLOCKSIZE) | \
	check-size $$$$(IMAGE_SIZE) | zyimage -d 6162 -v "ZyXEL Keenetic Extra II"
endef
TARGET_DEVICES += zyxel_keenetic-extra-ii
