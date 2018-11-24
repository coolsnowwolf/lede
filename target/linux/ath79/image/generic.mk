include ./common-buffalo.mk
include ./common-netgear.mk

DEVICE_VARS += ADDPATTERN_ID ADDPATTERN_VERSION

define Build/cybertan-trx
	@echo -n '' > $@-empty.bin
	-$(STAGING_DIR_HOST)/bin/trx -o $@.new \
		-f $(IMAGE_KERNEL) -F $@-empty.bin \
		-x 32 -a 0x10000 -x -32 -f $@
	-mv "$@.new" "$@"
	-rm $@-empty.bin
endef

define Build/addpattern
	-$(STAGING_DIR_HOST)/bin/addpattern -B $(ADDPATTERN_ID) \
		-v v$(ADDPATTERN_VERSION) -i $@ -o $@.new
	-mv "$@.new" "$@"
endef

define Build/elecom-header
  $(eval fw_size=$(word 1,$(1)))
  $(eval edimax_model=$(word 2,$(1)))
  $(eval product=$(word 3,$(1)))
  $(eval factory_bin=$(word 4,$(1)))
  if [ -e $(KDIR)/tmp/$(KERNEL_INITRAMFS_IMAGE) -a "$$(stat -c%s $@)" -lt "$(fw_size)" ]; then \
    $(CP) $(KDIR)/tmp/$(KERNEL_INITRAMFS_IMAGE) $(factory_bin); \
    $(STAGING_DIR_HOST)/bin/mkedimaximg \
      -b -s CSYS -m $(edimax_model) \
      -f 0x70000 -S 0x01100000 \
      -i $(factory_bin) -o $(factory_bin).new; \
    mv $(factory_bin).new $(factory_bin); \
    ( \
      echo -n -e "ELECOM\x00\x00$(product)" | dd bs=40 count=1 conv=sync; \
      echo -n "0.00" | dd bs=16 count=1 conv=sync; \
      dd if=$(factory_bin); \
    ) > $(factory_bin).new; \
    mv $(factory_bin).new $(factory_bin); \
    $(CP) $(factory_bin) $(BIN_DIR)/; \
	else \
		echo "WARNING: initramfs kernel image too big, cannot generate factory image" >&2; \
	fi

endef

define Device/avm_fritz300e
  ATH_SOC := ar7242
  DEVICE_TITLE := AVM FRITZ!WLAN Repeater 300E
  KERNEL := kernel-bin | append-dtb | lzma | eva-image
  KERNEL_INITRAMFS := $$(KERNEL)
  IMAGE_SIZE := 15232k
  IMAGE/sysupgrade.bin := append-kernel | pad-to 64k | \
	append-squashfs-fakeroot-be | pad-to 256 | \
	append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_PACKAGES := fritz-tffs rssileds -swconfig
endef
TARGET_DEVICES += avm_fritz300e

define Device/avm_fritz4020
  ATH_SOC := qca9561
  DEVICE_TITLE := AVM FRITZ!Box 4020
  IMAGE_SIZE := 15232k
  KERNEL := kernel-bin | append-dtb | lzma | eva-image
  KERNEL_INITRAMFS := $$(KERNEL)
  IMAGE/sysupgrade.bin := append-kernel | pad-to 64k | \
      append-squashfs-fakeroot-be | pad-to 256 | \
      append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_PACKAGES := fritz-tffs
endef
TARGET_DEVICES += avm_fritz4020

define Device/buffalo_bhr-4grv
  ATH_SOC := ar7242
  DEVICE_TITLE := Buffalo BHR-4GRV
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  IMAGE_SIZE := 32256k
  IMAGES += factory.bin tftp.bin
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := $$(IMAGE/default) | buffalo-enc BHR-4GRV 1.99 | buffalo-tag BHR-4GRV 3
  IMAGE/tftp.bin := $$(IMAGE/default) | buffalo-tftp-header
  SUPPORTED_DEVICES += wzr-hp-g450h
endef
TARGET_DEVICES += buffalo_bhr-4grv

define Device/buffalo_wzr-hp-ag300h
  ATH_SOC := ar7161
  DEVICE_TITLE := Buffalo WZR-HP-AG300H
  IMAGE_SIZE := 32256k
  IMAGES += factory.bin tftp.bin
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := $$(IMAGE/default) | buffalo-enc WZR-HP-AG300H 1.99 | buffalo-tag WZR-HP-AG300H 3
  IMAGE/tftp.bin := $$(IMAGE/default) | buffalo-tftp-header
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport kmod-leds-reset kmod-owl-loader
  SUPPORTED_DEVICES += wzr-hp-ag300h
endef
TARGET_DEVICES += buffalo_wzr-hp-ag300h

define Device/buffalo_wzr-hp-g302h-a1a0
  ATH_SOC := ar7242
  DEVICE_TITLE := Buffalo WZR-HP-G302H A1A0
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  IMAGE_SIZE := 32128k
  IMAGES += factory.bin tftp.bin
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := $$(IMAGE/default) | buffalo-enc WZR-HP-G302H 1.99 | buffalo-tag WZR-HP-G302H 4
  IMAGE/tftp.bin := $$(IMAGE/default) | buffalo-tftp-header
  SUPPORTED_DEVICES += wzr-hp-g300nh2
endef
TARGET_DEVICES += buffalo_wzr-hp-g302h-a1a0

define Device/buffalo_wzr-hp-g450h
  ATH_SOC := ar7242
  DEVICE_TITLE := Buffalo WZR-HP-G450H/WZR-450HP
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  IMAGE_SIZE := 32256k
  IMAGES += factory.bin tftp.bin
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := $$(IMAGE/default) | buffalo-enc WZR-HP-G450H 1.99 | buffalo-tag WZR-HP-G450H 3
  IMAGE/tftp.bin := $$(IMAGE/default) | buffalo-tftp-header
  SUPPORTED_DEVICES += wzr-hp-g450h
endef
TARGET_DEVICES += buffalo_wzr-hp-g450h

define Device/dlink_dir-825-b1
  ATH_SOC := ar7161
  DEVICE_TITLE := D-LINK DIR-825 B1
  IMAGE_SIZE := 6208k
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport kmod-leds-reset kmod-owl-loader
  SUPPORTED_DEVICES += dir-825-b1
endef
TARGET_DEVICES += dlink_dir-825-b1

define Device/elecom_wrc-300ghbk2-i
  ATH_SOC := qca9563
  DEVICE_TITLE := ELECOM WRC-300GHBK2-I
  IMAGE_SIZE := 7616k
  KERNEL_INITRAMFS := $$(KERNEL) | pad-to 2 | \
    elecom-header 7798706 RN51 WRC-300GHBK2-I \
      $(KDIR)/tmp/$$(KERNEL_INITRAMFS_PREFIX)-factory.bin
endef
TARGET_DEVICES += elecom_wrc-300ghbk2-i

define Device/embeddedwireless_dorin
  ATH_SOC := ar9331
  DEVICE_TITLE := Embedded Wireless Dorin
  DEVICE_PACKAGES := kmod-usb-chipidea2
  IMAGE_SIZE := 16000k
endef
TARGET_DEVICES += embeddedwireless_dorin

define Device/etactica_eg200
  ATH_SOC := ar9331
  DEVICE_TITLE := eTactica EG200
  DEVICE_PACKAGES := kmod-usb-chipidea2 kmod-ledtrig-oneshot \
	kmod-usb-serial kmod-usb-serial-ftdi kmod-usb-storage  kmod-fs-ext4
  SUPPORTED_DEVICES += rme-eg200
endef
TARGET_DEVICES += etactica_eg200

define Device/glinet_ar150
  ATH_SOC := ar9330
  DEVICE_TITLE := GL.iNet GL-AR150
  DEVICE_PACKAGES := kmod-usb-chipidea2
  IMAGE_SIZE := 16000k
  SUPPORTED_DEVICES += gl-ar150
endef
TARGET_DEVICES += glinet_ar150

define Device/glinet_ar300m_nor
  ATH_SOC := qca9533
  DEVICE_TITLE := GL.iNet GL-AR300M
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  IMAGE_SIZE := 16000k
  SUPPORTED_DEVICES += gl-ar300m
endef
TARGET_DEVICES += glinet_ar300m_nor

define Device/iodata_etg3-r
  ATH_SOC := ar9342
  DEVICE_TITLE := I-O DATA ETG3-R
  IMAGE_SIZE := 7680k
endef
TARGET_DEVICES += iodata_etg3-r

define Device/iodata_wn-ac1167dgr
  ATH_SOC := qca9557
  DEVICE_TITLE := I-O DATA WN-AC1167DGR
  IMAGE_SIZE := 14656k
  IMAGES += factory.bin
  IMAGE/factory.bin := append-kernel | pad-to $$$$(BLOCKSIZE) | \
    append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE) | \
    senao-header -r 0x30a -p 0x61 -t 2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-ath10k-ct ath10k-firmware-qca988x-ct
endef
TARGET_DEVICES += iodata_wn-ac1167dgr

define Device/iodata_wn-ac1600dgr2
  ATH_SOC := qca9557
  DEVICE_TITLE := I-O DATA WN-AC1600DGR2
  IMAGE_SIZE := 14656k
  IMAGES += factory.bin
  IMAGE/factory.bin := append-kernel | pad-to $$$$(BLOCKSIZE) | \
    append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE) | \
    senao-header -r 0x30a -p 0x60 -t 2 -v 200
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-ath10k-ct ath10k-firmware-qca988x-ct
endef
TARGET_DEVICES += iodata_wn-ac1600dgr2

define Device/ocedo_koala
  ATH_SOC := qca9558
  DEVICE_TITLE := OCEDO Koala
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct
  SUPPORTED_DEVICES += koala
  IMAGE_SIZE := 7424k
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += ocedo_koala

define Device/ocedo_raccoon
  ATH_SOC := ar9344
  DEVICE_TITLE := OCEDO Raccoon
  IMAGE_SIZE := 7424k
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += ocedo_raccoon

define Device/openmesh_om5p-ac-v2
  ATH_SOC := qca9558
  DEVICE_TITLE := OpenMesh OM5P-AC v2
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct om-watchdog
  IMAGE_SIZE := 7808k
  SUPPORTED_DEVICES += om5p-acv2
endef
TARGET_DEVICES += openmesh_om5p-ac-v2

define Device/pcs_cap324
  ATH_SOC := ar9344
  DEVICE_TITLE := PowerCloud Systems CAP324
  IMAGE_SIZE := 16000k
  IMAGES := sysupgrade.bin
  SUPPORTED_DEVICES += cap324
endef
TARGET_DEVICES += pcs_cap324

define Device/pcs_cr3000
  ATH_SOC := ar9341
  DEVICE_TITLE := PowerCloud Systems CR3000
  IMAGE_SIZE := 7808k
  IMAGES := sysupgrade.bin
  SUPPORTED_DEVICES += cr3000
endef
TARGET_DEVICES += pcs_cr3000

define Device/pcs_cr5000
  ATH_SOC := ar9344
  DEVICE_TITLE := PowerCloud Systems CR5000
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-core
  IMAGE_SIZE := 7808k
  IMAGES := sysupgrade.bin
  SUPPORTED_DEVICES += cr5000
endef
TARGET_DEVICES += pcs_cr5000

define Device/netgear_wndr3x00
  ATH_SOC := ar7161
  KERNEL := kernel-bin | append-dtb | lzma -d20 | netgear-uImage lzma
  IMAGES := sysupgrade.bin factory.img
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | netgear-squashfs | append-rootfs | pad-rootfs
  IMAGE/sysupgrade.bin := $$(IMAGE/default) | append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.img := $$(IMAGE/default) | netgear-dni | check-size $$$$(IMAGE_SIZE)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport kmod-leds-reset kmod-owl-loader
endef

define Device/netgear_wndr3700
  $(Device/netgear_wndr3x00)
  DEVICE_TITLE := NETGEAR WNDR3700
  NETGEAR_KERNEL_MAGIC := 0x33373030
  NETGEAR_BOARD_ID := WNDR3700
  IMAGE_SIZE := 7680k
  IMAGES += factory-NA.img
  IMAGE/factory-NA.img := $$(IMAGE/default) | netgear-dni NA | check-size $$$$(IMAGE_SIZE)
  SUPPORTED_DEVICES += wndr3700
endef
TARGET_DEVICES += netgear_wndr3700

define Device/netgear_wndr3700v2
  $(Device/netgear_wndr3x00)
  DEVICE_TITLE := NETGEAR WNDR3700v2
  NETGEAR_KERNEL_MAGIC := 0x33373031
  NETGEAR_BOARD_ID := WNDR3700v2
  NETGEAR_HW_ID := 29763654+16+64
  IMAGE_SIZE := 15872k
  SUPPORTED_DEVICES += wndr3700v2
endef
TARGET_DEVICES += netgear_wndr3700v2

define Device/pisen_wmm003n
  $(Device/tplink-8mlzma)
  ATH_SOC := ar9331
  DEVICE_TITLE := Pisen WMM003N (Cloud Easy Power)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-chipidea2
  TPLINK_HWID := 0x07030101
  IMAGES := sysupgrade.bin
endef
TARGET_DEVICES += pisen_wmm003n

define Device/netgear_wndr3800
  $(Device/netgear_wndr3x00)
  DEVICE_TITLE := NETGEAR WNDR3800
  NETGEAR_KERNEL_MAGIC := 0x33373031
  NETGEAR_BOARD_ID := WNDR3800
  NETGEAR_HW_ID := 29763654+16+128
  IMAGE_SIZE := 15872k
  SUPPORTED_DEVICES += wndr3800
endef
TARGET_DEVICES += netgear_wndr3800

define Device/phicomm_k2t
  ATH_SOC := qca9563
  DEVICE_TITLE := Phicomm K2T
  KERNEL := kernel-bin | append-dtb | lzma | uImage lzma
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | uImage lzma
  IMAGE_SIZE := 15744k
  IMAGES := sysupgrade.bin
  IMAGE/default := append-kernel | append-rootfs | pad-rootfs
  IMAGE/sysupgrade.bin := $$(IMAGE/default) | append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_PACKAGES := kmod-leds-reset kmod-ath10k-ct ath10k-firmware-qca9888-ct
endef
TARGET_DEVICES += phicomm_k2t

define Device/wd_mynet-wifi-rangeextender
  ATH_SOC := ar9344
  DEVICE_TITLE := Western Digital My Net Wi-Fi Range Extender
  DEVICE_PACKAGES := rssileds nvram -swconfig
  IMAGE_SIZE := 7808k
  ADDPATTERN_ID := mynet-rext
  ADDPATTERN_VERSION := 1.00.01
  IMAGE/sysupgrade.bin := append-rootfs | pad-rootfs | cybertan-trx | \
	addpattern | append-metadata
  SUPPORTED_DEVICES += mynet-rext
endef
TARGET_DEVICES += wd_mynet-wifi-rangeextender
