#
# MT7621 Profiles
#

KERNEL_DTB += -d21
DEVICE_VARS += TPLINK_BOARD_ID TPLINK_HEADER_VERSION TPLINK_HWID TPLINK_HWREV

define Build/elecom-gst-factory
  $(eval product=$(word 1,$(1)))
  $(eval version=$(word 2,$(1)))
  ( $(STAGING_DIR_HOST)/bin/mkhash md5 $@ | tr -d '\n' ) >> $@
  ( \
    echo -n "ELECOM $(product) v$(version)" | \
      dd bs=32 count=1 conv=sync; \
    dd if=$@; \
  ) > $@.new
  mv $@.new $@
  echo -n "MT7621_ELECOM_$(product)" >> $@
endef

define Build/elecom-wrc-factory
  $(eval product=$(word 1,$(1)))
  $(eval version=$(word 2,$(1)))
  $(STAGING_DIR_HOST)/bin/mkhash md5 $@ >> $@
  ( \
    echo -n "ELECOM $(product) v$(version)" | \
      dd bs=32 count=1 conv=sync; \
    dd if=$@; \
  ) > $@.new
  mv $@.new $@
endef

define Build/iodata-factory
  $(eval fw_size=$(word 1,$(1)))
  $(eval fw_type=$(word 2,$(1)))
  $(eval product=$(word 3,$(1)))
  $(eval factory_bin=$(word 4,$(1)))
  if [ -e $(KDIR)/tmp/$(KERNEL_INITRAMFS_IMAGE) -a "$$(stat -c%s $@)" -lt "$(fw_size)" ]; then \
    $(CP) $(KDIR)/tmp/$(KERNEL_INITRAMFS_IMAGE) $(factory_bin); \
    $(STAGING_DIR_HOST)/bin/mksenaofw \
      -r 0x30a -p $(product) -t $(fw_type) \
      -e $(factory_bin) -o $(factory_bin).new; \
    mv $(factory_bin).new $(factory_bin); \
    $(CP) $(factory_bin) $(BIN_DIR)/; \
	else \
		echo "WARNING: initramfs kernel image too big, cannot generate factory image" >&2; \
	fi
endef

# The OEM webinterface expects an kernel with initramfs which has the uImage
# header field ih_name.
# We don't wan't to set the header name field for the kernel include in the
# sysupgrade image as well, as this image shouldn't be accepted by the OEM
# webinterface. It will soft-brick the board.
define Build/wr1201-factory-header
	mkimage -A $(LINUX_KARCH) \
		-O linux -T kernel \
		-C lzma -a $(KERNEL_LOADADDR) -e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-n 'WR1201_8_128' -d $@ $@.new
	mv $@.new $@
endef

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
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += 11acnas

define Device/dir-860l-b1
  $(Device/seama)
  DTS := DIR-860L-B1
  BLOCKSIZE := 64k
  SEAMA_SIGNATURE := wrgac13_dlink.2013gui_dir860lb
  KERNEL := kernel-bin | patch-dtb | relocate-kernel | lzma | uImage lzma
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := D-Link DIR-860L B1
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += dir-860l-b1

define Device/mediatek_ap-mt7621a-v60
  DTS := AP-MT7621A-V60
  IMAGE_SIZE := $(ralink_default_fw_size_8M)
  DEVICE_TITLE := Mediatek AP-MT7621A-V60 EVB
  DEVICE_PACKAGES := kmod-usb3 kmod-sdhci-mt7620 kmod-sound-mt7620
endef
TARGET_DEVICES += mediatek_ap-mt7621a-v60

define Device/xzwifi_creativebox-v1
  DTS := CreativeBox-v1
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
  DEVICE_TITLE := CreativeBox v1
  DEVICE_PACKAGES := \
	kmod-ata-core kmod-ata-ahci kmod-mt7603 kmod-mt76x2 kmod-sdhci-mt7620 \
	kmod-usb3
endef
TARGET_DEVICES += xzwifi_creativebox-v1

define Device/elecom_wrc-1167ghbk2-s
  DTS := WRC-1167GHBK2-S
  IMAGE_SIZE := 15488k
  DEVICE_TITLE := ELECOM WRC-1167GHBK2-S
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) |\
    elecom-wrc-factory WRC-1167GHBK2-S 0.00
endef
TARGET_DEVICES += elecom_wrc-1167ghbk2-s

define Device/elecom_wrc-2533gst
  DTS := WRC-2533GST
  IMAGE_SIZE := 11264k
  DEVICE_TITLE := ELECOM WRC-2533GST
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) |\
    elecom-gst-factory WRC-2533GST 0.00
endef
TARGET_DEVICES += elecom_wrc-2533gst

define Device/elecom_wrc-1900gst
  DTS := WRC-1900GST
  IMAGE_SIZE := 11264k
  DEVICE_TITLE := ELECOM WRC-1900GST
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) |\
    elecom-gst-factory WRC-1900GST 0.00
endef
TARGET_DEVICES += elecom_wrc-1900gst

define Device/ew1200
  DTS := EW1200
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := AFOUNDRY EW1200
  DEVICE_PACKAGES := \
	kmod-ata-core kmod-ata-ahci kmod-mt76x2 kmod-mt7603 kmod-usb3 \
	kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += ew1200

define Device/firewrt
  DTS := FIREWRT
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Firefly FireWRT
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += firewrt

define Device/gehua_ghl-r-001
  DTS := GHL-R-001
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
  DEVICE_TITLE := GeHua GHL-R-001
  DEVICE_PACKAGES := \
	kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += gehua_ghl-r-001

define Device/gnubee_gb-pc1
  DTS := GB-PC1
  DEVICE_TITLE := GnuBee Personal Cloud One
  DEVICE_PACKAGES := kmod-ata-core kmod-ata-ahci kmod-usb3 kmod-sdhci-mt7620
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
endef
TARGET_DEVICES += gnubee_gb-pc1

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
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 wpad-basic
endef
TARGET_DEVICES += hc5962

define Device/iodata_wn-ax1167gr
  DTS := WN-AX1167GR
  IMAGE_SIZE := 15552k
  KERNEL_INITRAMFS := $$(KERNEL) | \
    iodata-factory 7864320 4 0x1055 $(KDIR)/tmp/$$(KERNEL_INITRAMFS_PREFIX)-factory.bin
  DEVICE_TITLE := I-O DATA WN-AX1167GR
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 wpad-basic
endef
TARGET_DEVICES += iodata_wn-ax1167gr

define Device/iodata_wn-gx300gr
  DTS := WN-GX300GR
  IMAGE_SIZE := 7798784
  DEVICE_TITLE := I-O DATA WN-GX300GR
  DEVICE_PACKAGES := kmod-mt7603 wpad-basic
endef
TARGET_DEVICES += iodata_wn-gx300gr

define Device/k2p
  DTS := K2P
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Phicomm K2P
endef
TARGET_DEVICES += k2p

define Device/xiaomi_mir3p
  DTS := MIR3P
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE:= 4096k
  UBINIZE_OPTS := -E 5
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
  DEVICE_TITLE := Xiaomi Mi Router 3 Pro
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/factory.bin := append-kernel | pad-to $$(KERNEL_SIZE) | append-ubi | check-size $$$$(IMAGE_SIZE)
  DEVICE_PACKAGES := \
	kmod-usb3 kmod-usb-ledtrig-usbport wpad-basic uboot-envtools
endef
TARGET_DEVICES += xiaomi_mir3p

define Device/xiaomi_mir3g
  DTS := MIR3G
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 32768k
  UBINIZE_OPTS := -E 5
  IMAGES += kernel1.bin rootfs0.bin
  IMAGE/kernel1.bin := append-kernel
  IMAGE/rootfs0.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  DEVICE_TITLE := Xiaomi Mi Router 3G
  SUPPORTED_DEVICES += R3G
  SUPPORTED_DEVICES += mir3g
  DEVICE_PACKAGES := \
	kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-basic \
	uboot-envtools
endef
TARGET_DEVICES += xiaomi_mir3g

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
	kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += newifi-d1

define Device/d-team_newifi-d2
  DTS := Newifi-D2
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
  DEVICE_TITLE := Newifi D2
  DEVICE_PACKAGES := \
	kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += d-team_newifi-d2

define Device/pbr-m1
  DTS := PBR-M1
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := PBR-M1
  DEVICE_PACKAGES := \
	kmod-ata-core kmod-ata-ahci kmod-mt7603 kmod-mt76x2 kmod-sdhci-mt7620 \
	kmod-usb3 kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += pbr-m1

define Device/r6220
  DTS := R6220
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 28672k
  UBINIZE_OPTS := -E 5
  IMAGES += kernel.bin rootfs.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/kernel.bin := append-kernel
  IMAGE/rootfs.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  DEVICE_TITLE := Netgear R6220
  DEVICE_PACKAGES := \
	kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += r6220

define Device/netgear_ex6150
  DTS := EX6150
  DEVICE_TITLE := Netgear EX6150
  DEVICE_PACKAGES := kmod-mt76x2 wpad-basic
  NETGEAR_BOARD_ID := U12H318T00_NETGEAR
  IMAGE_SIZE := 14848k
  IMAGES += factory.chk
  IMAGE/factory.chk := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | netgear-chk
endef
TARGET_DEVICES += netgear_ex6150

define Device/netgear_r6350
  DTS := R6350
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 40960k
  UBINIZE_OPTS := -E 5
  IMAGES += kernel.bin rootfs.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/kernel.bin := append-kernel
  IMAGE/rootfs.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  DEVICE_TITLE := Netgear R6350
  DEVICE_PACKAGES := \
	kmod-mt7603 kmod-usb3 kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += netgear_r6350

define Device/MikroTik
  BLOCKSIZE := 64k
  IMAGE_SIZE := 16128k
  DEVICE_PACKAGES := kmod-usb3
  LOADER_TYPE := elf
  PLATFORM := mt7621
  KERNEL := $(KERNEL_DTB) | loader-kernel
  IMAGE/sysupgrade.bin := append-kernel | kernel2minor -s 1024 | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
endef

define Device/mikrotik_rb750gr3
  $(Device/MikroTik)
  DTS := RB750Gr3
  DEVICE_TITLE := MikroTik RouterBOARD RB750Gr3
  DEVICE_PACKAGES += kmod-gpio-beeper
endef
TARGET_DEVICES += mikrotik_rb750gr3

define Device/mikrotik_rbm33g
  $(Device/MikroTik)
  DTS := RBM33G
  DEVICE_TITLE := MikroTik RouterBOARD M33G
endef
TARGET_DEVICES += mikrotik_rbm33g

define Device/mikrotik_rbm11g
  $(Device/MikroTik)
  DTS := RBM11G
  DEVICE_TITLE := MikroTik RouterBOARD M11G
endef
TARGET_DEVICES += mikrotik_rbm11g

define Device/mtc_wr1201
	DTS := WR1201
	IMAGE_SIZE := 16000k
	DEVICE_TITLE := MTC Wireless Router WR1201
	KERNEL_INITRAMFS := $(KERNEL_DTB) | wr1201-factory-header
	DEVICE_PACKAGES := kmod-sdhci-mt7620 kmod-mt76x2 kmod-usb3 \
		kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += mtc_wr1201

define Device/re350-v1
  DTS := RE350
  DEVICE_TITLE := TP-LINK RE350 v1
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 wpad-basic
  TPLINK_BOARD_ID := RE350-V1
  TPLINK_HWID := 0x0
  TPLINK_HWREV := 0
  TPLINK_HEADER_VERSION := 1
  IMAGE_SIZE := 6016k
  KERNEL := $(KERNEL_DTB) | tplink-v1-header -e -O
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade | append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
endef
TARGET_DEVICES += re350-v1

define Device/re6500
  DTS := RE6500
  DEVICE_TITLE := Linksys RE6500
  DEVICE_PACKAGES := kmod-mt76x2 wpad-basic
endef
TARGET_DEVICES += re6500

define Device/sap-g3200u3
  DTS := SAP-G3200U3
  DEVICE_TITLE := STORYLiNK SAP-G3200U3
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += sap-g3200u3

define Device/sk-wb8
  DTS := SK-WB8
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := SamKnows Whitebox 8
  DEVICE_PACKAGES := \
	kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport \
	uboot-envtools wpad-basic
endef
TARGET_DEVICES += sk-wb8

define Device/timecloud
  DTS := Timecloud
  DEVICE_TITLE := Thunder Timecloud
  DEVICE_PACKAGES := kmod-usb3
endef
TARGET_DEVICES += timecloud

define Device/ubnt-erx
  DTS := UBNT-ERX
  FILESYSTEMS := squashfs
  KERNEL_SIZE := 3145728
  KERNEL_INITRAMFS := $$(KERNEL) | ubnt-erx-factory-image $(KDIR)/tmp/$$(KERNEL_INITRAMFS_PREFIX)-factory.tar
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
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

define Device/unielec_u7621-06-256m-16m
  DTS := U7621-06-256M-16M
  IMAGE_SIZE := 16064k
  DEVICE_TITLE := UniElec U7621-06 (256M RAM/16M flash)
  DEVICE_PACKAGES := kmod-ata-core kmod-ata-ahci kmod-sdhci-mt7620 kmod-usb3
  SUPPORTED_DEVICES += u7621-06-256M-16M
endef
TARGET_DEVICES += unielec_u7621-06-256m-16m

define Device/unielec_u7621-06-512m-64m
  DTS := U7621-06-512M-64M
  IMAGE_SIZE := 65216k
  DEVICE_TITLE := UniElec U7621-06 (512M RAM/64M flash)
  DEVICE_PACKAGES := kmod-ata-core kmod-ata-ahci kmod-sdhci-mt7620 kmod-usb3
endef
TARGET_DEVICES += unielec_u7621-06-512m-64m

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
	kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-basic
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
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += wf-2881

define Device/mqmaker_witi-256m
  DTS := WITI-256M
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := MQmaker WiTi (256MB RAM)
  DEVICE_PACKAGES := \
	kmod-ata-core kmod-ata-ahci kmod-mt76x2 kmod-sdhci-mt7620 kmod-usb3 \
	kmod-usb-ledtrig-usbport wpad-basic
  SUPPORTED_DEVICES += witi
endef
TARGET_DEVICES += mqmaker_witi-256m

define Device/mqmaker_witi-512m
  DTS := WITI-512M
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := MQmaker WiTi (512MB RAM)
  DEVICE_PACKAGES := \
	kmod-ata-core kmod-ata-ahci kmod-mt76x2 kmod-sdhci-mt7620 kmod-usb3 \
	kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += mqmaker_witi-512m

define Device/wndr3700v5
  DTS := WNDR3700V5
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Netgear WNDR3700v5
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 wpad-basic
endef
TARGET_DEVICES += wndr3700v5

define Device/youhua_wr1200js
  DTS := WR1200JS
  IMAGE_SIZE := 16064k
  DEVICE_TITLE := YouHua WR1200JS
  DEVICE_PACKAGES := \
	kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += youhua_wr1200js

define Device/youku_yk-l2
  DTS := YOUKU-YK2
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Youku YK-L2
  DEVICE_PACKAGES := \
	kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += youku_yk-l2

define Device/wsr-1166
  DTS := WSR-1166
  IMAGE/sysupgrade.bin := trx | pad-rootfs | append-metadata
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Buffalo WSR-1166
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 wpad-basic
endef
TARGET_DEVICES += wsr-1166

define Device/wsr-600
  DTS := WSR-600
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Buffalo WSR-600
  DEVICE_PACKAGES := kmod-mt7603 kmod-rt2800-pci wpad-basic
endef
TARGET_DEVICES += wsr-600

define Device/zbt-we1326
  DTS := ZBT-WE1326
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := ZBT WE1326
  DEVICE_PACKAGES := \
	kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-sdhci-mt7620 wpad-basic
endef
TARGET_DEVICES += zbt-we1326

define Device/zbtlink_zbt-we3526
  DTS := ZBT-WE3526
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := ZBT WE3526
  DEVICE_PACKAGES := \
	kmod-sdhci-mt7620 kmod-mt7603 kmod-mt76x2 \
	kmod-usb3 kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += zbtlink_zbt-we3526

define Device/zbt-wg2626
  DTS := ZBT-WG2626
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := ZBT WG2626
  DEVICE_PACKAGES := \
	kmod-ata-core kmod-ata-ahci kmod-sdhci-mt7620 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += zbt-wg2626

define Device/zbt-wg3526-16M
  DTS := ZBT-WG3526-16M
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  SUPPORTED_DEVICES += zbt-wg3526
  DEVICE_TITLE := ZBT WG3526 (16MB flash)
  DEVICE_PACKAGES := \
	kmod-ata-core kmod-ata-ahci kmod-sdhci-mt7620 kmod-mt7603 kmod-mt76x2 \
	kmod-usb3 kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += zbt-wg3526-16M

define Device/zbt-wg3526-32M
  DTS := ZBT-WG3526-32M
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
  SUPPORTED_DEVICES += ac1200pro
  DEVICE_TITLE := ZBT WG3526 (32MB flash)
  DEVICE_PACKAGES := \
	kmod-ata-core kmod-ata-ahci kmod-sdhci-mt7620 kmod-mt7603 kmod-mt76x2 \
	kmod-usb3 kmod-usb-ledtrig-usbport wpad-basic
endef
TARGET_DEVICES += zbt-wg3526-32M
