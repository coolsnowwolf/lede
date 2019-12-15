#
# MT76x8 Profiles
#

define Device/alfa-network_awusfree1
  MTK_SOC := mt7628an
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := ALFA Network
  DEVICE_MODEL := AWUSFREE1
  DEVICE_PACKAGES := uboot-envtools
endef
TARGET_DEVICES += alfa-network_awusfree1

define Device/buffalo_wcr-1166ds
  MTK_SOC := mt7628an
  IMAGE_SIZE := 7936k
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
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WCR-1166DS
  DEVICE_PACKAGES := kmod-mt76x2
  SUPPORTED_DEVICES += wcr-1166ds
endef
TARGET_DEVICES += buffalo_wcr-1166ds

define Device/cudy_wr1000
  MTK_SOC := mt7628an
  IMAGE_SIZE := 7872k
  IMAGES += factory.bin
  IMAGE/factory.bin := \
        $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | jcg-header 92.122
  JCG_MAXSIZE := 8060928
  DEVICE_VENDOR := Cudy
  DEVICE_MODEL := WR1000
  DEVICE_PACKAGES := kmod-mt76x2
  SUPPORTED_DEVICES += wr1000
endef
TARGET_DEVICES += cudy_wr1000

define Device/d-team_pbr-d1
  MTK_SOC := mt7628an
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := PandoraBox
  DEVICE_MODEL := PBR-D1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += pbr-d1
endef
TARGET_DEVICES += d-team_pbr-d1

define Device/duzun_dm06
  MTK_SOC := mt7628an
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := DuZun
  DEVICE_MODEL := DM06
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += duzun-dm06
endef
TARGET_DEVICES += duzun_dm06

define Device/glinet_gl-mt300n-v2
  MTK_SOC := mt7628an
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := GL-MT300N
  DEVICE_VARIANT := V2
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += gl-mt300n-v2
endef
TARGET_DEVICES += glinet_gl-mt300n-v2

define Device/glinet_vixmini
  MTK_SOC := mt7628an
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := VIXMINI
  SUPPORTED_DEVICES += vixmini
endef
TARGET_DEVICES += glinet_vixmini

define Device/hilink_hlk-7628n
  MTK_SOC := mt7628an
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := HILINK
  DEVICE_MODEL := HLK-7628N
endef
TARGET_DEVICES += hilink_hlk-7628n

define Device/hiwifi_hc5661a
  MTK_SOC := mt7628an
  IMAGE_SIZE := 15808k
  DEVICE_VENDOR := HiWiFi
  DEVICE_MODEL := HC5661A
  SUPPORTED_DEVICES += hc5661a
endef
TARGET_DEVICES += hiwifi_hc5661a

define Device/hiwifi_hc5761a
  MTK_SOC := mt7628an
  IMAGE_SIZE := 15808k
  DEVICE_VENDOR := HiWiFi
  DEVICE_MODEL := HC5761A
  DEVICE_PACKAGES := kmod-mt76x0e kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += hiwifi_hc5761a

define Device/hiwifi_hc5861b
  MTK_SOC := mt7628an
  IMAGE_SIZE := 15808k
  DEVICE_VENDOR := HiWiFi
  DEVICE_MODEL := HC5861B
  DEVICE_PACKAGES := kmod-mt76x2
endef
TARGET_DEVICES += hiwifi_hc5861b

define Device/iptime_a3
  MTK_SOC := mt7628an
  IMAGE_SIZE := 7936k
  UIMAGE_NAME := a3
  DEVICE_VENDOR := ipTIME
  DEVICE_MODEL := A3
  DEVICE_PACKAGES := kmod-mt76x2
endef
TARGET_DEVICES += iptime_a3

define Device/iptime_a604m
  MTK_SOC := mt7628an
  IMAGE_SIZE := 7936k
  UIMAGE_NAME := a604m
  DEVICE_VENDOR := ipTIME
  DEVICE_MODEL := A604M
  DEVICE_PACKAGES := kmod-mt76x2
endef
TARGET_DEVICES += iptime_a604m

define Device/mediatek_linkit-smart-7688
  MTK_SOC := mt7628an
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := LinkIt Smart 7688
  DEVICE_PACKAGES:= kmod-usb2 kmod-usb-ohci uboot-envtools kmod-sdhci-mt7620
  SUPPORTED_DEVICES += linkits7688 linkits7688d
endef
TARGET_DEVICES += mediatek_linkit-smart-7688

define Device/mediatek_mt7628an-eval-board
  MTK_SOC := mt7628an
  BLOCKSIZE := 64k
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MT7628 EVB
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += mt7628
endef
TARGET_DEVICES += mediatek_mt7628an-eval-board

define Device/mercury_mac1200r-v2
  MTK_SOC := mt7628an
  IMAGE_SIZE := 7936k
  DEVICE_VENDOR := Mercury
  DEVICE_MODEL := MAC1200R
  DEVICE_VARIANT := v2.0
  SUPPORTED_DEVICES := mac1200rv2
  DEVICE_PACKAGES := kmod-mt76x2
  SUPPORTED_DEVICES += mac1200rv2
endef
TARGET_DEVICES += mercury_mac1200r-v2

define Device/netgear_r6120
  MTK_SOC := mt7628an
  BLOCKSIZE := 64k
  IMAGE_SIZE := 15744k
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := R6120
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci
  SERCOMM_HWID := CGQ
  SERCOMM_HWVER := A001
  SERCOMM_SWVER := 0x0040
  IMAGES += factory.img
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE)| append-rootfs | pad-rootfs
  IMAGE/sysupgrade.bin := $$(IMAGE/default) | append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.img := pad-extra 576k | $$(IMAGE/default) | pad-to $$$$(BLOCKSIZE) | \
	sercom-footer | pad-to 128 | zip R6120.bin | sercom-seal
endef
TARGET_DEVICES += netgear_r6120

define Device/onion_omega2
  MTK_SOC := mt7628an
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Onion
  DEVICE_MODEL := Omega2
  DEVICE_PACKAGES:= kmod-usb2 kmod-usb-ohci uboot-envtools
  SUPPORTED_DEVICES += omega2
endef
TARGET_DEVICES += onion_omega2

define Device/onion_omega2p
  MTK_SOC := mt7628an
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := Onion
  DEVICE_MODEL := Omega2+
  DEVICE_PACKAGES:= kmod-usb2 kmod-usb-ohci uboot-envtools kmod-sdhci-mt7620
  SUPPORTED_DEVICES += omega2p
endef
TARGET_DEVICES += onion_omega2p

define Device/rakwireless_rak633
  MTK_SOC := mt7628an
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Rakwireless
  DEVICE_MODEL := RAK633
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += rakwireless_rak633

define Device/skylab_skw92a
  MTK_SOC := mt7628an
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Skylab
  DEVICE_MODEL := SKW92A
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += skylab_skw92a

define Device/tama_w06
  MTK_SOC := mt7628an
  IMAGE_SIZE := 15040k
  DEVICE_VENDOR := Tama
  DEVICE_MODEL := W06
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += tama_w06

define Device/totolink_lr1200
  MTK_SOC := mt7628an
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := TOTOLINK
  DEVICE_MODEL := LR1200
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 uqmi
endef
TARGET_DEVICES += totolink_lr1200

define Device/tplink
  MTK_SOC := mt7628an
  DEVICE_VENDOR := TP-Link
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

define Device/tplink_archer-c20-v4
  $(Device/tplink)
  IMAGE_SIZE := 7808k
  DEVICE_MODEL := Archer C20
  DEVICE_VARIANT := v4
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0xc200004
  TPLINK_HWREV := 0x1
  TPLINK_HWREVADD := 0x4
  TPLINK_HVERSION := 3
  DEVICE_PACKAGES := kmod-mt76x0e
  SUPPORTED_DEVICES += tplink,c20-v4
endef
TARGET_DEVICES += tplink_archer-c20-v4

define Device/tplink_archer-c50-v3
  $(Device/tplink)
  IMAGE_SIZE := 7808k
  DEVICE_MODEL := Archer C50
  DEVICE_VARIANT := v3
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0x001D9BA4
  TPLINK_HWREV := 0x79
  TPLINK_HWREVADD := 0x1
  TPLINK_HVERSION := 3
  DEVICE_PACKAGES := kmod-mt76x2
  SUPPORTED_DEVICES += tplink,c50-v3
endef
TARGET_DEVICES += tplink_archer-c50-v3

define Device/tplink_archer-c50-v4
  $(Device/tplink)
  IMAGE_SIZE := 7616k
  DEVICE_MODEL := Archer C50
  DEVICE_VARIANT := v4
  TPLINK_FLASHLAYOUT := 8MSUmtk
  TPLINK_HWID := 0x001D589B
  TPLINK_HWREV := 0x93
  TPLINK_HWREVADD := 0x2
  TPLINK_HVERSION := 3
  DEVICE_PACKAGES := kmod-mt76x2
  IMAGES := sysupgrade.bin
  SUPPORTED_DEVICES += tplink,c50-v4
endef
TARGET_DEVICES += tplink_archer-c50-v4

define Device/tplink_tl-mr3020-v3
  $(Device/tplink)
  IMAGE_SIZE := 7808k
  DEVICE_MODEL := TL-MR3020
  DEVICE_VARIANT := v3
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
  IMAGE_SIZE := 7808k
  DEVICE_MODEL := TL-MR3420
  DEVICE_VARIANT := v5
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0x34200005
  TPLINK_HWREV := 0x5
  TPLINK_HWREVADD := 0x5
  TPLINK_HVERSION := 3
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += tplink_tl-mr3420-v5

define Device/tplink_tl-wa801nd-v5
  $(Device/tplink)
  IMAGE_SIZE := 7808k
  DEVICE_MODEL := TL-WA801ND
  DEVICE_VARIANT := v5
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0x08010005
  TPLINK_HWREV := 0x1
  TPLINK_HWREVADD := 0x5
  TPLINK_HVERSION := 3
endef
TARGET_DEVICES += tplink_tl-wa801nd-v5

define Device/tplink_tl-wr802n-v4
  $(Device/tplink)
  IMAGE_SIZE := 7808k
  DEVICE_MODEL := TL-WR802N
  DEVICE_VARIANT := v4
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0x08020004
  TPLINK_HWREV := 0x1
  TPLINK_HWREVADD := 0x4
  TPLINK_HVERSION := 3
endef
TARGET_DEVICES += tplink_tl-wr802n-v4

define Device/tplink_tl-wr840n-v4
  $(Device/tplink)
  IMAGE_SIZE := 7808k
  DEVICE_MODEL := TL-WR840N
  DEVICE_VARIANT := v4
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0x08400004
  TPLINK_HWREV := 0x1
  TPLINK_HWREVADD := 0x4
  TPLINK_HVERSION := 3
  SUPPORTED_DEVICES += tl-wr840n-v4
endef
TARGET_DEVICES += tplink_tl-wr840n-v4

define Device/tplink_tl-wr840n-v5
  MTK_SOC := mt7628an
  IMAGE_SIZE := 3904k
  DEVICE_VENDOR := TP-Link
  DEVICE_MODEL := TL-WR840N
  DEVICE_VARIANT := v5
  TPLINK_FLASHLAYOUT := 4Mmtk
  TPLINK_HWID := 0x08400005
  TPLINK_HWREV := 0x1
  TPLINK_HWREVADD := 0x5
  TPLINK_HVERSION := 3
  KERNEL := $(KERNEL_DTB)
  KERNEL_INITRAMFS := $(KERNEL_DTB) | tplink-v2-header -e
  IMAGE/sysupgrade.bin := tplink-v2-image -s -e | append-metadata | \
	check-size $$$$(IMAGE_SIZE)
  SUPPORTED_DEVICES += tl-wr840n-v5
endef
TARGET_DEVICES += tplink_tl-wr840n-v5

define Device/tplink_tl-wr841n-v13
  $(Device/tplink)
  IMAGE_SIZE := 7808k
  DEVICE_MODEL := TL-WR841N
  DEVICE_VARIANT := v13
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0x08410013
  TPLINK_HWREV := 0x268
  TPLINK_HWREVADD := 0x13
  TPLINK_HVERSION := 3
  SUPPORTED_DEVICES += tl-wr841n-v13
endef
TARGET_DEVICES += tplink_tl-wr841n-v13

define Device/tplink_tl-wr841n-v14
  $(Device/tplink)
  IMAGE_SIZE := 3968k
  DEVICE_MODEL := TL-WR841N
  DEVICE_VARIANT := v14
  TPLINK_FLASHLAYOUT := 4Mmtk
  TPLINK_HWID := 0x08410014
  TPLINK_HWREV := 0x1
  TPLINK_HWREVADD := 0x14
  TPLINK_HVERSION := 3
  IMAGE/tftp-recovery.bin := pad-extra 64k | $$(IMAGE/factory.bin)
endef
TARGET_DEVICES += tplink_tl-wr841n-v14

define Device/tplink_tl-wr842n-v5
  $(Device/tplink)
  IMAGE_SIZE := 7808k
  DEVICE_MODEL := TL-WR842N
  DEVICE_VARIANT := v5
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
  IMAGE_SIZE := 7808k
  DEVICE_MODEL := TL-WR902AC
  DEVICE_VARIANT := v3
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0x000dc88f
  TPLINK_HWREV := 0x89
  TPLINK_HWREVADD := 0x1
  TPLINK_HVERSION := 3
  DEVICE_PACKAGES := kmod-mt76x0e kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += tplink_tl-wr902ac-v3

define Device/unielec_u7628-01-16m
  MTK_SOC := mt7628an
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := UniElec
  DEVICE_MODEL := U7628-01
  DEVICE_VARIANT := 16M
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += u7628-01-128M-16M unielec,u7628-01-128m-16m
endef
TARGET_DEVICES += unielec_u7628-01-16m

define Device/vocore_vocore2
  MTK_SOC := mt7628an
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := VoCore
  DEVICE_MODEL := VoCore2
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport \
    kmod-sdhci-mt7620
  SUPPORTED_DEVICES += vocore2
endef
TARGET_DEVICES += vocore_vocore2

define Device/vocore_vocore2-lite
  MTK_SOC := mt7628an
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := VoCore
  DEVICE_MODEL := VoCore2-Lite
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport \
    kmod-sdhci-mt7620
  SUPPORTED_DEVICES += vocore2lite
endef
TARGET_DEVICES += vocore_vocore2-lite

define Device/wavlink_wl-wn570ha1
  MTK_SOC := mt7628an
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Wavlink
  DEVICE_MODEL := WL-WN570HA1
  DEVICE_PACKAGES := kmod-mt76x0e
endef
TARGET_DEVICES += wavlink_wl-wn570ha1

define Device/wavlink_wl-wn575a3
  MTK_SOC := mt7628an
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Wavlink
  DEVICE_MODEL := WL-WN575A3
  DEVICE_PACKAGES := kmod-mt76x2
  SUPPORTED_DEVICES += wl-wn575a3
endef
TARGET_DEVICES += wavlink_wl-wn575a3

define Device/widora_neo-16m
  MTK_SOC := mt7628an
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Widora
  DEVICE_MODEL := Widora-NEO
  DEVICE_VARIANT := 16M
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += widora-neo
endef
TARGET_DEVICES += widora_neo-16m

define Device/widora_neo-32m
  MTK_SOC := mt7628an
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := Widora
  DEVICE_MODEL := Widora-NEO
  DEVICE_VARIANT := 32M
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += widora_neo-32m

define Device/wiznet_wizfi630s
  MTK_SOC := mt7628an
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := WIZnet
  DEVICE_MODEL := WizFi630S
endef
TARGET_DEVICES += wiznet_wizfi630s

define Device/wrtnode_wrtnode2p
  MTK_SOC := mt7628an
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := WRTnode
  DEVICE_MODEL := WRTnode 2P
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += wrtnode2p
endef
TARGET_DEVICES += wrtnode_wrtnode2p

define Device/wrtnode_wrtnode2r
  MTK_SOC := mt7628an
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := WRTnode
  DEVICE_MODEL := WRTnode 2R
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += wrtnode2r
endef
TARGET_DEVICES += wrtnode_wrtnode2r

define Device/xiaomi_mir4a-100m
  MTK_SOC := mt7628an
  IMAGE_SIZE := 14976k
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Mi Router 4A
  DEVICE_VARIANT := 100M Edition
  DEVICE_PACKAGES := kmod-mt76x2
endef
TARGET_DEVICES += xiaomi_mir4a-100m

define Device/xiaomi_miwifi-nano
  MTK_SOC := mt7628an
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := MiWiFi Nano
  DEVICE_PACKAGES := uboot-envtools
  SUPPORTED_DEVICES += miwifi-nano
endef
TARGET_DEVICES += xiaomi_miwifi-nano

define Device/zbtlink_zbt-we1226
  MTK_SOC := mt7628an
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-WE1226
endef
TARGET_DEVICES += zbtlink_zbt-we1226

define Device/zyxel_keenetic-extra-ii
  MTK_SOC := mt7628an
  IMAGE_SIZE := 14912k
  BLOCKSIZE := 64k
  DEVICE_VENDOR := ZyXEL
  DEVICE_MODEL := Keenetic Extra II
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | pad-to $$$$(BLOCKSIZE) | \
	check-size $$$$(IMAGE_SIZE) | zyimage -d 6162 -v "ZyXEL Keenetic Extra II"
endef
TARGET_DEVICES += zyxel_keenetic-extra-ii
