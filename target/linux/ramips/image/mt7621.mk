#
# MT7621 Profiles
#

include ./common-tp-link.mk

DEFAULT_SOC := mt7621

KERNEL_DTB += -d21
DEVICE_VARS += ELECOM_HWNAME LINKSYS_HWNAME

define Build/elecom-wrc-gs-factory
	$(eval product=$(word 1,$(1)))
	$(eval version=$(word 2,$(1)))
	$(eval hash_opt=$(word 3,$(1)))
	$(MKHASH) md5 $(hash_opt) $@ >> $@
	( \
		echo -n "ELECOM $(product) v$(version)" | \
			dd bs=32 count=1 conv=sync; \
		dd if=$@; \
	) > $@.new
	mv $@.new $@
endef

define Build/gemtek-trailer
	printf "%s%08X" ".GEMTEK." "$$(cksum $@ | cut -d ' ' -f1)" >> $@
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
		$(MKHASH) md5 $(KDIR)/tmp/$(KERNEL_INITRAMFS_IMAGE) > $(1).md5; \
		$(TAR) -rf $(1) --transform='s/^.*/vmlinux.tmp.md5/' $(1).md5; \
		\
		echo "dummy" > $(1).rootfs; \
		$(TAR) -rf $(1) --transform='s/^.*/squashfs.tmp/' $(1).rootfs; \
		\
		$(MKHASH) md5 $(1).rootfs > $(1).md5; \
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

define Build/zytrx-header
	$(eval board=$(word 1,$(1)))
	$(eval version=$(word 2,$(1)))
	$(STAGING_DIR_HOST)/bin/zytrx -B '$(board)' -v '$(version)' -i $@ -o $@.new
	mv $@.new $@
endef

define Device/dsa-migration
  DEVICE_COMPAT_VERSION := 1.1
  DEVICE_COMPAT_MESSAGE := Config cannot be migrated from swconfig to DSA
endef

define Device/adslr_g7
  $(Device/dsa-migration)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := ADSLR
  DEVICE_MODEL := G7
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += adslr_g7

define Device/afoundry_ew1200
  $(Device/dsa-migration)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := AFOUNDRY
  DEVICE_MODEL := EW1200
  DEVICE_PACKAGES := kmod-ata-ahci kmod-mt76x2 kmod-mt7603 kmod-usb3 \
	kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += ew1200
endef
TARGET_DEVICES += afoundry_ew1200

define Device/alfa-network_quad-e4g
  $(Device/dsa-migration)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := ALFA Network
  DEVICE_MODEL := Quad-E4G
  DEVICE_PACKAGES := kmod-ata-ahci kmod-sdhci-mt7620 kmod-usb3 uboot-envtools \
	-wpad-openssl
  SUPPORTED_DEVICES += quad-e4g
endef
TARGET_DEVICES += alfa-network_quad-e4g

define Device/ampedwireless_ally_common
  $(Device/dsa-migration)
  DEVICE_VENDOR := Amped Wireless
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware uboot-envtools
  IMAGE_SIZE := 32768k
  KERNEL_SIZE := 4096k
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  UBINIZE_OPTS := -E 5
  KERNEL_INITRAMFS := $(KERNEL_DTB) | uImage lzma -n 'flashable-initramfs' |\
	edimax-header -s CSYS -m RN68 -f 0x001c0000 -S 0x01100000
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef

define Device/ampedwireless_ally-r1900k
  $(Device/ampedwireless_ally_common)
  DEVICE_MODEL := ALLY-R1900K
  DEVICE_PACKAGES += kmod-usb3
endef
TARGET_DEVICES += ampedwireless_ally-r1900k

define Device/ampedwireless_ally-00x19k
  $(Device/ampedwireless_ally_common)
  DEVICE_MODEL := ALLY-00X19K
endef
TARGET_DEVICES += ampedwireless_ally-00x19k

define Device/asiarf_ap7621-001
  $(Device/dsa-migration)
  IMAGE_SIZE := 16000k
  DEVICE_VENDOR := AsiaRF
  DEVICE_MODEL := AP7621-001
  DEVICE_PACKAGES := kmod-sdhci-mt7620 kmod-mt76x2 kmod-usb3 -wpad-openssl
endef
TARGET_DEVICES += asiarf_ap7621-001

define Device/asiarf_ap7621-nv1
  $(Device/dsa-migration)
  IMAGE_SIZE := 16000k
  DEVICE_VENDOR := AsiaRF
  DEVICE_MODEL := AP7621-NV1
  DEVICE_PACKAGES := kmod-sdhci-mt7620 kmod-mt76x2 kmod-usb3 -wpad-openssl
endef
TARGET_DEVICES += asiarf_ap7621-nv1

define Device/asus_rt-ac57u
  $(Device/dsa-migration)
  DEVICE_VENDOR := ASUS
  DEVICE_MODEL := RT-AC57U
  IMAGE_SIZE := 16064k
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += asus_rt-ac57u

define Device/asus_rt-ac65p
  $(Device/dsa-migration)
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
  DEVICE_PACKAGES := kmod-usb3 kmod-mt7615e kmod-mt7615-firmware uboot-envtools
endef
TARGET_DEVICES += asus_rt-ac65p

define Device/asus_rt-ac85p
  $(Device/dsa-migration)
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
  DEVICE_PACKAGES := kmod-usb3 kmod-mt7615e kmod-mt7615-firmware uboot-envtools
endef
TARGET_DEVICES += asus_rt-ac85p

define Device/asus_rt-n56u-b1
  $(Device/dsa-migration)
  DEVICE_VENDOR := ASUS
  DEVICE_MODEL := RT-N56U
  DEVICE_VARIANT := B1
  IMAGE_SIZE := 16064k
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += asus_rt-n56u-b1

define Device/buffalo_wsr-1166dhp
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE/sysupgrade.bin := trx | pad-rootfs | append-metadata
  IMAGE_SIZE := 15936k
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WSR-1166DHP
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2
  SUPPORTED_DEVICES += wsr-1166
endef
TARGET_DEVICES += buffalo_wsr-1166dhp

define Device/buffalo_wsr-2533dhpl
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 7936k
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WSR-2533DHPL
  DEVICE_ALT0_VENDOR := Buffalo
  DEVICE_ALT0_MODEL := WSR-2533DHP
  IMAGE/sysupgrade.bin := trx | pad-rootfs | append-metadata
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += buffalo_wsr-2533dhpl

define Device/buffalo_wsr-600dhp
  $(Device/dsa-migration)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WSR-600DHP
  DEVICE_PACKAGES := kmod-mt7603 kmod-rt2800-pci
  SUPPORTED_DEVICES += wsr-600
endef
TARGET_DEVICES += buffalo_wsr-600dhp

define Device/cudy_wr1300
  $(Device/dsa-migration)
  IMAGE_SIZE := 15872k
  DEVICE_VENDOR := Cudy
  DEVICE_MODEL := WR1300
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb2 kmod-usb3 \
	kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += cudy_wr1300

define Device/cudy_wr2100
  $(Device/dsa-migration)
  DEVICE_VENDOR := Cudy
  DEVICE_MODEL := WR2100
  IMAGE_SIZE := 15872k
  UIMAGE_NAME := R11
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += cudy_wr2100

define Device/dlink_dir-8xx-a1
  $(Device/dsa-migration)
  IMAGE_SIZE := 16000k
  DEVICE_VENDOR := D-Link
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware
  KERNEL_INITRAMFS := $$(KERNEL) | uimage-padhdr 96
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | uimage-padhdr 96 |\
	pad-rootfs | check-size | append-metadata
  IMAGE/factory.bin := append-kernel | append-rootfs | uimage-padhdr 96 |\
	check-size
endef

define Device/dlink_dir-8xx-r1
  $(Device/dsa-migration)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := D-Link
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware
  KERNEL_INITRAMFS := $$(KERNEL)
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | \
	check-size | append-metadata
endef

define Device/dlink_dir-xx60-a1
  $(Device/dsa-migration)
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 40960k
  UBINIZE_OPTS := -E 5
  DEVICE_VENDOR := D-Link
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware kmod-usb3 \
	kmod-usb-ledtrig-usbport
  KERNEL := $$(KERNEL) | uimage-padhdr 96
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/factory.bin := append-kernel | pad-to $$(KERNEL_SIZE) | append-ubi | \
	check-size
endef

define Device/dlink_dir-1960-a1
  $(Device/dlink_dir-xx60-a1)
  DEVICE_MODEL := DIR-1960
  DEVICE_VARIANT := A1
endef
TARGET_DEVICES += dlink_dir-1960-a1

define Device/dlink_dir-2640-a1
  $(Device/dlink_dir-xx60-a1)
  DEVICE_MODEL := DIR-2640
  DEVICE_VARIANT := A1
endef
TARGET_DEVICES += dlink_dir-2640-a1

define Device/dlink_dir-2660-a1
  $(Device/dlink_dir-xx60-a1)
  DEVICE_MODEL := DIR-2660
  DEVICE_VARIANT := A1
endef
TARGET_DEVICES += dlink_dir-2660-a1

define Device/dlink_dir-853-a3
  $(Device/dlink_dir-xx60-a1)
  DEVICE_MODEL := DIR-853
  DEVICE_VARIANT := A3
endef
TARGET_DEVICES += dlink_dir-853-a3

define Device/dlink_dir-853-r1
  $(Device/dlink_dir-8xx-r1)
  DEVICE_MODEL := DIR-853
  DEVICE_VARIANT := R1
  DEVICE_PACKAGES += kmod-usb3 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += dlink_dir-853-r1

define Device/dlink_dir-860l-b1
  $(Device/dsa-migration)
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
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport
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

define Device/dlink_dir-882-r1
  $(Device/dlink_dir-8xx-r1)
  DEVICE_MODEL := DIR-882
  DEVICE_VARIANT := R1
  DEVICE_PACKAGES += kmod-usb3 kmod-usb-ledtrig-usbport
  IMAGES += factory.bin
  IMAGE/factory.bin := append-kernel | append-rootfs | check-size | \
	sign-dlink-ru 57c5375741c30ca9ebcb36713db4ba51 \
	ab0dff19af8842cdb70a86b4b68d23f7
endef
TARGET_DEVICES += dlink_dir-882-r1

define Device/d-team_newifi-d2
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := Newifi
  DEVICE_MODEL := D2
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += d-team_newifi-d2

define Device/d-team_pbr-m1
  $(Device/dsa-migration)
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := PandoraBox
  DEVICE_MODEL := PBR-M1
  DEVICE_PACKAGES := kmod-ata-ahci kmod-mt7603 kmod-mt76x2 kmod-sdhci-mt7620 \
	kmod-usb3 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += pbr-m1
endef
TARGET_DEVICES += d-team_pbr-m1

define Device/edimax_ra21s
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Edimax
  DEVICE_MODEL := RA21S
  DEVICE_ALT0_VENDOR := Edimax
  DEVICE_ALT0_MODEL := Gemini RA21S
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | \
	elx-header 02020040 8844A2D168B45A2D
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += edimax_ra21s

define Device/edimax_re23s
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 15680k
  DEVICE_VENDOR := Edimax
  DEVICE_MODEL := RE23S
  DEVICE_ALT0_VENDOR := Edimax
  DEVICE_ALT0_MODEL := Gemini RE23S
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | \
	edimax-header -s CSYS -m RN76 -f 0x70000 -S 0x01100000 | pad-rootfs | \
	check-size | append-metadata
  IMAGES += factory.bin
  IMAGE/factory.bin := append-kernel | append-rootfs | \
	edimax-header -s CSYS -m RN76 -f 0x70000 -S 0x01100000 | pad-rootfs | \
	check-size
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += edimax_re23s

define Device/edimax_rg21s
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Edimax
  DEVICE_MODEL := Gemini AC2600 RG21S
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | \
	elx-header 02020038 8844A2D168B45A2D
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += edimax_rg21s

define Device/elecom_wrc-1167ghbk2-s
  $(Device/dsa-migration)
  IMAGE_SIZE := 15488k
  DEVICE_VENDOR := ELECOM
  DEVICE_MODEL := WRC-1167GHBK2-S
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | \
	elecom-wrc-gs-factory WRC-1167GHBK2-S 0.00
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += elecom_wrc-1167ghbk2-s

define Device/elecom_wrc-gs
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  DEVICE_VENDOR := ELECOM
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | \
	elecom-wrc-gs-factory $$$$(ELECOM_HWNAME) 0.00 -N | \
	append-string MT7621_ELECOM_$$$$(ELECOM_HWNAME)
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware
endef

define Device/elecom_wrc-1167gs2-b
  $(Device/elecom_wrc-gs)
  IMAGE_SIZE := 11264k
  DEVICE_MODEL := WRC-1167GS2-B
  ELECOM_HWNAME := WRC-1167GS2
endef
TARGET_DEVICES += elecom_wrc-1167gs2-b

define Device/elecom_wrc-1167gst2
  $(Device/elecom_wrc-gs)
  IMAGE_SIZE := 24576k
  DEVICE_MODEL := WRC-1167GST2
  ELECOM_HWNAME := WRC-1167GST2
endef
TARGET_DEVICES += elecom_wrc-1167gst2

define Device/elecom_wrc-1750gs
  $(Device/elecom_wrc-gs)
  IMAGE_SIZE := 11264k
  DEVICE_MODEL := WRC-1750GS
  ELECOM_HWNAME := WRC-1750GS
endef
TARGET_DEVICES += elecom_wrc-1750gs

define Device/elecom_wrc-1750gst2
  $(Device/elecom_wrc-gs)
  IMAGE_SIZE := 24576k
  DEVICE_MODEL := WRC-1750GST2
  ELECOM_HWNAME := WRC-1750GST2
endef
TARGET_DEVICES += elecom_wrc-1750gst2

define Device/elecom_wrc-1750gsv
  $(Device/elecom_wrc-gs)
  IMAGE_SIZE := 11264k
  DEVICE_MODEL := WRC-1750GSV
  ELECOM_HWNAME := WRC-1750GSV
endef
TARGET_DEVICES += elecom_wrc-1750gsv

define Device/elecom_wrc-1900gst
  $(Device/elecom_wrc-gs)
  IMAGE_SIZE := 11264k
  DEVICE_MODEL := WRC-1900GST
  ELECOM_HWNAME := WRC-1900GST
endef
TARGET_DEVICES += elecom_wrc-1900gst

define Device/elecom_wrc-2533ghbk-i
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  DEVICE_VENDOR := ELECOM
  DEVICE_MODEL := WRC-2533GHBK-I
  IMAGE_SIZE := 9856k
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | \
	elx-header 0107002d 8844A2D168B45A2D | \
	elecom-product-header WRC-2533GHBK-I
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += elecom_wrc-2533ghbk-i

define Device/elecom_wrc-2533gst
  $(Device/elecom_wrc-gs)
  IMAGE_SIZE := 11264k
  DEVICE_MODEL := WRC-2533GST
  ELECOM_HWNAME := WRC-2533GST
endef
TARGET_DEVICES += elecom_wrc-2533gst

define Device/elecom_wrc-2533gst2
  $(Device/elecom_wrc-gs)
  IMAGE_SIZE := 24576k
  DEVICE_MODEL := WRC-2533GST2
  ELECOM_HWNAME := WRC-2533GST2
endef
TARGET_DEVICES += elecom_wrc-2533gst2

define Device/firefly_firewrt
  $(Device/dsa-migration)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Firefly
  DEVICE_MODEL := FireWRT
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += firewrt
endef
TARGET_DEVICES += firefly_firewrt

define Device/gehua_ghl-r-001
  $(Device/dsa-migration)
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := GeHua
  DEVICE_MODEL := GHL-R-001
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += gehua_ghl-r-001

define Device/glinet_gl-mt1300
  $(Device/dsa-migration)
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := GL-MT1300
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware kmod-usb3
endef
TARGET_DEVICES += glinet_gl-mt1300

define Device/gnubee_gb-pc1
  $(Device/dsa-migration)
  DEVICE_VENDOR := GnuBee
  DEVICE_MODEL := Personal Cloud One
  DEVICE_PACKAGES := kmod-ata-ahci kmod-usb3 kmod-sdhci-mt7620 -wpad-openssl
  IMAGE_SIZE := 32448k
endef
TARGET_DEVICES += gnubee_gb-pc1

define Device/gnubee_gb-pc2
  $(Device/dsa-migration)
  DEVICE_VENDOR := GnuBee
  DEVICE_MODEL := Personal Cloud Two
  DEVICE_PACKAGES := kmod-ata-ahci kmod-usb3 kmod-sdhci-mt7620 -wpad-openssl
  IMAGE_SIZE := 32448k
endef
TARGET_DEVICES += gnubee_gb-pc2

define Device/hiwifi_hc5962
  $(Device/dsa-migration)
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
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3
endef
TARGET_DEVICES += hiwifi_hc5962

define Device/iodata_wn-ax1167gr
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 15552k
  KERNEL_INITRAMFS := $$(KERNEL) | \
	iodata-factory 7864320 4 0x1055 $(KDIR)/tmp/$$(KERNEL_INITRAMFS_PREFIX)-factory.bin
  DEVICE_VENDOR := I-O DATA
  DEVICE_MODEL := WN-AX1167GR
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2
endef
TARGET_DEVICES += iodata_wn-ax1167gr

define Device/iodata_nand
  $(Device/dsa-migration)
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

# The OEM webinterface expects an kernel with initramfs which has the uImage
# header field ih_name.
# We don't want to set the header name field for the kernel include in the
# sysupgrade image as well, as this image shouldn't be accepted by the OEM
# webinterface. It will soft-brick the board.

define Device/iodata_wn-ax1167gr2
  $(Device/iodata_nand)
  DEVICE_MODEL := WN-AX1167GR2
  KERNEL_INITRAMFS := $(KERNEL_DTB) | loader-kernel | lzma | \
	uImage lzma -M 0x434f4d42 -n '3.10(XBC.1)b10' | iodata-mstc-header
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += iodata_wn-ax1167gr2

define Device/iodata_wn-ax2033gr
  $(Device/iodata_nand)
  DEVICE_MODEL := WN-AX2033GR
  KERNEL_INITRAMFS := $(KERNEL_DTB) | loader-kernel | lzma | \
	uImage lzma -M 0x434f4d42 -n '3.10(VST.1)C10' | iodata-mstc-header
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += iodata_wn-ax2033gr

define Device/iodata_wn-dx1167r
  $(Device/iodata_nand)
  DEVICE_MODEL := WN-DX1167R
  KERNEL_INITRAMFS := $(KERNEL_DTB) | loader-kernel | lzma | \
	uImage lzma -M 0x434f4d43 -n '3.10(XIK.1)b10' | iodata-mstc-header
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += iodata_wn-dx1167r

define Device/iodata_wn-dx1200gr
  $(Device/iodata_nand)
  DEVICE_MODEL := WN-DX1200GR
  KERNEL_INITRAMFS := $(KERNEL_DTB) | loader-kernel | lzma | \
	uImage lzma -M 0x434f4d43 -n '3.10(XIQ.0)b20' | iodata-mstc-header
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt7615e kmod-mt7663-firmware-ap
endef
TARGET_DEVICES += iodata_wn-dx1200gr

define Device/iodata_wn-gx300gr
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 7616k
  DEVICE_VENDOR := I-O DATA
  DEVICE_MODEL := WN-GX300GR
  DEVICE_PACKAGES := kmod-mt7603
endef
TARGET_DEVICES += iodata_wn-gx300gr

define Device/iodata_wnpr2600g
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  DEVICE_VENDOR := I-O DATA
  DEVICE_MODEL := WNPR2600G
  IMAGE_SIZE := 13952k
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | \
	elx-header 0104003a 8844A2D168B45A2D
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += iodata_wnpr2600g

define Device/iptime_a6ns-m
  $(Device/dsa-migration)
  IMAGE_SIZE := 16128k
  UIMAGE_NAME := a6nm
  DEVICE_VENDOR := ipTIME
  DEVICE_MODEL := A6ns-M
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware kmod-usb3 \
	kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += iptime_a6ns-m

define Device/iptime_a8004t
  $(Device/dsa-migration)
  IMAGE_SIZE := 16128k
  UIMAGE_NAME := a8004t
  DEVICE_VENDOR := ipTIME
  DEVICE_MODEL := A8004T
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware kmod-usb3
endef
TARGET_DEVICES += iptime_a8004t

define Device/jcg_jhr-ac876m
  $(Device/dsa-migration)
  IMAGE_SIZE := 16064k
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | jcg-header 89.1
  JCG_MAXSIZE := 16064k
  DEVICE_VENDOR := JCG
  DEVICE_MODEL := JHR-AC876M
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware kmod-usb3 \
	kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += jcg_jhr-ac876m

define Device/jcg_q20
  $(Device/dsa-migration)
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  UBINIZE_OPTS := -E 5
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 91136k
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/factory.bin := append-kernel | pad-to $$(KERNEL_SIZE) | append-ubi | \
	check-size
  DEVICE_VENDOR := JCG
  DEVICE_MODEL := Q20
  DEVICE_PACKAGES := kmod-mt7915e uboot-envtools
endef
TARGET_DEVICES += jcg_q20

define Device/jcg_y2
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 16064k
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | jcg-header 95.1
  JCG_MAXSIZE := 16064k
  DEVICE_VENDOR := JCG
  DEVICE_MODEL := Y2
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware kmod-usb3
endef
TARGET_DEVICES += jcg_y2

define Device/lenovo_newifi-d1
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := Newifi
  DEVICE_MODEL := D1
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-sdhci-mt7620 \
	kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += newifi-d1
endef
TARGET_DEVICES += lenovo_newifi-d1

define Device/linksys_e5600
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 26624k
  DEVICE_VENDOR := Linksys
  DEVICE_MODEL := E5600
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt7615e kmod-mt7663-firmware-ap \
	kmod-mt7663-firmware-sta uboot-envtools
  UBINIZE_OPTS := -E 5
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | check-size | append-metadata
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | \
	append-ubi | check-size | gemtek-trailer
endef
TARGET_DEVICES += linksys_e5600

define Device/linksys_ea7xxx
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 36864k
  DEVICE_VENDOR := Linksys
  DEVICE_PACKAGES := kmod-usb3 kmod-mt7615e kmod-mt7615-firmware \
	uboot-envtools
  UBINIZE_OPTS := -E 5
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | check-size | append-metadata
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | \
	append-ubi | check-size | linksys-image type=$$$$(LINKSYS_HWNAME)
endef

define Device/linksys_ea7300-v1
  $(Device/linksys_ea7xxx)
  DEVICE_MODEL := EA7300
  DEVICE_VARIANT := v1
  LINKSYS_HWNAME := EA7300
endef
TARGET_DEVICES += linksys_ea7300-v1

define Device/linksys_ea7300-v2
  $(Device/linksys_ea7xxx)
  DEVICE_MODEL := EA7300
  DEVICE_VARIANT := v2
  LINKSYS_HWNAME := EA7300v2
  DEVICE_PACKAGES += kmod-mt7603
endef
TARGET_DEVICES += linksys_ea7300-v2

define Device/linksys_ea7500-v2
  $(Device/linksys_ea7xxx)
  DEVICE_MODEL := EA7500
  DEVICE_VARIANT := v2
  LINKSYS_HWNAME := EA7500v2
endef
TARGET_DEVICES += linksys_ea7500-v2

define Device/linksys_ea8100-v1
  $(Device/linksys_ea7xxx)
  DEVICE_MODEL := EA8100
  DEVICE_VARIANT := v1
  LINKSYS_HWNAME := EA8100
endef
TARGET_DEVICES += linksys_ea8100-v1

define Device/linksys_ea8100-v2
  $(Device/linksys_ea7xxx)
  DEVICE_MODEL := EA8100
  DEVICE_VARIANT := v2
  LINKSYS_HWNAME := EA8100v2
endef
TARGET_DEVICES += linksys_ea8100-v2

define Device/linksys_re6500
  $(Device/dsa-migration)
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Linksys
  DEVICE_MODEL := RE6500
  DEVICE_PACKAGES := kmod-mt76x2
  SUPPORTED_DEVICES += re6500
endef
TARGET_DEVICES += linksys_re6500

define Device/mediatek_ap-mt7621a-v60
  $(Device/dsa-migration)
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Mediatek
  DEVICE_MODEL := AP-MT7621A-V60 EVB
  DEVICE_PACKAGES := kmod-usb3 kmod-sdhci-mt7620 kmod-sound-mt7620 -wpad-openssl
endef
TARGET_DEVICES += mediatek_ap-mt7621a-v60

define Device/mediatek_mt7621-eval-board
  $(Device/dsa-migration)
  BLOCKSIZE := 64k
  IMAGE_SIZE := 15104k
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MT7621 EVB
  DEVICE_PACKAGES := -wpad-openssl
  SUPPORTED_DEVICES += mt7621
endef
TARGET_DEVICES += mediatek_mt7621-eval-board

define Device/MikroTik
  $(Device/dsa-migration)
  DEVICE_VENDOR := MikroTik
  BLOCKSIZE := 64k
  IMAGE_SIZE := 16128k
  DEVICE_PACKAGES := kmod-usb3
  KERNEL_NAME := vmlinuz
  KERNEL := kernel-bin | append-dtb-elf
  IMAGE/sysupgrade.bin := append-kernel | kernel2minor -s 1024 | \
	pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | check-size | \
	append-metadata
endef

define Device/mikrotik_routerboard-750gr3
  $(Device/MikroTik)
  DEVICE_MODEL := RouterBOARD 750Gr3
  DEVICE_PACKAGES += -wpad-openssl
  SUPPORTED_DEVICES += mikrotik,rb750gr3
endef
TARGET_DEVICES += mikrotik_routerboard-750gr3

define Device/mikrotik_routerboard-760igs
  $(Device/MikroTik)
  DEVICE_MODEL := RouterBOARD 760iGS
  DEVICE_PACKAGES += kmod-sfp -wpad-openssl
endef
TARGET_DEVICES += mikrotik_routerboard-760igs

define Device/mikrotik_routerboard-m11g
  $(Device/MikroTik)
  DEVICE_MODEL := RouterBOARD M11G
  DEVICE_PACKAGES := -wpad-openssl
  SUPPORTED_DEVICES += mikrotik,rbm11g
endef
TARGET_DEVICES += mikrotik_routerboard-m11g

define Device/mikrotik_routerboard-m33g
  $(Device/MikroTik)
  DEVICE_MODEL := RouterBOARD M33G
  DEVICE_PACKAGES := -wpad-openssl
  SUPPORTED_DEVICES += mikrotik,rbm33g
endef
TARGET_DEVICES += mikrotik_routerboard-m33g

define Device/mqmaker_witi
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := MQmaker
  DEVICE_MODEL := WiTi
  DEVICE_PACKAGES := kmod-ata-ahci kmod-mt76x2 kmod-sdhci-mt7620 kmod-usb3 \
	kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += witi mqmaker,witi-256m mqmaker,witi-512m
endef
TARGET_DEVICES += mqmaker_witi

define Device/mtc_wr1201
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 16000k
  DEVICE_VENDOR := MTC
  DEVICE_MODEL := Wireless Router WR1201
  KERNEL_INITRAMFS := $(KERNEL_DTB) | uImage lzma -n 'WR1201_8_128'
  DEVICE_PACKAGES := kmod-sdhci-mt7620 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += mtc_wr1201

define Device/netgear_ex6150
  $(Device/dsa-migration)
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := EX6150
  DEVICE_PACKAGES := kmod-mt76x2
  NETGEAR_BOARD_ID := U12H318T00_NETGEAR
  IMAGE_SIZE := 14848k
  IMAGES += factory.chk
  IMAGE/factory.chk := $$(sysupgrade_bin) | check-size | netgear-chk
endef
TARGET_DEVICES += netgear_ex6150

define Device/netgear_sercomm_nand
  $(Device/dsa-migration)
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
  DEVICE_PACKAGES := kmod-mt7603 kmod-usb3 kmod-usb-ledtrig-usbport
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
  DEVICE_PACKAGES += kmod-mt7615e kmod-mt7615-firmware
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
  $(Device/dsa-migration)
  $(Device/netgear_sercomm_nor)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 15232k
  DEVICE_MODEL := WNDR3700
  DEVICE_VARIANT := v5
  SERCOMM_HWNAME := WNDR3700v5
  SERCOMM_HWID := AYB
  SERCOMM_HWVER := A001
  SERCOMM_SWVER := 0x1054
  SERCOMM_PAD := 320k
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += wndr3700v5
endef
TARGET_DEVICES += netgear_wndr3700-v5

define Device/netis_wf2881
  $(Device/dsa-migration)
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
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += netis_wf2881

define Device/phicomm_k2p
  $(Device/dsa-migration)
  IMAGE_SIZE := 15744k
  DEVICE_VENDOR := Phicomm
  DEVICE_MODEL := K2P
  SUPPORTED_DEVICES += k2p
  DEVICE_PACKAGES := -luci-newapi -wpad-openssl kmod-mt7615d_dbdc luci-app-adbyby-plus xray-core UnblockNeteaseMusicGo
endef
TARGET_DEVICES += phicomm_k2p

define Device/planex_vr500
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 65216k
  DEVICE_VENDOR := Planex
  DEVICE_MODEL := VR500
  DEVICE_PACKAGES := kmod-usb3 -wpad-openssl
  SUPPORTED_DEVICES += vr500
endef
TARGET_DEVICES += planex_vr500

define Device/samknows_whitebox-v8
  $(Device/dsa-migration)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := SamKnows
  DEVICE_MODEL := Whitebox 8
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport uboot-envtools
  SUPPORTED_DEVICES += sk-wb8
endef
TARGET_DEVICES += samknows_whitebox-v8

define Device/sercomm_na502
  $(Device/uimage-lzma-loader)
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 20480k
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  UBINIZE_OPTS := -E 5
  KERNEL_SIZE := 4096k
  DEVICE_VENDOR := SERCOMM
  DEVICE_MODEL := NA502
  DEVICE_PACKAGES := kmod-mt76x2 kmod-mt7603 kmod-usb3
endef
TARGET_DEVICES += sercomm_na502

define Device/storylink_sap-g3200u3
  $(Device/dsa-migration)
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := STORYLiNK
  DEVICE_MODEL := SAP-G3200U3
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb3 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += sap-g3200u3
endef
TARGET_DEVICES += storylink_sap-g3200u3

define Device/telco-electronics_x1
  $(Device/dsa-migration)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Telco Electronics
  DEVICE_MODEL := X1
  DEVICE_PACKAGES := kmod-usb3 kmod-mt76
endef
TARGET_DEVICES += telco-electronics_x1

define Device/tenbay_t-mb5eu-v01
  $(Device/dsa-migration)
  DEVICE_VENDOR := Tenbay
  DEVICE_MODEL := T-MB5EU-V01
  DEVICE_DTS_CONFIG := config@1
  DEVICE_PACKAGES += kmod-mt7915e kmod-usb3
  KERNEL := kernel-bin | lzma | fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb
  IMAGE_SIZE := 15808k
  SUPPORTED_DEVICES += mt7621-dm2-t-mb5eu-v01-nor
endef
TARGET_DEVICES += tenbay_t-mb5eu-v01

define Device/thunder_timecloud
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Thunder
  DEVICE_MODEL := Timecloud
  DEVICE_PACKAGES := kmod-usb3 -wpad-openssl
  SUPPORTED_DEVICES += timecloud
endef
TARGET_DEVICES += thunder_timecloud

define Device/totolink_a7000r
  $(Device/dsa-migration)
  IMAGE_SIZE := 16064k
  UIMAGE_NAME := C8340R1C-9999
  DEVICE_VENDOR := TOTOLINK
  DEVICE_MODEL := A7000R
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += totolink_a7000r

define Device/totolink_x5000r
  $(Device/dsa-migration)
  IMAGE_SIZE := 16064k
  UIMAGE_NAME := C8343R-9999
  DEVICE_VENDOR := TOTOLINK
  DEVICE_MODEL := X5000R
  DEVICE_PACKAGES := kmod-mt7915e
endef
TARGET_DEVICES += totolink_x5000r

define Device/tplink_archer-a6-v3
  $(Device/dsa-migration)
  $(Device/tplink-safeloader)
  DEVICE_MODEL := Archer A6
  DEVICE_VARIANT := V3
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt7615e \
	kmod-mt7663-firmware-ap kmod-mt7663-firmware-sta
  TPLINK_BOARD_ID := ARCHER-A6-V3
  KERNEL := $(KERNEL_DTB) | uImage lzma
  IMAGE_SIZE := 15744k
endef
TARGET_DEVICES += tplink_archer-a6-v3

define Device/tplink_archer-c6-v3
  $(Device/dsa-migration)
  $(Device/tplink-safeloader)
  DEVICE_MODEL := Archer C6
  DEVICE_VARIANT := V3
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt7615e \
	kmod-mt7663-firmware-ap kmod-mt7663-firmware-sta
  TPLINK_BOARD_ID := ARCHER-C6-V3
  KERNEL := $(KERNEL_DTB) | uImage lzma
  IMAGE_SIZE := 15744k
endef
TARGET_DEVICES += tplink_archer-c6-v3

define Device/tplink_archer-c6u-v1
  $(Device/dsa-migration)
  $(Device/tplink-safeloader)
  DEVICE_MODEL := Archer C6U
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-mt7603 \
	kmod-mt7615e kmod-mt7663-firmware-ap \
	kmod-usb3 kmod-usb-ledtrig-usbport
  KERNEL := $(KERNEL_DTB) | uImage lzma
  TPLINK_BOARD_ID := ARCHER-C6U-V1
  IMAGE_SIZE := 15744k
endef
TARGET_DEVICES += tplink_archer-c6u-v1

define Device/tplink_eap235-wall-v1
  $(Device/dsa-migration)
  $(Device/tplink-safeloader)
  DEVICE_MODEL := EAP235-Wall
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt7615e kmod-mt7663-firmware-ap
  TPLINK_BOARD_ID := EAP235-WALL-V1
  IMAGE_SIZE := 13440k
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory | \
	pad-extra 128
endef
TARGET_DEVICES += tplink_eap235-wall-v1

define Device/tplink_re350-v1
  $(Device/dsa-migration)
  $(Device/tplink-safeloader)
  DEVICE_MODEL := RE350
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2
  TPLINK_BOARD_ID := RE350-V1
  IMAGE_SIZE := 6016k
  SUPPORTED_DEVICES += re350-v1
endef
TARGET_DEVICES += tplink_re350-v1

define Device/tplink_re500-v1
  $(Device/dsa-migration)
  $(Device/tplink-safeloader)
  DEVICE_MODEL := RE500
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware
  TPLINK_BOARD_ID := RE500-V1
  IMAGE_SIZE := 14208k
endef
TARGET_DEVICES += tplink_re500-v1

define Device/tplink_re650-v1
  $(Device/dsa-migration)
  $(Device/tplink-safeloader)
  DEVICE_MODEL := RE650
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware
  TPLINK_BOARD_ID := RE650-V1
  IMAGE_SIZE := 14208k
endef
TARGET_DEVICES += tplink_re650-v1

define Device/ubnt_edgerouter_common
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  DEVICE_VENDOR := Ubiquiti
  IMAGE_SIZE := 256768k
  FILESYSTEMS := squashfs
  KERNEL_SIZE := 3145728
  KERNEL_INITRAMFS := $$(KERNEL) | \
	ubnt-erx-factory-image $(KDIR)/tmp/$$(KERNEL_INITRAMFS_PREFIX)-factory.tar
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  DEVICE_PACKAGES += -wpad-openssl
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

define Device/ubnt_unifi-6-lite
  $(Device/dsa-migration)
  DEVICE_VENDOR := Ubiquiti
  DEVICE_MODEL := UniFi 6 Lite
  DEVICE_DTS_CONFIG := config@1
  DEVICE_PACKAGES += kmod-mt7603 kmod-mt7915e
  KERNEL := kernel-bin | lzma | fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb
  IMAGE_SIZE := 15424k
endef
TARGET_DEVICES += ubnt_unifi-6-lite

define Device/ubnt_unifi-nanohd
  $(Device/dsa-migration)
  DEVICE_VENDOR := Ubiquiti
  DEVICE_MODEL := UniFi nanoHD
  DEVICE_PACKAGES += kmod-mt7603 kmod-mt7615e kmod-mt7615-firmware
  IMAGE_SIZE := 15552k
endef
TARGET_DEVICES += ubnt_unifi-nanohd

define Device/unielec_u7621-01-16m
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := UniElec
  DEVICE_MODEL := U7621-01
  DEVICE_VARIANT := 16M
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3
endef
TARGET_DEVICES += unielec_u7621-01-16m

define Device/unielec_u7621-06-16m
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := UniElec
  DEVICE_MODEL := U7621-06
  DEVICE_VARIANT := 16M
  DEVICE_PACKAGES := kmod-ata-ahci kmod-sdhci-mt7620 kmod-usb3 -wpad-openssl
  SUPPORTED_DEVICES += u7621-06-256M-16M unielec,u7621-06-256m-16m
endef
TARGET_DEVICES += unielec_u7621-06-16m

define Device/unielec_u7621-06-64m
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 65216k
  DEVICE_VENDOR := UniElec
  DEVICE_MODEL := U7621-06
  DEVICE_VARIANT := 64M
  DEVICE_PACKAGES := kmod-ata-ahci kmod-sdhci-mt7620 kmod-usb3 -wpad-openssl
  SUPPORTED_DEVICES += unielec,u7621-06-512m-64m
endef
TARGET_DEVICES += unielec_u7621-06-64m

define Device/wavlink_wl-wn531a6
  $(Device/dsa-migration)
  DEVICE_VENDOR := Wavlink
  DEVICE_MODEL := WL-WN531A6
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt7615e kmod-mt7615-firmware kmod-usb3
  IMAGE_SIZE := 15040k
endef
TARGET_DEVICES += wavlink_wl-wn531a6

define Device/wevo_11acnas
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 16064k
  UIMAGE_NAME := 11AC-NAS-Router(0.0.0)
  DEVICE_VENDOR := WeVO
  DEVICE_MODEL := 11AC NAS Router
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += 11acnas
endef
TARGET_DEVICES += wevo_11acnas

define Device/wevo_w2914ns-v2
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 16064k
  UIMAGE_NAME := W2914NS-V2(0.0.0)
  DEVICE_VENDOR := WeVO
  DEVICE_MODEL := W2914NS
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += w2914nsv2
endef
TARGET_DEVICES += wevo_w2914ns-v2

define Device/winstars_ws-wn583a6
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Winstars
  DEVICE_MODEL := WS-WN583A6
  DEVICE_ALT0_VENDOR := Gemeita
  DEVICE_ALT0_MODEL := AC2100
  KERNEL_INITRAMFS_SUFFIX := -WN583A6$$(KERNEL_SUFFIX)
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += winstars_ws-wn583a6

define Device/xiaomi_nand_separate
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  DEVICE_VENDOR := Xiaomi
  DEVICE_PACKAGES := uboot-envtools
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  UBINIZE_OPTS := -E 5
  IMAGES += kernel1.bin rootfs0.bin
  IMAGE/kernel1.bin := append-kernel
  IMAGE/rootfs0.bin := append-ubi | check-size
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef

define Device/xiaomi_mi-router-3g
  $(Device/xiaomi_nand_separate)
  DEVICE_MODEL := Mi Router 3G
  IMAGE_SIZE := 124416k
  DEVICE_PACKAGES += kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += R3G mir3g xiaomi,mir3g
endef
TARGET_DEVICES += xiaomi_mi-router-3g

define Device/xiaomi_mi-router-3g-v2
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 14848k
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Mi Router 3G
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2
  SUPPORTED_DEVICES += xiaomi,mir3g-v2
endef
TARGET_DEVICES += xiaomi_mi-router-3g-v2

define Device/xiaomi_mi-router-3-pro
  $(Device/dsa-migration)
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
  DEVICE_PACKAGES := kmod-mt7615e kmod-mt7615-firmware kmod-usb3 \
	kmod-usb-ledtrig-usbport uboot-envtools
  SUPPORTED_DEVICES += xiaomi,mir3p
endef
TARGET_DEVICES += xiaomi_mi-router-3-pro

define Device/xiaomi_mi-router-4
  $(Device/xiaomi_nand_separate)
  DEVICE_MODEL := Mi Router 4
  IMAGE_SIZE := 124416k
  DEVICE_PACKAGES += kmod-mt7603 kmod-mt76x2
endef
TARGET_DEVICES += xiaomi_mi-router-4

define Device/xiaomi_mi-router-4a-gigabit
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 14848k
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Mi Router 4A
  DEVICE_VARIANT := Gigabit Edition
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2
endef
TARGET_DEVICES += xiaomi_mi-router-4a-gigabit

define Device/xiaomi_mi-router-ac2100
  $(Device/xiaomi_nand_separate)
  DEVICE_MODEL := Mi Router AC2100
  IMAGE_SIZE := 120320k
  DEVICE_PACKAGES += kmod-mt7603 kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += xiaomi_mi-router-ac2100

define Device/xiaomi_mi-router-cr6606
  $(Device/uimage-lzma-loader)
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  UBINIZE_OPTS := -E 5
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 128512k
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/factory.bin := append-kernel | pad-to $$(KERNEL_SIZE) | append-ubi | check-size
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Mi Router CR6606/CR6608/CR6609
  DEVICE_PACKAGES += kmod-mt7915e wpad-openssl uboot-envtools
endef
TARGET_DEVICES += xiaomi_mi-router-cr6606

define Device/xiaomi_redmi-router-ac2100
  $(Device/xiaomi_nand_separate)
  DEVICE_MODEL := Redmi Router AC2100
  IMAGE_SIZE := 120320k
  DEVICE_PACKAGES += kmod-mt7603 kmod-mt7615e kmod-mt7615-firmware
endef
TARGET_DEVICES += xiaomi_redmi-router-ac2100

define Device/xiaoyu_xy-c5
  $(Device/dsa-migration)
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := XiaoYu
  DEVICE_MODEL := XY-C5
  DEVICE_PACKAGES := kmod-ata-ahci kmod-usb3 -wpad-openssl
endef
TARGET_DEVICES += xiaoyu_xy-c5

define Device/xzwifi_creativebox-v1
  $(Device/dsa-migration)
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := CreativeBox
  DEVICE_MODEL := v1
  DEVICE_PACKAGES := kmod-ata-ahci kmod-mt7603 kmod-mt76x2 kmod-sdhci-mt7620 \
	kmod-usb3 -wpad-openssl
endef
TARGET_DEVICES += xzwifi_creativebox-v1

define Device/youhua_wr1200js
  $(Device/dsa-migration)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := YouHua
  DEVICE_MODEL := WR1200JS
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += youhua_wr1200js

define Device/youku_yk-l2
  $(Device/dsa-migration)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Youku
  DEVICE_MODEL := YK-L2
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += youku_yk-l2

define Device/zbtlink_zbt-we1326
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-WE1326
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 kmod-sdhci-mt7620
  SUPPORTED_DEVICES += zbt-we1326
endef
TARGET_DEVICES += zbtlink_zbt-we1326

define Device/zbtlink_zbt-we3526
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-WE3526
  DEVICE_PACKAGES := kmod-sdhci-mt7620 kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += zbtlink_zbt-we3526

define Device/zbtlink_zbt-wg2626
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-WG2626
  DEVICE_PACKAGES := kmod-ata-ahci kmod-sdhci-mt7620 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += zbt-wg2626
endef
TARGET_DEVICES += zbtlink_zbt-wg2626

define Device/zbtlink_zbt-wg3526-16m
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-WG3526
  DEVICE_VARIANT := 16M
  DEVICE_PACKAGES := kmod-ata-ahci kmod-sdhci-mt7620 kmod-mt7603 kmod-mt76x2 \
	kmod-usb3 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += zbt-wg3526 zbt-wg3526-16M
endef
TARGET_DEVICES += zbtlink_zbt-wg3526-16m

define Device/zbtlink_zbt-wg3526-32m
  $(Device/dsa-migration)
  $(Device/uimage-lzma-loader)
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-WG3526
  DEVICE_VARIANT := 32M
  DEVICE_PACKAGES := kmod-ata-ahci kmod-sdhci-mt7620 kmod-mt7603 kmod-mt76x2 \
	kmod-usb3 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += ac1200pro zbt-wg3526-32M
endef
TARGET_DEVICES += zbtlink_zbt-wg3526-32m

define Device/zio_freezio
  $(Device/dsa-migration)
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := ZIO
  DEVICE_MODEL := FREEZIO
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt76x2 kmod-usb3 \
	kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += zio_freezio

define Device/zyxel_nr7101
  $(Device/dsa-migration)
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  UBINIZE_OPTS := -E 5
  DEVICE_VENDOR := ZyXEL
  DEVICE_MODEL := NR7101
  DEVICE_PACKAGES := kmod-mt7603 kmod-usb3 uboot-envtools kmod-usb-net-qmi-wwan kmod-usb-serial-option uqmi
  KERNEL := $(KERNEL_DTB) | uImage lzma | zytrx-header $$(DEVICE_MODEL) $$(VERSION_DIST)-$$(REVISION)
  KERNEL_INITRAMFS := $(KERNEL_DTB) | uImage lzma | zytrx-header $$(DEVICE_MODEL) 9.99(ABUV.9)$$(VERSION_DIST)-recovery
  KERNEL_INITRAMFS_SUFFIX := -recovery.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += zyxel_nr7101

define Device/zyxel_wap6805
  $(Device/dsa-migration)
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4096k
  UBINIZE_OPTS := -E 5
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := ZyXEL
  DEVICE_MODEL := WAP6805
  DEVICE_PACKAGES := kmod-mt7603 kmod-mt7621-qtn-rgmii
  KERNEL := $(KERNEL_DTB) | uImage lzma | uimage-padhdr 160
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += zyxel_wap6805
