#
# MT7621 Profiles
#

DEVICE_VARS += TPLINK_BOARD_ID TPLINK_HEADER_VERSION TPLINK_HWID TPLINK_HWREV

define Build/ubnt-erx-factory-image
	if [ -e $(KDIR)/tmp/$(KERNEL_INITRAMFS_IMAGE) -a "$$(stat -c%s $@)" -lt "$(KERNEL_SIZE)" ]; then \
		echo '21001:6' > $(1).compat; \
		$(TAR) -cf $(1) --transform='s/^.*/compat/' $(1).compat; \
		\
		$(TAR) -rf $(1) --transform='s/^.*/vmlinux.tmp/' $(KDIR)/tmp/$(KERNEL_INITRAMFS_IMAGE); \
		mkhash md5 $(KDIR)/tmp/$(KERNEL_INITRAMFS_IMAGE) > $(1).md5; \
		$(TAR) -rf $(1) --transform='s/^.*/vmlinux.tmp.md5/' $(1).md5; \
		\
		echo "dummy" > $(1).rootfs; \
		$(TAR) -rf $(1) --transform='s/^.*/squashfs.tmp/' $(1).rootfs; \
		\
		mkhash md5 $(1).rootfs > $(1).md5; \
		$(TAR) -rf $(1) --transform='s/^.*/squashfs.tmp.md5/' $(1).md5; \
		\
		echo '$(BOARD) $(VERSION_CODE) $(VERSION_NUMBER)' > $(1).version; \
		$(TAR) -rf $(1) --transform='s/^.*/version.tmp/' $(1).version; \
		\
		$(CP) $(1) $(BIN_DIR)/; \
	else \
		echo "WARNING: initramfs kernel image too big, cannot generate factory image" >&2; \
	fi
endef

define Device/11acnas
  DTS := 11ACNAS
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := WeVO 11AC NAS Router
  DEVICE_PACKAGES := kmod-mt7603 kmod-usb3 kmod-usb-ledtrig-usbport wpad-mini
endef
TARGET_DEVICES += 11acnas

define Device/dir-860l-b1
  DTS := DIR-860L-B1
  BLOCKSIZE := 64k
  IMAGES += factory.bin
  KERNEL := kernel-bin | patch-dtb | relocate-kernel | lzma | uImage lzma
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  IMAGE/sysupgrade.bin := \
	append-kernel | pad-offset $$$$(BLOCKSIZE) 64 | append-rootfs | \
	seama -m "dev=/dev/mtdblock/2" -m "type=firmware" | \
	pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := \
	append-kernel | pad-offset $$$$(BLOCKSIZE) 64 | \
	append-rootfs | pad-rootfs -x 64 | \
	seama -m "dev=/dev/mtdblock/2" -m "type=firmware" | \
	seama-seal -m "signature=wrgac13_dlink.2013gui_dir860lb" | \
	check-size $$$$(IMAGE_SIZE)
  DEVICE_TITLE := D-Link DIR-860L B1
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-mini
endef
TARGET_DEVICES += dir-860l-b1

define Device/mediatek_ap-mt7621a-v60
  DTS := AP-MT7621A-V60
  IMAGE_SIZE := $(ralink_default_fw_size_8M)
  DEVICE_TITLE := Mediatek AP-MT7621A-V60 EVB
  DEVICE_PACKAGES := kmod-usb3 kmod-sdhci-mt7620 kmod-sound-mt7620
endef
TARGET_DEVICES += mediatek_ap-mt7621a-v60

define Device/ew1200
  DTS := EW1200
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := AFOUNDRY EW1200
  DEVICE_PACKAGES := \
	kmod-ata-core kmod-ata-ahci kmod-mt76x2 kmod-mt7603 kmod-usb3 \
	kmod-usb-ledtrig-usbport wpad-mini
endef
TARGET_DEVICES += ew1200

define Device/firewrt
  DTS := FIREWRT
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Firefly FireWRT
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-mini
endef
TARGET_DEVICES += firewrt

define Device/gb-pc1
  DTS := GB-PC1
  DEVICE_TITLE := GnuBee Personal Cloud One
  DEVICE_PACKAGES := kmod-ata-core kmod-ata-ahci kmod-usb3 kmod-sdhci-mt7620
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
endef
TARGET_DEVICES += gb-pc1

define Device/gnubee_gb-pc2
  DTS := GB-PC2
  DEVICE_TITLE := GnuBee Personal Cloud Two
  DEVICE_PACKAGES := kmod-ata-core kmod-ata-ahci kmod-usb3 kmod-sdhci-mt7620
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
endef
TARGET_DEVICES += gnubee_gb-pc2

define Device/hc5962
  DTS := HC5962
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 2097152
  UBINIZE_OPTS := -E 5
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/factory.bin := append-kernel | pad-to $$(KERNEL_SIZE) | append-ubi | check-size $$$$(IMAGE_SIZE)
  DEVICE_TITLE := HiWiFi HC5962
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 wpad-mini
endef
TARGET_DEVICES += hc5962

define Device/k2p
  DTS := K2P
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Phicomm K2P
endef
TARGET_DEVICES += k2p

define Device/mir3g
  DTS := MIR3G
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  KERNEL := $(KERNEL_DTB) | uImage lzma
  IMAGE_SIZE := 32768k
  UBINIZE_OPTS := -E 5
  IMAGES := sysupgrade.tar kernel1.bin rootfs0.bin
  IMAGE/kernel1.bin := append-kernel
  IMAGE/rootfs0.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.tar := sysupgrade-tar | append-metadata
  DEVICE_TITLE := Xiaomi Mi Router 3G
  SUPPORTED_DEVICES += R3G
  DEVICE_PACKAGES := \
	kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-mini \
	uboot-envtools
endef
TARGET_DEVICES += mir3g

define Device/mt7621
  DTS := MT7621
  BLOCKSIZE := 64k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := MediaTek MT7621 EVB
endef
TARGET_DEVICES += mt7621

define Device/newifi-d1
  DTS := Newifi-D1
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
  DEVICE_TITLE := Newifi D1
  DEVICE_PACKAGES := \
	kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-mini
endef
TARGET_DEVICES += newifi-d1

define Device/d-team_newifi-d2
  DTS := Newifi-D2
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
  DEVICE_TITLE := Newifi D2
  DEVICE_PACKAGES := \
	kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += d-team_newifi-d2

define Device/pbr-m1
  DTS := PBR-M1
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := PBR-M1
  DEVICE_PACKAGES := \
	kmod-ata-core kmod-ata-ahci kmod-mt7603 kmod-mt76x2 kmod-sdhci-mt7620 \
	kmod-usb3 kmod-usb-ledtrig-usbport wpad-mini
endef
TARGET_DEVICES += pbr-m1

define Device/r6220
  DTS := R6220
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  KERNEL := $(KERNEL_DTB) | uImage lzma
  IMAGE_SIZE := 28672k
  UBINIZE_OPTS := -E 5
  IMAGES := sysupgrade.tar kernel.bin rootfs.bin
  IMAGE/sysupgrade.tar := sysupgrade-tar | append-metadata
  IMAGE/kernel.bin := append-kernel
  IMAGE/rootfs.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  DEVICE_TITLE := Netgear R6220
  DEVICE_PACKAGES := \
	kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-mini
endef
TARGET_DEVICES += r6220

define Device/rb750gr3
  DTS := RB750Gr3
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := MikroTik RB750Gr3
  DEVICE_PACKAGES := kmod-usb3 uboot-envtools
endef
TARGET_DEVICES += rb750gr3

define Device/re350-v1
  DTS := RE350
  DEVICE_TITLE := TP-LINK RE350 v1
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 wpad-mini
  TPLINK_BOARD_ID := RE350-V1
  TPLINK_HWID := 0x0
  TPLINK_HWREV := 0
  TPLINK_HEADER_VERSION := 1
  IMAGE_SIZE := 6016k
  KERNEL := $(KERNEL_DTB) | tplink-v1-header -e
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade | append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
endef
TARGET_DEVICES += re350-v1

define Device/re6500
  DTS := RE6500
  DEVICE_TITLE := Linksys RE6500
  DEVICE_PACKAGES := kmod-mt76x2 wpad-mini
endef
TARGET_DEVICES += re6500

define Device/sap-g3200u3
  DTS := SAP-G3200U3
  DEVICE_TITLE := STORYLiNK SAP-G3200U3
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-mini
endef
TARGET_DEVICES += sap-g3200u3

define Device/sk-wb8
  DTS := SK-WB8
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := SamKnows Whitebox 8
  DEVICE_PACKAGES := \
	kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport \
	uboot-envtools wpad-mini
endef
TARGET_DEVICES += sk-wb8

define Device/timecloud
  DTS := Timecloud
  DEVICE_TITLE := Thunder Timecloud
  DEVICE_PACKAGES := kmod-usb3
endef
TARGET_DEVICES += timecloud

define Device/u7621-06-256M-16M
  DTS := U7621-06-256M-16M
  IMAGE_SIZE := 16064k
  DEVICE_TITLE := UniElec U7621-06 (256M RAM/16M flash)
  DEVICE_PACKAGES := kmod-ata-core kmod-ata-ahci kmod-sdhci-mt7620 kmod-usb3
endef
TARGET_DEVICES += u7621-06-256M-16M

define Device/ubnt-erx
  DTS := UBNT-ERX
  FILESYSTEMS := squashfs
  KERNEL_SIZE := 3145728
  KERNEL := $(KERNEL_DTB) | uImage lzma
  IMAGES := sysupgrade.tar
  KERNEL_INITRAMFS := $$(KERNEL) | ubnt-erx-factory-image $(KDIR)/tmp/$$(KERNEL_INITRAMFS_PREFIX)-factory.tar
  IMAGE/sysupgrade.tar := sysupgrade-tar | append-metadata
  DEVICE_TITLE := Ubiquiti EdgeRouter X
endef
TARGET_DEVICES += ubnt-erx

define Device/ubnt-erx-sfp
  $(Device/ubnt-erx)
  DTS := UBNT-ERX-SFP
  DEVICE_TITLE := Ubiquiti EdgeRouter X-SFP
  DEVICE_PACKAGES += kmod-i2c-algo-pca kmod-gpio-pca953x kmod-i2c-gpio-custom
endef
TARGET_DEVICES += ubnt-erx-sfp

define Device/vr500
  DTS := VR500
  IMAGE_SIZE := 66453504
  DEVICE_TITLE := Planex VR500
  DEVICE_PACKAGES := kmod-usb3
endef
TARGET_DEVICES += vr500

define Device/w2914nsv2
  DTS := W2914NSV2
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := WeVO W2914NS v2
  DEVICE_PACKAGES := \
	kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport kmod-mt76 \
	wpad-mini
endef
TARGET_DEVICES += w2914nsv2

define Device/wf-2881
  DTS := WF-2881
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  FILESYSTEMS := squashfs
  IMAGE_SIZE := 129280k
  KERNEL := $(KERNEL_DTB) | pad-offset $$(BLOCKSIZE) 64 | uImage lzma
  UBINIZE_OPTS := -E 5
  IMAGE/sysupgrade.bin := append-kernel | append-ubi | append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_TITLE := NETIS WF-2881
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += wf-2881

define Device/witi
  DTS := WITI
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := MQmaker WiTi
  DEVICE_PACKAGES := \
	kmod-ata-core kmod-ata-ahci kmod-mt76x2 kmod-sdhci-mt7620 kmod-usb3 \
	kmod-usb-ledtrig-usbport wpad-mini
endef
TARGET_DEVICES += witi

define Device/wndr3700v5
  DTS := WNDR3700V5
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Netgear WNDR3700v5
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 wpad-mini
endef
TARGET_DEVICES += wndr3700v5

define Device/youhua_wr1200js
  DTS := WR1200JS
  IMAGE_SIZE := 16064k
  DEVICE_TITLE := YouHua WR1200JS
  DEVICE_PACKAGES := \
	kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += youhua_wr1200js

define Device/wsr-1166
  DTS := WSR-1166
  IMAGE/sysupgrade.bin := trx | pad-rootfs | append-metadata
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Buffalo WSR-1166
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 wpad-mini
endef
TARGET_DEVICES += wsr-1166

define Device/wsr-600
  DTS := WSR-600
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Buffalo WSR-600
  DEVICE_PACKAGES := kmod-mt7603 kmod-rt2800-pci wpad-mini
endef
TARGET_DEVICES += wsr-600

define Device/zbt-we1326
  DTS := ZBT-WE1326
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := ZBT WE1326
  DEVICE_PACKAGES := \
	kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-sdhci-mt7620 wpad-mini
endef
TARGET_DEVICES += zbt-we1326

define Device/zbtlink_zbt-we3526
  DTS := ZBT-WE3526
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := ZBT WE3526
  DEVICE_PACKAGES := \
	kmod-sdhci-mt7620 kmod-mt7603 kmod-mt76x2 \
	kmod-usb3 kmod-usb-ledtrig-usbport wpad-mini
endef
TARGET_DEVICES += zbtlink_zbt-we3526

define Device/zbt-wg2626
  DTS := ZBT-WG2626
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := ZBT WG2626
  DEVICE_PACKAGES := \
	kmod-ata-core kmod-ata-ahci kmod-sdhci-mt7620 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport wpad-mini
endef
TARGET_DEVICES += zbt-wg2626

define Device/zbt-wg3526-16M
  DTS := ZBT-WG3526-16M
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  SUPPORTED_DEVICES += zbt-wg3526
  DEVICE_TITLE := ZBT WG3526 (16MB flash)
  DEVICE_PACKAGES := \
	kmod-ata-core kmod-ata-ahci kmod-sdhci-mt7620 kmod-mt7603 kmod-mt76x2 \
	kmod-usb3 kmod-usb-ledtrig-usbport wpad-mini
endef
TARGET_DEVICES += zbt-wg3526-16M

define Device/zbt-wg3526-32M
  DTS := ZBT-WG3526-32M
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
  SUPPORTED_DEVICES += ac1200pro
  DEVICE_TITLE := ZBT WG3526 (32MB flash)
  DEVICE_PACKAGES := \
	kmod-ata-core kmod-ata-ahci kmod-sdhci-mt7620 kmod-mt7603 kmod-mt76x2 \
	kmod-usb3 kmod-usb-ledtrig-usbport wpad-mini
endef
TARGET_DEVICES += zbt-wg3526-32M

# FIXME: is this still needed?
define Image/Prepare
#define Build/Compile
	rm -rf $(KDIR)/relocate
	$(CP) ../../generic/image/relocate $(KDIR)
	$(MAKE) -C $(KDIR)/relocate KERNEL_ADDR=$(KERNEL_LOADADDR) CROSS_COMPILE=$(TARGET_CROSS)
	$(CP) $(KDIR)/relocate/loader.bin $(KDIR)/loader.bin
endef
