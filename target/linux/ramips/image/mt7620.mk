#
# MT7620A Profiles
#

DEVICE_VARS += TPLINK_FLASHLAYOUT TPLINK_HWID TPLINK_HWREV TPLINK_HWREVADD TPLINK_HVERSION \
	DLINK_ROM_ID DLINK_FAMILY_MEMBER DLINK_FIRMWARE_SIZE

define Build/elecom-header
	cp $@ $(KDIR)/v_0.0.0.bin
	( \
		mkhash md5 $(KDIR)/v_0.0.0.bin && \
		echo 458 \
	) | mkhash md5 > $(KDIR)/v_0.0.0.md5
	$(STAGING_DIR_HOST)/bin/tar -c \
		$(if $(SOURCE_DATE_EPOCH),--mtime=@$(SOURCE_DATE_EPOCH)) \
		-f $@ -C $(KDIR) v_0.0.0.bin v_0.0.0.md5
endef

define Device/ai-br100
  DTS := AI-BR100
  IMAGE_SIZE := 7936k
  DEVICE_TITLE := Aigale Ai-BR100
  DEVICE_PACKAGES:= kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += ai-br100

define Device/alfa-network_ac1200rm
  DTS := AC1200RM
  IMAGE_SIZE := 16064k
  DEVICE_TITLE := ALFA Network AC1200RM
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci uboot-envtools
endef
TARGET_DEVICES += alfa-network_ac1200rm

define Device/Archer
  TPLINK_HWREVADD := 0
  TPLINK_HVERSION := 3
  KERNEL := $(KERNEL_DTB)
  KERNEL_INITRAMFS := $(KERNEL_DTB) | tplink-v2-header -e
  IMAGE/factory.bin := tplink-v2-image -e
  IMAGE/sysupgrade.bin := tplink-v2-image -s -e | append-metadata
endef

define Device/ArcherC20i
  $(Device/Archer)
  DTS := ArcherC20i
  SUPPORTED_DEVICES := c20i
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0xc2000001
  TPLINK_HWREV := 58
  IMAGES += factory.bin
  DEVICE_PACKAGES := kmod-mt76x0e
  DEVICE_TITLE := TP-Link ArcherC20i
  DEVICE_PACKAGES := kmod-mt76x0e kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += ArcherC20i

define Device/ArcherC50v1
  $(Device/Archer)
  DTS := ArcherC50
  SUPPORTED_DEVICES := c50
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0xc7500001
  TPLINK_HWREV := 69
  IMAGES += factory-us.bin factory-eu.bin
  IMAGE/factory-us.bin := tplink-v2-image -e -w 0
  IMAGE/factory-eu.bin := tplink-v2-image -e -w 2
  DEVICE_TITLE := TP-Link ArcherC50v1
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += ArcherC50v1

define Device/ArcherMR200
  $(Device/Archer)
  DTS := ArcherMR200
  SUPPORTED_DEVICES := mr200
  TPLINK_FLASHLAYOUT := 8MLmtk
  TPLINK_HWID := 0xd7500001
  TPLINK_HWREV := 0x4a
  DEVICE_PACKAGES := kmod-mt76x0e kmod-usb2 kmod-usb-net kmod-usb-net-rndis kmod-usb-serial kmod-usb-serial-option adb-enablemodem
  DEVICE_TITLE := TP-Link ArcherMR200
endef
TARGET_DEVICES += ArcherMR200

define Device/bdcom_wap2100-sk
  DTS := BDCOM-WAP2100-SK
  IMAGE_SIZE := 15808k
  DEVICE_TITLE := BDCOM WAP2100-SK (ZTE ZXECS EBG3130)
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-mt76x2 kmod-mt76x0e kmod-sdhci-mt7620 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += bdcom_wap2100-sk

define Device/bocco
  DTS := BOCCO
  DEVICE_TITLE := YUKAI Engineering BOCCO
  DEVICE_PACKAGES := kmod-sound-core kmod-sound-mt7620 kmod-i2c-ralink
endef
TARGET_DEVICES += bocco

define Device/c108
  DTS := C108
  IMAGE_SIZE := 16777216
  DEVICE_TITLE := HNET C108
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-sdhci-mt7620
endef
TARGET_DEVICES += c108

define Device/cf-wr800n
  DTS := CF-WR800N
  DEVICE_TITLE := Comfast CF-WR800N
endef
TARGET_DEVICES += cf-wr800n

define Device/cs-qr10
  DTS := CS-QR10
  DEVICE_TITLE := Planex CS-QR10
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci \
	kmod-sound-core kmod-sound-mt7620 \
	kmod-i2c-ralink kmod-sdhci-mt7620
endef
TARGET_DEVICES += cs-qr10

define Device/d240
  DTS := D240
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Sanlinking Technologies D240
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci kmod-sdhci-mt7620
endef
TARGET_DEVICES += d240

define Device/db-wrt01
  DTS := DB-WRT01
  DEVICE_TITLE := Planex DB-WRT01
endef
TARGET_DEVICES += db-wrt01

define Device/dch-m225
  $(Device/seama)
  DTS := DCH-M225
  BLOCKSIZE := 4k
  SEAMA_SIGNATURE := wapn22_dlink.2013gui_dap1320b
  IMAGE_SIZE := 6848k
  DEVICE_TITLE := D-Link DCH-M225
  DEVICE_PACKAGES := kmod-sound-core kmod-sound-mt7620 kmod-i2c-ralink
endef
TARGET_DEVICES += dch-m225

define Device/dir-810l
  DTS := DIR-810L
  DEVICE_PACKAGES := kmod-mt76x0e
  DEVICE_TITLE := D-Link DIR-810L
  IMAGE_SIZE := 6720k
endef
TARGET_DEVICES += dir-810l

define Device/dlink_dwr-116-a1
  DTS := DWR-116-A1
  DEVICE_TITLE := D-Link DWR-116 A1/A2
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci jboot-tools
  DLINK_ROM_ID := DLK6E3803001
  DLINK_FAMILY_MEMBER := 0x6E38
  DLINK_FIRMWARE_SIZE := 0x7E0000
  KERNEL := $(KERNEL_DTB)
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := mkdlinkfw | pad-rootfs | append-metadata
  IMAGE/factory.bin := mkdlinkfw | pad-rootfs | mkdlinkfw-factory
endef
TARGET_DEVICES += dlink_dwr-116-a1

define Device/dlink_dwr-118-a1
  DTS := DWR-118-A1
  DEVICE_TITLE := D-Link DWR-118 A1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci jboot-tools kmod-mt76x0e
  DLINK_ROM_ID := DLK6E3811001
  DLINK_FAMILY_MEMBER := 0x6E38
  DLINK_FIRMWARE_SIZE := 0xFE0000
  KERNEL := $(KERNEL_DTB)
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := mkdlinkfw | pad-rootfs | append-metadata
  IMAGE/factory.bin := mkdlinkfw | pad-rootfs | mkdlinkfw-factory
endef
TARGET_DEVICES += dlink_dwr-118-a1

define Device/dlink_dwr-118-a2
  DTS := DWR-118-A2
  DEVICE_TITLE := D-Link DWR-118 A2
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci jboot-tools
  DLINK_ROM_ID := DLK6E3814001
  DLINK_FAMILY_MEMBER := 0x6E38
  DLINK_FIRMWARE_SIZE := 0xFE0000
  KERNEL := $(KERNEL_DTB)
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := mkdlinkfw | pad-rootfs | append-metadata
  IMAGE/factory.bin := mkdlinkfw | pad-rootfs | mkdlinkfw-factory
endef
TARGET_DEVICES += dlink_dwr-118-a2

define Device/dlink_dwr-921-c1
  DTS := DWR-921-C1
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := D-Link DWR-921 C1
  DLINK_ROM_ID := DLK6E2414001
  DLINK_FAMILY_MEMBER := 0x6E24
  DLINK_FIRMWARE_SIZE := 0xFE0000
  KERNEL := $(KERNEL_DTB)
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := mkdlinkfw | pad-rootfs | append-metadata
  IMAGE/factory.bin := mkdlinkfw | pad-rootfs | mkdlinkfw-factory
  DEVICE_PACKAGES := jboot-tools \
	kmod-usb2 kmod-usb-net-qmi-wwan kmod-usb-serial-option uqmi
endef
TARGET_DEVICES += dlink_dwr-921-c1

define Device/dlink_dwr-921-c3
  $(Device/dlink_dwr-921-c1)
  DEVICE_TITLE := D-Link DWR-921 C3
  DLINK_ROM_ID := DLK6E2414009
  SUPPORTED_DEVICES := dlink,dwr-921-c1
endef
TARGET_DEVICES += dlink_dwr-921-c3

define Device/dlink_dwr-922-e2
  DTS := DWR-922-E2
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := D-Link DWR-922 E2
  DLINK_ROM_ID := DLK6E2414005
  DLINK_FAMILY_MEMBER := 0x6E24
  DLINK_FIRMWARE_SIZE := 0xFE0000
  KERNEL := $(KERNEL_DTB)
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := mkdlinkfw | pad-rootfs | append-metadata
  IMAGE/factory.bin := mkdlinkfw | pad-rootfs | mkdlinkfw-factory
  DEVICE_PACKAGES := jboot-tools \
	kmod-usb2 kmod-usb-net-qmi-wwan kmod-usb-serial-option uqmi
endef
TARGET_DEVICES += dlink_dwr-922-e2

define Device/e1700
  DTS := E1700
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	umedia-header 0x013326
  DEVICE_TITLE := Linksys E1700
endef
TARGET_DEVICES += e1700

define Device/ex2700
  NETGEAR_HW_ID := 29764623+4+0+32+2x2+0
  NETGEAR_BOARD_ID := EX2700
  DTS := EX2700
  BLOCKSIZE := 4k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  IMAGES += factory.bin
  KERNEL := $(KERNEL_DTB) | uImage lzma | pad-offset 64k 64 | append-uImage-fakehdr filesystem
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	netgear-dni
  DEVICE_TITLE := Netgear EX2700
endef
TARGET_DEVICES += ex2700

define Device/ex3700-ex3800
  NETGEAR_BOARD_ID := U12H319T00_NETGEAR
  DTS := EX3700
  BLOCKSIZE := 4k
  IMAGE_SIZE := 7744k
  IMAGES += factory.chk
  IMAGE/factory.chk := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | netgear-chk
  DEVICE_PACKAGES := kmod-mt76x2
  DEVICE_TITLE := Netgear EX3700/EX3800
  SUPPORTED_DEVICES := ex3700
endef
TARGET_DEVICES += ex3700-ex3800

define Device/gl-mt300a
  DTS := GL-MT300A
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := GL-Inet GL-MT300A
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += gl-mt300a

define Device/u25awf-h1
  DTS := U25AWF-H1
  IMAGE_SIZE := 16064k
  DEVICE_TITLE := Kimax U25AWF-H1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += u25awf-h1

define Device/gl-mt300n
  DTS := GL-MT300N
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := GL-Inet GL-MT300N
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += gl-mt300n

define Device/gl-mt750
  DTS := GL-MT750
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := GL-Inet GL-MT750
  DEVICE_PACKAGES := kmod-mt76x0e kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += gl-mt750

define Device/hc5661
  DTS := HC5661
  IMAGE_SIZE := 15872k
  DEVICE_TITLE := HiWiFi HC5661
  DEVICE_PACKAGES := kmod-sdhci-mt7620
endef
TARGET_DEVICES += hc5661

define Device/hc5761
  DTS := HC5761
  IMAGE_SIZE := 15872k
  DEVICE_TITLE := HiWiFi HC5761
  DEVICE_PACKAGES := kmod-mt76x0e kmod-usb2 kmod-usb-ohci kmod-sdhci-mt7620 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += hc5761

define Device/hc5861
  DTS := HC5861
  IMAGE_SIZE := 15872k
  DEVICE_TITLE := HiWiFi HC5861
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci kmod-sdhci-mt7620 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += hc5861

define Device/kimax_u35wf
  DTS := U35WF
  IMAGE_SIZE := 16064k
  DEVICE_TITLE := Kimax U35WF
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += kimax_u35wf

define Device/kng_rc
  DTS := kng_rc
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := ZyXEL Keenetic Viva
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport kmod-switch-rtl8366-smi kmod-switch-rtl8367b
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | pad-to 64k | check-size $$$$(IMAGE_SIZE) | \
	zyimage -d 8997 -v "ZyXEL Keenetic Viva"
  SUPPORTED_DEVICES := kng_rc
endef
TARGET_DEVICES += kng_rc

define Device/kn_rc
  DTS := kn_rc
  DEVICE_TITLE := ZyXEL Keenetic Omni
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(IMAGE/sysupgrade.bin) | pad-to 64k | check-size $$$$(IMAGE_SIZE) | \
	zyimage -d 4882 -v "ZyXEL Keenetic Omni"
  SUPPORTED_DEVICES := kn_rc
endef
TARGET_DEVICES += kn_rc

define Device/kn_rf
  DTS := kn_rf
  DEVICE_TITLE := ZyXEL Keenetic Omni II
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(IMAGE/sysupgrade.bin) | pad-to 64k | check-size $$$$(IMAGE_SIZE) | \
	zyimage -d 2102034 -v "ZyXEL Keenetic Omni II"
  SUPPORTED_DEVICES := kn_rf
endef
TARGET_DEVICES += kn_rf

define Device/microwrt
  DTS := MicroWRT
  IMAGE_SIZE := 16128k
  DEVICE_TITLE := Microduino MicroWRT
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += microwrt

define Device/lava_lr-25g001
  DTS := LR-25G001
  DEVICE_TITLE := LAVA LR-25G001
  DLINK_ROM_ID := LVA6E3804001
  DLINK_FAMILY_MEMBER := 0x6E38
  DLINK_FIRMWARE_SIZE := 0xFE0000
  KERNEL := $(KERNEL_DTB)
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := mkdlinkfw | pad-rootfs | append-metadata
  IMAGE/factory.bin := mkdlinkfw | pad-rootfs | mkdlinkfw-factory
  DEVICE_PACKAGES := jboot-tools kmod-usb2 kmod-usb-ohci kmod-mt76x0e
endef
TARGET_DEVICES += lava_lr-25g001

define Device/miwifi-mini
  DTS := MIWIFI-MINI
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Xiaomi MiWiFi Mini
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += miwifi-mini

define Device/mlw221
  DTS := MLW221
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Kingston MLW221
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += mlw221

define Device/mlwg2
  DTS := MLWG2
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Kingston MLWG2
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += mlwg2

define Device/mt7620a
  DTS := MT7620a
  DEVICE_TITLE := MediaTek MT7620a EVB
endef
TARGET_DEVICES += mt7620a

define Device/mt7620a_mt7530
  DTS := MT7620a_MT7530
  DEVICE_TITLE := MediaTek MT7620a + MT7530 EVB
  SUPPORTED_DEVICES := mt7620a_mt7530
endef
TARGET_DEVICES += mt7620a_mt7530

define Device/mt7620a_mt7610e
  DTS := MT7620a_MT7610e
  DEVICE_TITLE := MediaTek MT7620a + MT7610e EVB
  SUPPORTED_DEVICES := mt7620a_mt7610e
endef
TARGET_DEVICES += mt7620a_mt7610e

define Device/mt7620a_v22sg
  DTS := MT7620a_V22SG
  DEVICE_TITLE := MediaTek MT7620a V22SG
  SUPPORTED_DEVICES := mt7620a_v22sg
endef
TARGET_DEVICES += mt7620a_v22sg

define Device/mzk-750dhp
  DTS := MZK-750DHP
  DEVICE_TITLE := Planex MZK-750DHP
  DEVICE_PACKAGES := kmod-mt76x0e
endef
TARGET_DEVICES += mzk-750dhp

define Device/mzk-ex300np
  DTS := MZK-EX300NP
  DEVICE_TITLE := Planex MZK-EX300NP
endef
TARGET_DEVICES += mzk-ex300np

define Device/mzk-ex750np
  DTS := MZK-EX750NP
  DEVICE_TITLE := Planex MZK-EX750NP
  DEVICE_PACKAGES := kmod-mt76x2
endef
TARGET_DEVICES += mzk-ex750np

define Device/na930
  DTS := NA930
  IMAGE_SIZE := 20m
  DEVICE_TITLE := Sercomm NA930
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += na930

define Device/oy-0001
  DTS := OY-0001
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Oh Yeah OY-0001
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += oy-0001

define Device/psg1208
  DTS := PSG1208
  DEVICE_TITLE := Phicomm PSG1208
  DEVICE_PACKAGES := kmod-mt76x2
endef
TARGET_DEVICES += psg1208

define Device/psg1218a
  DTS := PSG1218A
  DEVICE_TITLE := Phicomm PSG1218 rev.Ax
  DEVICE_PACKAGES := kmod-mt76x2
  SUPPORTED_DEVICES += psg1218
endef
TARGET_DEVICES += psg1218a

define Device/psg1218b
  DTS := PSG1218B
  DEVICE_TITLE := Phicomm PSG1218 rev.Bx
  DEVICE_PACKAGES := kmod-mt76x2
  SUPPORTED_DEVICES += psg1218
endef
TARGET_DEVICES += psg1218b

define Device/phicomm_k2g
  DTS := K2G
  IMAGE_SIZE := 7552k
  DEVICE_TITLE := Phicomm K2G
  DEVICE_PACKAGES := kmod-mt76x2
endef
TARGET_DEVICES += phicomm_k2g

define Device/rp-n53
  DTS := RP-N53
  DEVICE_TITLE := Asus RP-N53
endef
TARGET_DEVICES += rp-n53

define Device/rt-n12p
  DTS := RT-N12-PLUS
  DEVICE_TITLE := Asus RT-N11P/RT-N12+/RT-N12Eb1
endef
TARGET_DEVICES += rt-n12p

define Device/rt-n14u
  DTS := RT-N14U
  DEVICE_TITLE := Asus RT-N14u
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += rt-n14u

define Device/rt-ac51u
  DTS := RT-AC51U
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Asus RT-AC51U
  DEVICE_PACKAGES := kmod-mt76x0e kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += rt-ac51u

define Device/tiny-ac
  DTS := TINY-AC
  DEVICE_TITLE := Dovado Tiny AC
  DEVICE_PACKAGES := kmod-mt76x0e kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += tiny-ac

define Device/edimax_br-6478ac-v2
  DTS := BR-6478AC-V2
  DEVICE_TITLE := Edimax BR-6478AC V2
  BLOCKSIZE := 64k
  IMAGE_SIZE := 7616k
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | \
	edimax-header -s CSYS -m RN68 -f 0x70000 -S 0x01100000 | pad-rootfs | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += edimax_br-6478ac-v2

define Device/tplink_c2-v1
  $(Device/Archer)
  DTS := ArcherC2-v1
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0xc7500001
  TPLINK_HWREV := 50
  IMAGES += factory.bin
  DEVICE_TITLE := TP-Link Archer C2 v1
  DEVICE_PACKAGES := kmod-mt76x0e kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport kmod-switch-rtl8366-smi kmod-switch-rtl8367b
endef
TARGET_DEVICES += tplink_c2-v1

define Device/tplink_c20-v1
  $(Device/Archer)
  DTS := ArcherC20v1
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0xc2000001
  TPLINK_HWREV := 0x44
  TPLINK_HWREVADD := 0x1
  IMAGES += factory.bin
  DEVICE_TITLE := TP-Link ArcherC20 v1
  DEVICE_PACKAGES := kmod-mt76x0e kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += tplink_c20-v1

define Device/vonets_var11n-300
  DTS := VAR11N-300
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  BLOCKSIZE := 4k
  DEVICE_TITLE := Vonets VAR11N-300
endef
TARGET_DEVICES += vonets_var11n-300

define Device/ravpower_wd03
  DTS := WD03
  IMAGE_SIZE := $(ralink_default_fw_size_8M)
  DEVICE_TITLE := Ravpower WD03
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += ravpower_wd03

define Device/whr-1166d
  DTS := WHR-1166D
  IMAGE_SIZE := 15040k
  DEVICE_TITLE := Buffalo WHR-1166D
  DEVICE_PACKAGES := kmod-mt76x2
endef
TARGET_DEVICES += whr-1166d

define Device/whr-300hp2
  DTS := WHR-300HP2
  IMAGE_SIZE := 6848k
  DEVICE_TITLE := Buffalo WHR-300HP2
endef
TARGET_DEVICES += whr-300hp2

define Device/whr-600d
  DTS := WHR-600D
  IMAGE_SIZE := 6848k
  DEVICE_TITLE := Buffalo WHR-600D
endef
TARGET_DEVICES += whr-600d

define Device/wmr-300
  DTS := WMR-300
  DEVICE_TITLE := Buffalo WMR-300
endef
TARGET_DEVICES += wmr-300

define Device/wn3000rpv3
  NETGEAR_HW_ID := 29764836+8+0+32+2x2+0
  NETGEAR_BOARD_ID := WN3000RPv3
  DTS := WN3000RPV3
  BLOCKSIZE := 4k
  IMAGES += factory.bin
  KERNEL := $(KERNEL_DTB) | uImage lzma | pad-offset 64k 64 | append-uImage-fakehdr filesystem
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	netgear-dni
  DEVICE_TITLE := Netgear WN3000RPv3
endef
TARGET_DEVICES += wn3000rpv3

define Device/wrh-300cr
  DTS := WRH-300CR
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	elecom-header
  DEVICE_TITLE := Elecom WRH-300CR
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += wrh-300cr

define Device/wrtnode
  DTS := WRTNODE
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := WRTNode
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += wrtnode

define Device/wt3020-4M
  DTS := WT3020-4M
  BLOCKSIZE := 4k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  IMAGES += factory.bin
  SUPPORTED_DEVICES += wt3020
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	poray-header -B WT3020 -F 4M
  DEVICE_TITLE := Nexx WT3020 (4MB)
endef
TARGET_DEVICES += wt3020-4M

define Device/wt3020-8M
  DTS := WT3020-8M
  IMAGES += factory.bin
  SUPPORTED_DEVICES += wt3020
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	poray-header -B WT3020 -F 8M
  DEVICE_TITLE := Nexx WT3020 (8MB)
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += wt3020-8M

define Device/y1
  DTS := Y1
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Lenovo Y1
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += y1

define Device/y1s
  DTS := Y1S
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Lenovo Y1S
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += y1s

define Device/youku-yk1
  DTS := YOUKU-YK1
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
  DEVICE_TITLE := YOUKU YK1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-sdhci-mt7620 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += youku-yk1

define Device/we1026-5g-16m
  DTS := WE1026-5G-16M
  IMAGE_SIZE := 16777216
  SUPPORTED_DEVICES += we1026-5g-16m
  DEVICE_TITLE := Zbtlink ZBT-WE1026-5G (16M)
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci kmod-sdhci-mt7620
endef
TARGET_DEVICES += we1026-5g-16m

define Device/xiaomi_miwifi-r3
  DTS := MIWIFI-R3
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 32768k
  UBINIZE_OPTS := -E 5
  IMAGES += kernel1.bin rootfs0.bin
  IMAGE/kernel1.bin := append-kernel | check-size $$$$(KERNEL_SIZE)
  IMAGE/rootfs0.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  DEVICE_TITLE := Xiaomi Mi Router R3
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci uboot-envtools
endef
TARGET_DEVICES += xiaomi_miwifi-r3

define Device/zbt-ape522ii
  DTS := ZBT-APE522II
  DEVICE_TITLE := Zbtlink ZBT-APE522II
  DEVICE_PACKAGES := kmod-mt76x2
endef
TARGET_DEVICES += zbt-ape522ii

define Device/zbt-cpe102
  DTS := ZBT-CPE102
  DEVICE_TITLE := Zbtlink ZBT-CPE102
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += zbt-cpe102

define Device/zbt-wa05
  DTS := ZBT-WA05
  DEVICE_TITLE := Zbtlink ZBT-WA05
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += zbt-wa05

define Device/zbt-we2026
  DTS := ZBT-WE2026
  DEVICE_TITLE := Zbtlink ZBT-WE2026
endef
TARGET_DEVICES += zbt-we2026

define Device/zbt-we826-16M
  DTS := ZBT-WE826-16M
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  SUPPORTED_DEVICES += zbt-we826
  DEVICE_TITLE := Zbtlink ZBT-WE826 (16M)
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci kmod-sdhci-mt7620
endef
TARGET_DEVICES += zbt-we826-16M

define Device/zbt-we826-32M
  DTS := ZBT-WE826-32M
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
  DEVICE_TITLE := Zbtlink ZBT-WE826 (32M)
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci kmod-sdhci-mt7620
endef
TARGET_DEVICES += zbt-we826-32M

define Device/zbt-wr8305rt
  DTS := ZBT-WR8305RT
  DEVICE_TITLE := Zbtlink ZBT-WR8305RT
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += zbt-wr8305rt

define Device/zte-q7
  DTS := ZTE-Q7
  DEVICE_TITLE := ZTE Q7
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += zte-q7
