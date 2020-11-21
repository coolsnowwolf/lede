#
# MT7621 Profiles
#

include ./common-tp-link.mk

DEFAULT_SOC := mt7621

KERNEL_DTB += -d21
DEVICE_VARS += UIMAGE_MAGIC SERCOMM_HWNAME

# The OEM webinterface expects an kernel with initramfs which has the uImage
# header field ih_name.
# We don't want to set the header name field for the kernel include in the
# sysupgrade image as well, as this image shouldn't be accepted by the OEM
# webinterface. It will soft-brick the board.
define Build/custom-initramfs-uimage
	mkimage -A $(LINUX_KARCH) \
		-O linux -T kernel \
		-C lzma -a $(KERNEL_LOADADDR) $(if $(UIMAGE_MAGIC),-M $(UIMAGE_MAGIC),) \
		-e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-n '$(1)' -d $@ $@.new
	mv $@.new $@
endef

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

define Build/iodata-mstc-header
	( \
		data_size_crc="$$(dd if=$@ ibs=64 skip=1 2>/dev/null | gzip -c | \
			tail -c 8 | od -An -tx8 --endian little | tr -d ' \n')"; \
		echo -ne "$$(echo $$data_size_crc | sed 's/../\\x&/g')" | \
			dd of=$@ bs=8 count=1 seek=7 conv=notrunc 2>/dev/null; \
	)
	dd if=/dev/zero of=$@ bs=4 count=1 seek=1 conv=notrunc 2>/dev/null
	( \
		header_crc="$$(dd if=$@ bs=64 count=1 2>/dev/null | gzip -c | \
			tail -c 8 | od -An -N4 -tx4 --endian little | tr -d ' \n')"; \
		echo -ne "$$(echo $$header_crc | sed 's/../\\x&/g')" | \
			dd of=$@ bs=4 count=1 seek=1 conv=notrunc 2>/dev/null; \
	)
endef

define Build/ubnt-erx-factory-image
	if [ -e $(KDIR)/tmp/$(KERNEL_INITRAMFS_IMAGE) -a "$$(stat -c%s $@)" -lt "$(KERNEL_SIZE)" ]; then \
		echo '21001:7' > $(1).compat; \
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

define Device/afoundry_ew1200
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := AFOUNDRY
  DEVICE_MODEL := EW1200
  DEVICE_PACKAGES := kmod-ata-ahci kmod-mt76x2 kmod-mt7603 kmod-usb3 \
	kmod-usb-ledtrig-usbport wpad-openssl
  SUPPORTED_DEVICES += ew1200
endef
TARGET_DEVICES += afoundry_ew1200

define Device/alfa-network_quad-e4g
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := ALFA Network
  DEVICE_MODEL := Quad-E4G
  DEVICE_PACKAGES := kmod-ata-ahci kmod-sdhci-mt7620 kmod-usb3 uboot-envtools
endef
TARGET_DEVICES += alfa-network_quad-e4g

define Device/asiarf_ap7621-001
  IMAGE_SIZE := 16000k
  DEVICE_VENDOR := AsiaRF
  DEVICE_MODEL := AP7621-001
  DEVICE_PACKAGES := kmod-sdhci-mt7620 kmod-mt76x2 kmod-usb3
endef
TARGET_DEVICES += asiarf_ap7621-001

define Device/asiarf_ap7621-nv1
  IMAGE_SIZE := 16000k
  DEVICE_VENDOR := AsiaRF
  DEVICE_MODEL := AP7621-NV1
  DEVICE_PACKAGES := kmod-sdhci-mt7620 kmod-mt76x2 kmod-usb3
endef
TARGET_DEVICES += asiarf_ap7621-nv1

define Device/asus_rt-ac57u
  DEVICE_VENDOR := ASUS
  DEVICE_MODEL := RT-AC57U
  IMAGE_SIZE := 16064k
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport wpad-openssl
endef
TARGET_DEVICES += asus_rt-ac57u

define Device/asus_rt-ac65p
  DEVICE_VENDOR := ASUS
  DEVICE_MODEL := RT-AC65P
  IMAGE_SIZE := 51200k
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/factory.bin := append-kernel | pad-to $$(KERNEL_SIZE) | append-ubi | \
	check-size
  DEVICE_PACKAGES := kmod-usb3 kmod-mt7615e kmod-mt7615-firmware wpad-openssl uboot-envtools
endef
TARGET_DEVICES += asus_rt-ac65p

define Device/asus_rt-ac85p
  DEVICE_VENDOR := ASUS
  DEVICE_MODEL := RT-AC85P
  IMAGE_SIZE := 51200k
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/factory.bin := append-kernel | pad-to $$(KERNEL_SIZE) | append-ubi | \
	check-size
  DEVICE_PACKAGES := kmod-usb3 kmod-mt7615e kmod-mt7615-firmware wpad-openssl uboot-envtools
endef
TARGET_DEVICES += asus_rt-ac85p

define Device/buffalo_wsr-1166dhp
  $(Device/uimage-lzma-loader)
  IMAGE/sysupgrade.bin := trx | pad-rootfs | append-metadata
  IMAGE_SIZE := 15936k
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WSR-1166DHP
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 wpad-openssl
  SUPPORTED_DEVICES += wsr-1166
endef
TARGET_DEVICES += buffalo_wsr-1166dhp

define Device/buffalo_wsr-2533dhpl
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 7936k
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WSR-2533DHPL
  DEVICE_ALT0_VENDOR := Buffalo
  DEVICE_ALT0_MODEL := WSR-2533DHP
  IMAGE/sysupgrade.bin := trx | pad-rootfs | append-metadata
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware wpad-openssl
endef
TARGET_DEVICES += buffalo_wsr-2533dhpl

define Device/buffalo_wsr-600dhp
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WSR-600DHP
  DEVICE_PACKAGES := kmod-mt7603 kmod-rt2800-pci wpad-openssl
  SUPPORTED_DEVICES += wsr-600
endef
TARGET_DEVICES += buffalo_wsr-600dhp

define Device/dlink_dir-8xx-a1
  IMAGE_SIZE := 16000k
  DEVICE_VENDOR := D-Link
  DEVICE_PACKAGES := kmod-mt7615d luci-app-mtwifi uboot-envtools
endef

define Device/dlink_dir-860l-b1
  $(Device/seama)
  BLOCKSIZE := 64k
  SEAMA_SIGNATURE := wrgac13_dlink.2013gui_dir860lb
  LOADER_TYPE := bin
  KERNEL := kernel-bin | append-dtb | lzma | loader-kernel | relocate-kernel | \
	lzma -a0 | uImage lzma
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-860L
  DEVICE_VARIANT := B1
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-openssl
  SUPPORTED_DEVICES += dir-860l-b1
endef
TARGET_DEVICES += dlink_dir-860l-b1

define Device/dlink_dir-867-a1
  $(Device/dlink_dir-8xx-a1)
  DEVICE_MODEL := DIR-867
  DEVICE_VARIANT := A1
endef
TARGET_DEVICES += dlink_dir-867-a1

define Device/dlink_dir-878-a1
  $(Device/dlink_dir-8xx-a1)
  DEVICE_MODEL := DIR-878
  DEVICE_VARIANT := A1
endef
TARGET_DEVICES += dlink_dir-878-a1

define Device/dlink_dir-882-a1
  $(Device/dlink_dir-8xx-a1)
  DEVICE_MODEL := DIR-882
  DEVICE_VARIANT := A1
  DEVICE_PACKAGES += kmod-usb3 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += dlink_dir-882-a1

define Device/d-team_newifi-d2
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := Newifi
  DEVICE_MODEL := D2
  DEVICE_PACKAGES := kmod-mt7603e kmod-mt76x2e kmod-usb3 \
	kmod-usb-ledtrig-usbport luci-app-mtwifi
endef
TARGET_DEVICES += d-team_newifi-d2

define Device/d-team_pbr-m1
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := PandoraBox
  DEVICE_MODEL := PBR-M1
  DEVICE_PACKAGES := kmod-ata-ahci kmod-mt7603 kmod-mt76x2 kmod-sdhci-mt7620 \
	kmod-usb3 kmod-usb-ledtrig-usbport wpad-openssl
  SUPPORTED_DEVICES += pbr-m1
endef
TARGET_DEVICES += d-team_pbr-m1

define Device/edimax_ra21s
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Edimax
  DEVICE_MODEL := RA21S
  DEVICE_ALT0_VENDOR := Edimax
  DEVICE_ALT0_MODEL := Gemini RA21S
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | \
	elx-header 02020040 8844A2D168B45A2D
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware wpad-openssl
endef
TARGET_DEVICES += edimax_ra21s

define Device/edimax_rg21s
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Edimax
  DEVICE_MODEL := Gemini AC2600 RG21S
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | \
	elx-header 02020038 8844A2D168B45A2D
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware wpad-openssl
endef
TARGET_DEVICES += edimax_rg21s

define Device/elecom_wrc-1167ghbk2-s
  IMAGE_SIZE := 15488k
  DEVICE_VENDOR := ELECOM
  DEVICE_MODEL := WRC-1167GHBK2-S
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | \
	elecom-wrc-factory WRC-1167GHBK2-S 0.00
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware wpad-openssl
endef
TARGET_DEVICES += elecom_wrc-1167ghbk2-s

define Device/elecom_wrc-1900gst
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 11264k
  DEVICE_VENDOR := ELECOM
  DEVICE_MODEL := WRC-1900GST
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | \
	elecom-gst-factory WRC-1900GST 0.00
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware wpad-openssl
endef
TARGET_DEVICES += elecom_wrc-1900gst

define Device/elecom_wrc-2533gst
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 11264k
  DEVICE_VENDOR := ELECOM
  DEVICE_MODEL := WRC-2533GST
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | \
	elecom-gst-factory WRC-2533GST 0.00
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware wpad-openssl
endef
TARGET_DEVICES += elecom_wrc-2533gst

define Device/elecom_wrc-2533gst2
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 24576k
  DEVICE_VENDOR := ELECOM
  DEVICE_MODEL := WRC-2533GST2
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | \
	elecom-gst-factory WRC-2533GST2 0.00
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware wpad-openssl
endef
TARGET_DEVICES += elecom_wrc-2533gst2

define Device/firefly_firewrt
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Firefly
  DEVICE_MODEL := FireWRT
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-openssl
  SUPPORTED_DEVICES += firewrt
endef
TARGET_DEVICES += firefly_firewrt

define Device/gehua_ghl-r-001
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := GeHua
  DEVICE_MODEL := GHL-R-001
  DEVICE_PACKAGES := kmod-mt7603e kmod-mt76x2e kmod-usb3 \
	kmod-usb-ledtrig-usbport luci-app-mtwifi
endef
TARGET_DEVICES += gehua_ghl-r-001

define Device/gnubee_gb-pc1
  DEVICE_VENDOR := GnuBee
  DEVICE_MODEL := Personal Cloud One
  DEVICE_PACKAGES := kmod-ata-ahci kmod-usb3 kmod-sdhci-mt7620
  IMAGE_SIZE := 32448k
endef
TARGET_DEVICES += gnubee_gb-pc1

define Device/gnubee_gb-pc2
  DEVICE_VENDOR := GnuBee
  DEVICE_MODEL := Personal Cloud Two
  DEVICE_PACKAGES := kmod-ata-ahci kmod-usb3 kmod-sdhci-mt7620
  IMAGE_SIZE := 32448k
endef
TARGET_DEVICES += gnubee_gb-pc2

define Device/hiwifi_hc5962
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  UBINIZE_OPTS := -E 5
  IMAGE_SIZE := 32768k
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/factory.bin := append-kernel | pad-to $$(KERNEL_SIZE) | append-ubi | \
	check-size
  DEVICE_VENDOR := HiWiFi
  DEVICE_MODEL := HC5962
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 wpad-openssl
endef
TARGET_DEVICES += hiwifi_hc5962

define Device/iodata_wn-ax1167gr
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 15552k
  KERNEL_INITRAMFS := $$(KERNEL) | \
	iodata-factory 7864320 4 0x1055 $(KDIR)/tmp/$$(KERNEL_INITRAMFS_PREFIX)-factory.bin
  DEVICE_VENDOR := I-O DATA
  DEVICE_MODEL := WN-AX1167GR
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 wpad-openssl
endef
TARGET_DEVICES += iodata_wn-ax1167gr

define Device/iodata_nand
  DEVICE_VENDOR := I-O DATA
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  UBINIZE_OPTS := -E 5
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 51200k
  LOADER_TYPE := bin
  KERNEL := kernel-bin | append-dtb | lzma | loader-kernel | lzma | uImage lzma
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef

define Device/iodata_wn-ax1167gr2
  $(Device/iodata_nand)
  UIMAGE_MAGIC := 0x434f4d42
  DEVICE_MODEL := WN-AX1167GR2
  KERNEL_INITRAMFS := $(KERNEL_DTB) | loader-kernel | lzma | \
	custom-initramfs-uimage 3.10(XBC.1)b10 | iodata-mstc-header
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware wpad-openssl
endef
TARGET_DEVICES += iodata_wn-ax1167gr2

define Device/iodata_wn-ax2033gr
  $(Device/iodata_nand)
  UIMAGE_MAGIC := 0x434f4d42
  DEVICE_MODEL := WN-AX2033GR
  KERNEL_INITRAMFS := $(KERNEL_DTB) | loader-kernel | lzma | \
	custom-initramfs-uimage 3.10(VST.1)C10 | iodata-mstc-header
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt7615e kmod-mt7615-firmware wpad-openssl
endef
TARGET_DEVICES += iodata_wn-ax2033gr

define Device/iodata_wn-dx1167r
  $(Device/iodata_nand)
  UIMAGE_MAGIC := 0x434f4d43
  DEVICE_MODEL := WN-DX1167R
  KERNEL_INITRAMFS := $(KERNEL_DTB) | loader-kernel | lzma | \
	custom-initramfs-uimage 3.10(XIK.1)b10 | iodata-mstc-header
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware wpad-openssl
endef
TARGET_DEVICES += iodata_wn-dx1167r

define Device/iodata_wn-gx300gr
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 7616k
  DEVICE_VENDOR := I-O DATA
  DEVICE_MODEL := WN-GX300GR
  DEVICE_PACKAGES := kmod-mt7603 wpad-openssl
endef
TARGET_DEVICES += iodata_wn-gx300gr

define Device/iodata_wnpr2600g
  $(Device/uimage-lzma-loader)
  DEVICE_VENDOR := I-O DATA
  DEVICE_MODEL := WNPR2600G
  IMAGE_SIZE := 13952k
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | \
	elx-header 0104003a 8844A2D168B45A2D
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware wpad-openssl
endef
TARGET_DEVICES += iodata_wnpr2600g

define Device/iptime_a6ns-m
  IMAGE_SIZE := 16128k
  UIMAGE_NAME := a6nm
  DEVICE_VENDOR := ipTIME
  DEVICE_MODEL := A6ns-M
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware kmod-usb3 kmod-usb-ledtrig-usbport wpad-openssl
endef
TARGET_DEVICES += iptime_a6ns-m

define Device/iptime_a8004t
  IMAGE_SIZE := 16128k
  UIMAGE_NAME := a8004t
  DEVICE_VENDOR := ipTIME
  DEVICE_MODEL := A8004T
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware kmod-usb3 wpad-openssl
endef
TARGET_DEVICES += iptime_a8004t

define Device/jcg_jhr-ac876m
  IMAGE_SIZE := 16064k
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | jcg-header 89.1
  JCG_MAXSIZE := 16064k
  DEVICE_VENDOR := JCG
  DEVICE_MODEL := JHR-AC876M
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware kmod-usb3 kmod-usb-ledtrig-usbport wpad-openssl
endef
TARGET_DEVICES += jcg_jhr-ac876m

define Device/jcg_y2
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 16064k
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | jcg-header 95.1
  JCG_MAXSIZE := 16064k
  DEVICE_VENDOR := JCG
  DEVICE_MODEL := Y2
  DEVICE_PACKAGES := kmod-mt7615d kmod-usb3 luci-app-mtwifi
endef
TARGET_DEVICES += jcg_y2

define Device/jdcloud_re-sp-01b
  IMAGE_SIZE := 27328k
  DEVICE_VENDOR := JDCloud
  DEVICE_MODEL := RE-SP-01B
  DEVICE_PACKAGES := kmod-fs-ext4 kmod-mt7603 kmod-mt7615e kmod-sdhci-mt7620 \
	kmod-usb3 wpad-openssl
endef
TARGET_DEVICES += jdcloud_re-sp-01b

define Device/lenovo_newifi-d1
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := Newifi
  DEVICE_MODEL := D1
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-sdhci-mt7620 \
	kmod-usb-ledtrig-usbport wpad-openssl
  SUPPORTED_DEVICES += newifi-d1
endef
TARGET_DEVICES += lenovo_newifi-d1

define Device/linksys_ea7500-v2
  $(Device/uimage-lzma-loader)
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 36864k
  DEVICE_VENDOR := Linksys
  DEVICE_MODEL := EA7500
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := kmod-usb3 kmod-mt7615e kmod-mt7615-firmware wpad-openssl uboot-envtools
  UBINIZE_OPTS := -E 5
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata | check-size
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | \
	append-ubi | check-size | linksys-image type=EA7500v2
endef
TARGET_DEVICES += linksys_ea7500-v2

define Device/linksys_re6500
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Linksys
  DEVICE_MODEL := RE6500
  DEVICE_PACKAGES := kmod-mt76x2 wpad-openssl
  SUPPORTED_DEVICES += re6500
endef
TARGET_DEVICES += linksys_re6500

define Device/mediatek_ap-mt7621a-v60
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Mediatek
  DEVICE_MODEL := AP-MT7621A-V60 EVB
  DEVICE_PACKAGES := kmod-usb3 kmod-sdhci-mt7620 kmod-sound-mt7620
endef
TARGET_DEVICES += mediatek_ap-mt7621a-v60

define Device/mediatek_mt7621-eval-board
  BLOCKSIZE := 64k
  IMAGE_SIZE := 15104k
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MT7621 EVB
  SUPPORTED_DEVICES += mt7621
endef
TARGET_DEVICES += mediatek_mt7621-eval-board

define Device/MikroTik
  DEVICE_VENDOR := MikroTik
  BLOCKSIZE := 64k
  IMAGE_SIZE := 16128k
  DEVICE_PACKAGES := kmod-usb3
  LOADER_TYPE := elf
  KERNEL := $(KERNEL_DTB) | loader-kernel
  IMAGE/sysupgrade.bin := append-kernel | kernel2minor -s 1024 | \
	pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | append-metadata | \
	check-size
endef

define Device/mikrotik_routerboard-750gr3
  $(Device/MikroTik)
  DEVICE_MODEL := RouterBOARD 750Gr3
  DEVICE_PACKAGES += kmod-gpio-beeper
  SUPPORTED_DEVICES += mikrotik,rb750gr3
endef
TARGET_DEVICES += mikrotik_routerboard-750gr3

define Device/mikrotik_routerboard-m11g
  $(Device/MikroTik)
  DEVICE_MODEL := RouterBOARD M11G
  SUPPORTED_DEVICES += mikrotik,rbm11g
endef
TARGET_DEVICES += mikrotik_routerboard-m11g

define Device/mikrotik_routerboard-m33g
  $(Device/MikroTik)
  DEVICE_MODEL := RouterBOARD M33G
  SUPPORTED_DEVICES += mikrotik,rbm33g
endef
TARGET_DEVICES += mikrotik_routerboard-m33g

define Device/motorola_mr2600
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Motorola
  DEVICE_MODEL := MR2600
  DEVICE_PACKAGES := kmod-mt7615d kmod-usb3 kmod-usb-ledtrig-usbport luci-app-mtwifi uboot-envtools
endef
TARGET_DEVICES += motorola_mr2600

define Device/mqmaker_witi
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := MQmaker
  DEVICE_MODEL := WiTi
  DEVICE_PACKAGES := kmod-ata-ahci kmod-mt76x2 kmod-sdhci-mt7620 kmod-usb3 \
	kmod-usb-ledtrig-usbport wpad-openssl
  SUPPORTED_DEVICES += witi mqmaker,witi-256m mqmaker,witi-512m
endef
TARGET_DEVICES += mqmaker_witi

define Device/mtc_wr1201
  IMAGE_SIZE := 16000k
  DEVICE_VENDOR := MTC
  DEVICE_MODEL := Wireless Router WR1201
  KERNEL_INITRAMFS := $(KERNEL_DTB) | custom-initramfs-uimage WR1201_8_128
  DEVICE_PACKAGES := kmod-sdhci-mt7620 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport wpad-openssl
endef
TARGET_DEVICES += mtc_wr1201

define Device/netgear_ex6150
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := EX6150
  DEVICE_PACKAGES := kmod-mt76x2 wpad-openssl
  NETGEAR_BOARD_ID := U12H318T00_NETGEAR
  IMAGE_SIZE := 14848k
  IMAGES += factory.chk
  IMAGE/factory.chk := $$(sysupgrade_bin) | check-size | netgear-chk
endef
TARGET_DEVICES += netgear_ex6150

define Device/netgear_sercomm_nand
  $(Device/uimage-lzma-loader)
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  UBINIZE_OPTS := -E 5
  IMAGES += factory.img kernel.bin rootfs.bin
  IMAGE/factory.img := pad-extra 2048k | append-kernel | pad-to 6144k | \
	append-ubi | pad-to $$$$(BLOCKSIZE) | sercom-footer | pad-to 128 | \
	zip $$$$(SERCOMM_HWNAME).bin | sercom-seal
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/kernel.bin := append-kernel
  IMAGE/rootfs.bin := append-ubi | check-size
  DEVICE_VENDOR := NETGEAR
  DEVICE_PACKAGES := kmod-mt7603 kmod-usb3 kmod-usb-ledtrig-usbport wpad-openssl
endef

define Device/netgear_r6220
  $(Device/netgear_sercomm_nand)
  DEVICE_MODEL := R6220
  SERCOMM_HWNAME := R6220
  SERCOMM_HWID := AYA
  SERCOMM_HWVER := A001
  SERCOMM_SWVER := 0x0086
  IMAGE_SIZE := 28672k
  DEVICE_PACKAGES += kmod-mt76x2
  SUPPORTED_DEVICES += r6220
endef
TARGET_DEVICES += netgear_r6220

define Device/netgear_r6260
  $(Device/netgear_sercomm_nand)
  DEVICE_MODEL := R6260
  SERCOMM_HWNAME := R6260
  SERCOMM_HWID := CHJ
  SERCOMM_HWVER := A001
  SERCOMM_SWVER := 0x0052
  IMAGE_SIZE := 40960k
  DEVICE_PACKAGES += kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += netgear_r6260

define Device/netgear_r6350
  $(Device/netgear_sercomm_nand)
  DEVICE_MODEL := R6350
  SERCOMM_HWNAME := R6350
  SERCOMM_HWID := CHJ
  SERCOMM_HWVER := A001
  SERCOMM_SWVER := 0x0052
  IMAGE_SIZE := 40960k
  DEVICE_PACKAGES += kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += netgear_r6350

define Device/netgear_r6700-v2
  $(Device/netgear_sercomm_nand)
  DEVICE_MODEL := R6700
  DEVICE_VARIANT := v2
  DEVICE_ALT0_VENDOR := NETGEAR
  DEVICE_ALT0_MODEL := Nighthawk AC2400
  DEVICE_ALT0_VARIANT := v1
  DEVICE_ALT1_VENDOR := NETGEAR
  DEVICE_ALT1_MODEL := R7200
  DEVICE_ALT1_VARIANT := v1
  SERCOMM_HWNAME := R6950
  SERCOMM_HWID := BZV
  SERCOMM_HWVER := A001
  SERCOMM_SWVER := 0x1032
  IMAGE_SIZE := 40960k
  DEVICE_PACKAGES += kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += netgear_r6700-v2

define Device/netgear_r6800
  $(Device/netgear_sercomm_nand)
  DEVICE_MODEL := R6800
  SERCOMM_HWNAME := R6950
  SERCOMM_HWID := BZV
  SERCOMM_HWVER := A001
  SERCOMM_SWVER := 0x0062
  IMAGE_SIZE := 40960k
  DEVICE_PACKAGES += kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += netgear_r6800

define Device/netgear_r6850
  $(Device/netgear_sercomm_nand)
  DEVICE_MODEL := R6850
  SERCOMM_HWNAME := R6850
  SERCOMM_HWID := CHJ
  SERCOMM_HWVER := A001
  SERCOMM_SWVER := 0x0052
  IMAGE_SIZE := 40960k
  DEVICE_PACKAGES += -kmod-mt7603 -wpad-openssl kmod-mt7603e kmod-mt7615d luci-app-mtwifi
endef
TARGET_DEVICES += netgear_r6850

define Device/netgear_wac104
  $(Device/netgear_sercomm_nand)
  DEVICE_MODEL := WAC104
  SERCOMM_HWNAME := WAC104
  SERCOMM_HWID := CAY
  SERCOMM_HWVER := A001
  SERCOMM_SWVER := 0x0006
  IMAGE_SIZE := 28672k
  DEVICE_PACKAGES += kmod-mt76x2
endef
TARGET_DEVICES += netgear_wac104

define Device/netgear_wac124
  $(Device/netgear_sercomm_nand)
  DEVICE_MODEL := WAC124
  SERCOMM_HWNAME := WAC124
  SERCOMM_HWID := CTL
  SERCOMM_HWVER := A003
  SERCOMM_SWVER := 0x0402
  IMAGE_SIZE := 40960k
  DEVICE_PACKAGES += kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += netgear_wac124

define Device/netgear_wndr3700-v5
  $(Device/uimage-lzma-loader)
  BLOCKSIZE := 64k
  IMAGE_SIZE := 15232k
  SERCOMM_HWID := AYB
  SERCOMM_HWVER := A001
  SERCOMM_SWVER := 0x1054
  IMAGES += factory.img
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | \
	pad-rootfs
  IMAGE/sysupgrade.bin := $$(IMAGE/default) | append-metadata | check-size
  IMAGE/factory.img := pad-extra 320k | $$(IMAGE/default) | \
	pad-to $$$$(BLOCKSIZE) | sercom-footer | pad-to 128 | zip WNDR3700v5.bin | \
	sercom-seal
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := WNDR3700
  DEVICE_VARIANT := v5
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport wpad-openssl
  SUPPORTED_DEVICES += wndr3700v5
endef
TARGET_DEVICES += netgear_wndr3700-v5

define Device/netis_wf2881
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  FILESYSTEMS := squashfs
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 129280k
  UBINIZE_OPTS := -E 5
  UIMAGE_NAME := WF2881_0.0.00
  KERNEL_INITRAMFS := $(KERNEL_DTB) | netis-tail WF2881 | uImage lzma
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi | \
	check-size
  DEVICE_VENDOR := NETIS
  DEVICE_MODEL := WF2881
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-openssl
endef
TARGET_DEVICES += netis_wf2881

define Device/phicomm_k2p
  IMAGE_SIZE := 15744k
  DEVICE_VENDOR := Phicomm
  DEVICE_MODEL := K2P
  SUPPORTED_DEVICES += k2p
  DEVICE_PACKAGES := kmod-mt7615d luci-app-mtwifi
endef
TARGET_DEVICES += phicomm_k2p

define Device/planex_vr500
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 65216k
  DEVICE_VENDOR := Planex
  DEVICE_MODEL := VR500
  DEVICE_PACKAGES := kmod-usb3
  SUPPORTED_DEVICES += vr500
endef
TARGET_DEVICES += planex_vr500

define Device/samknows_whitebox-v8
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := SamKnows
  DEVICE_MODEL := Whitebox 8
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport uboot-envtools wpad-openssl
  SUPPORTED_DEVICES += sk-wb8
endef
TARGET_DEVICES += samknows_whitebox-v8

define Device/storylink_sap-g3200u3
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := STORYLiNK
  DEVICE_MODEL := SAP-G3200U3
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport wpad-openssl
  SUPPORTED_DEVICES += sap-g3200u3
endef
TARGET_DEVICES += storylink_sap-g3200u3

define Device/telco-electronics_x1
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Telco Electronics
  DEVICE_MODEL := X1
  DEVICE_PACKAGES := kmod-usb3 kmod-mt76 wpad-openssl
endef
TARGET_DEVICES += telco-electronics_x1

define Device/thunder_timecloud
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Thunder
  DEVICE_MODEL := Timecloud
  DEVICE_PACKAGES := kmod-usb3
  SUPPORTED_DEVICES += timecloud
endef
TARGET_DEVICES += thunder_timecloud

define Device/totolink_a7000r
  IMAGE_SIZE := 16064k
  UIMAGE_NAME := C8340R1C-9999
  DEVICE_VENDOR := TOTOLINK
  DEVICE_MODEL := A7000R
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware wpad-openssl
endef
TARGET_DEVICES += totolink_a7000r

define Device/adslr_g7
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := ADSLR
  DEVICE_MODEL := G7
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware wpad-openssl
endef
TARGET_DEVICES += adslr_g7

define Device/tplink_re350-v1
  $(Device/tplink-safeloader)
  DEVICE_MODEL := RE350
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 wpad-openssl
  TPLINK_BOARD_ID := RE350-V1
  IMAGE_SIZE := 6016k
  SUPPORTED_DEVICES += re350-v1
endef
TARGET_DEVICES += tplink_re350-v1

define Device/tplink_re650-v1
  $(Device/tplink-safeloader)
  DEVICE_MODEL := RE650
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware wpad-openssl
  TPLINK_BOARD_ID := RE650-V1
  IMAGE_SIZE := 14208k
endef
TARGET_DEVICES += tplink_re650-v1

define Device/ubnt_edgerouter_common
  $(Device/uimage-lzma-loader)
  DEVICE_VENDOR := Ubiquiti
  IMAGE_SIZE := 256768k
  FILESYSTEMS := squashfs
  KERNEL_SIZE := 3145728
  KERNEL_INITRAMFS := $$(KERNEL) | \
	ubnt-erx-factory-image $(KDIR)/tmp/$$(KERNEL_INITRAMFS_PREFIX)-factory.tar
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef

define Device/ubnt_edgerouter-x
  $(Device/ubnt_edgerouter_common)
  DEVICE_MODEL := EdgeRouter X
  SUPPORTED_DEVICES += ubnt-erx ubiquiti,edgerouterx
endef
TARGET_DEVICES += ubnt_edgerouter-x

define Device/ubnt_edgerouter-x-sfp
  $(Device/ubnt_edgerouter_common)
  DEVICE_MODEL := EdgeRouter X SFP
  DEVICE_PACKAGES += kmod-i2c-algo-pca kmod-gpio-pca953x kmod-sfp
  SUPPORTED_DEVICES += ubnt-erx-sfp ubiquiti,edgerouterx-sfp
endef
TARGET_DEVICES += ubnt_edgerouter-x-sfp

define Device/ubnt_unifi-nanohd
  DEVICE_VENDOR := Ubiquiti
  DEVICE_MODEL := UniFi nanoHD
  DEVICE_PACKAGES += kmod-mt7603 kmod-mt7615e kmod-mt7615-firmware wpad-openssl
  IMAGE_SIZE := 15552k
endef
TARGET_DEVICES += ubnt_unifi-nanohd

define Device/unielec_u7621-06-16m
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := UniElec
  DEVICE_MODEL := U7621-06
  DEVICE_VARIANT := 16M
  DEVICE_PACKAGES := kmod-ata-ahci kmod-sdhci-mt7620 kmod-usb3
  SUPPORTED_DEVICES += u7621-06-256M-16M unielec,u7621-06-256m-16m
endef
TARGET_DEVICES += unielec_u7621-06-16m

define Device/unielec_u7621-06-64m
  IMAGE_SIZE := 65216k
  DEVICE_VENDOR := UniElec
  DEVICE_MODEL := U7621-06
  DEVICE_VARIANT := 64M
  DEVICE_PACKAGES := kmod-ata-ahci kmod-sdhci-mt7620 kmod-usb3
  SUPPORTED_DEVICES += unielec,u7621-06-512m-64m
endef
TARGET_DEVICES += unielec_u7621-06-64m

define Device/wevo_11acnas
  IMAGE_SIZE := 16064k
  UIMAGE_NAME := 11AC-NAS-Router(0.0.0)
  DEVICE_VENDOR := WeVO
  DEVICE_MODEL := 11AC NAS Router
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport wpad-openssl
  SUPPORTED_DEVICES += 11acnas
endef
TARGET_DEVICES += wevo_11acnas

define Device/wevo_w2914ns-v2
  IMAGE_SIZE := 16064k
  UIMAGE_NAME := W2914NS-V2(0.0.0)
  DEVICE_VENDOR := WeVO
  DEVICE_MODEL := W2914NS
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport wpad-openssl
  SUPPORTED_DEVICES += w2914nsv2
endef
TARGET_DEVICES += wevo_w2914ns-v2

define Device/xiaomi_mir3g
  $(Device/uimage-lzma-loader)
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 124416k
  UBINIZE_OPTS := -E 5
  IMAGES += kernel1.bin rootfs0.bin
  IMAGE/kernel1.bin := append-kernel
  IMAGE/rootfs0.bin := append-ubi | check-size
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Mi Router 3G
  SUPPORTED_DEVICES += R3G
  SUPPORTED_DEVICES += mir3g
  DEVICE_PACKAGES := kmod-mt7603e kmod-mt76x2e kmod-usb3 \
	kmod-usb-ledtrig-usbport luci-app-mtwifi uboot-envtools
endef
TARGET_DEVICES += xiaomi_mir3g

define Device/xiaomi_mir3g-v2
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 14848k
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Mi Router 3G
  DEVICE_VARIANT := v2
  DEVICE_ALT0_VENDOR := Xiaomi
  DEVICE_ALT0_MODEL := Mi Router 4A
  DEVICE_ALT0_VARIANT := Gigabit Edition
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 wpad-openssl
endef
TARGET_DEVICES += xiaomi_mir3g-v2

define Device/xiaomi_mir3p
  $(Device/uimage-lzma-loader)
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE:= 4096k
  UBINIZE_OPTS := -E 5
  IMAGE_SIZE := 255488k
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Mi Router 3 Pro
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/factory.bin := append-kernel | pad-to $$(KERNEL_SIZE) | append-ubi | \
	check-size
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware kmod-usb3 kmod-usb-ledtrig-usbport \
	wpad-openssl uboot-envtools
endef
TARGET_DEVICES += xiaomi_mir3p

define Device/xiaomi_mir4
  $(Device/uimage-lzma-loader)
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 124416k
  UBINIZE_OPTS := -E 5
  IMAGES += kernel1.bin rootfs0.bin
  IMAGE/kernel1.bin := append-kernel
  IMAGE/rootfs0.bin := append-ubi | check-size
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Mi Router 4
  SUPPORTED_DEVICES += R4
  SUPPORTED_DEVICES += mir4
  DEVICE_PACKAGES := kmod-mt7603e kmod-mt76x2e luci-app-mtwifi uboot-envtools
endef
TARGET_DEVICES += xiaomi_mir4

define Device/xiaomi-ac2100
  $(Device/uimage-lzma-loader)
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 120320k
  UBINIZE_OPTS := -E 5
  IMAGES += kernel1.bin rootfs0.bin factory.bin
  IMAGE/kernel1.bin := append-kernel
  IMAGE/rootfs0.bin := append-ubi | check-size
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/factory.bin := append-kernel | pad-to $$(KERNEL_SIZE) | append-ubi | check-size
  DEVICE_VENDOR := Xiaomi
  DEVICE_PACKAGES := kmod-mt7603e kmod-mt7615d luci-app-mtwifi uboot-envtools
endef

define Device/xiaomi_mi-router-ac2100
  $(Device/xiaomi-ac2100)
  DEVICE_MODEL := Mi Router AC2100
endef
TARGET_DEVICES += xiaomi_mi-router-ac2100

define Device/xiaomi_redmi-router-ac2100
  $(Device/xiaomi-ac2100)
  DEVICE_MODEL := Redmi Router AC2100
endef
TARGET_DEVICES += xiaomi_redmi-router-ac2100

define Device/xiaoyu_xy-c5
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := XiaoYu
  DEVICE_MODEL := XY-C5
  DEVICE_PACKAGES := kmod-ata-ahci kmod-usb3
endef
TARGET_DEVICES += xiaoyu_xy-c5

define Device/xzwifi_creativebox-v1
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := CreativeBox
  DEVICE_MODEL := v1
  DEVICE_PACKAGES := kmod-ata-ahci kmod-mt7603 kmod-mt76x2 kmod-sdhci-mt7620 \
	kmod-usb3
endef
TARGET_DEVICES += xzwifi_creativebox-v1

define Device/youhua_wr1200js
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := YouHua
  DEVICE_MODEL := WR1200JS
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport wpad-openssl
endef
TARGET_DEVICES += youhua_wr1200js

define Device/youku_yk-l2
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Youku
  DEVICE_MODEL := YK-L2
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport wpad-openssl
endef
TARGET_DEVICES += youku_yk-l2

define Device/zbtlink_zbt-we1326
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-WE1326
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-sdhci-mt7620 \
	wpad-openssl
  SUPPORTED_DEVICES += zbt-we1326
endef
TARGET_DEVICES += zbtlink_zbt-we1326

define Device/zbtlink_zbt-we3526
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-WE3526
  DEVICE_PACKAGES := kmod-sdhci-mt7620 kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport wpad-openssl
endef
TARGET_DEVICES += zbtlink_zbt-we3526

define Device/zbtlink_zbt-wg2626
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-WG2626
  DEVICE_PACKAGES := kmod-ata-ahci kmod-sdhci-mt7620 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport wpad-openssl
  SUPPORTED_DEVICES += zbt-wg2626
endef
TARGET_DEVICES += zbtlink_zbt-wg2626

define Device/zbtlink_zbt-wg3526-16m
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-WG3526
  DEVICE_VARIANT := 16M
  DEVICE_PACKAGES := kmod-ata-ahci kmod-sdhci-mt7620 kmod-mt7603 kmod-mt76x2 \
	kmod-usb3 kmod-usb-ledtrig-usbport wpad-openssl
  SUPPORTED_DEVICES += zbt-wg3526 zbt-wg3526-16M
endef
TARGET_DEVICES += zbtlink_zbt-wg3526-16m

define Device/zbtlink_zbt-wg3526-32m
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-WG3526
  DEVICE_VARIANT := 32M
  DEVICE_PACKAGES := kmod-ata-ahci kmod-sdhci-mt7620 kmod-mt7603 kmod-mt76x2 \
	kmod-usb3 kmod-usb-ledtrig-usbport wpad-openssl
  SUPPORTED_DEVICES += ac1200pro zbt-wg3526-32M
endef
TARGET_DEVICES += zbtlink_zbt-wg3526-32m

define Device/zio_freezio
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := ZIO
  DEVICE_MODEL := FREEZIO
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport wpad-openssl
endef
TARGET_DEVICES += zio_freezio
