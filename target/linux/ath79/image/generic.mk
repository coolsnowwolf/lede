include ./common-buffalo.mk
include ./common-mikrotik.mk
include ./common-netgear.mk
include ./common-tp-link.mk
include ./common-yuncore.mk

DEVICE_VARS += ADDPATTERN_ID ADDPATTERN_VERSION
DEVICE_VARS += SEAMA_SIGNATURE SEAMA_MTDBLOCK
DEVICE_VARS += KERNEL_INITRAMFS_PREFIX

define Build/add-elecom-factory-initramfs
  $(eval edimax_model=$(word 1,$(1)))
  $(eval product=$(word 2,$(1)))

  $(STAGING_DIR_HOST)/bin/mkedimaximg \
	-b -s CSYS -m $(edimax_model) \
	-f 0x70000 -S 0x01100000 \
	-i $@ -o $@.factory

  ( \
	echo -n -e "ELECOM\x00\x00$(product)" | dd bs=40 count=1 conv=sync; \
	echo -n "0.00" | dd bs=16 count=1 conv=sync; \
	dd if=$@.factory; \
  ) > $@.factory.new

  if [ "$$(stat -c%s $@.factory.new)" -le $$(($(subst k,* 1024,$(subst m, * 1024k,$(IMAGE_SIZE))))) ]; then \
	mv $@.factory.new $(BIN_DIR)/$(KERNEL_INITRAMFS_PREFIX)-factory.bin; \
  else \
	echo "WARNING: initramfs kernel image too big, cannot generate factory image" >&2; \
  fi
endef

define Build/addpattern
	-$(STAGING_DIR_HOST)/bin/addpattern -B $(ADDPATTERN_ID) \
		-v v$(ADDPATTERN_VERSION) -i $@ -o $@.new
	-mv "$@.new" "$@"
endef

define Build/cybertan-trx
	@echo -n '' > $@-empty.bin
	-$(STAGING_DIR_HOST)/bin/trx -o $@.new \
		-f $(IMAGE_KERNEL) -F $@-empty.bin \
		-x 32 -a 0x10000 -x -32 -f $@
	-mv "$@.new" "$@"
	-rm $@-empty.bin
endef

define Build/nec-enc
  $(STAGING_DIR_HOST)/bin/nec-enc \
    -i $@ -o $@.new -k $(1)
  mv $@.new $@
endef

define Build/nec-fw
  ( stat -c%s $@ | tr -d "\n" | dd bs=16 count=1 conv=sync; ) >> $@
  ( \
    echo -n -e "$(1)" | dd bs=16 count=1 conv=sync; \
    echo -n "0.0.00" | dd bs=16 count=1 conv=sync; \
    dd if=$@; \
  ) > $@.new
  mv $@.new $@
endef

define Build/pisen_wmb001n-factory
  -[ -f "$@" ] && \
  mkdir -p "$@.tmp" && \
  cp "$(KDIR)/loader-$(word 1,$(1)).uImage" "$@.tmp/uImage" && \
  mv "$@" "$@.tmp/rootfs" && \
  cp "bin/pisen_wmb001n_factory-header.bin" "$@" && \
  $(TAR) -cp --numeric-owner --owner=0 --group=0 --mode=a-s --sort=name \
    $(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") \
    -C "$@.tmp" . | gzip -9n >> "$@" && \
  rm -rf "$@.tmp"
endef

define Device/seama
  KERNEL := kernel-bin | append-dtb | relocate-kernel | lzma
  KERNEL_INITRAMFS := $$(KERNEL) | seama
  IMAGES += factory.bin
  SEAMA_MTDBLOCK := 1

  # 64 bytes offset:
  # - 28 bytes seama_header
  # - 36 bytes of META data (4-bytes aligned)
  IMAGE/default := append-kernel | pad-offset $$$$(BLOCKSIZE) 64 | append-rootfs
  IMAGE/sysupgrade.bin := $$(IMAGE/default) | seama | pad-rootfs | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := $$(IMAGE/default) | pad-rootfs -x 64 | seama | \
	seama-seal | check-size $$$$(IMAGE_SIZE)
  SEAMA_SIGNATURE :=
endef


define Device/8dev_carambola2
  SOC := ar9331
  DEVICE_VENDOR := 8devices
  DEVICE_MODEL := Carambola2
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-chipidea2
  IMAGE_SIZE := 16000k
  SUPPORTED_DEVICES += carambola2
endef
TARGET_DEVICES += 8dev_carambola2

define Device/adtran_bsap1880
  SOC := ar7161
  DEVICE_VENDOR := Adtran/Bluesocket
  DEVICE_PACKAGES += -swconfig -uboot-envtools fconfig
  KERNEL := kernel-bin | append-dtb | lzma | pad-to $$(BLOCKSIZE)
  KERNEL_INITRAMFS := kernel-bin | append-dtb
  IMAGE_SIZE := 11200k
  IMAGES += kernel.bin rootfs.bin
  IMAGE/kernel.bin := append-kernel
  IMAGE/rootfs.bin := append-rootfs | pad-rootfs
  IMAGE/sysupgrade.bin := append-rootfs | pad-rootfs | \
	check-size $$$$(IMAGE_SIZE) | sysupgrade-tar rootfs=$$$$@ | append-metadata
endef

define Device/adtran_bsap1800-v2
  $(Device/adtran_bsap1880)
  DEVICE_MODEL := BSAP-1800
  DEVICE_VARIANT := v2
endef
TARGET_DEVICES += adtran_bsap1800-v2

define Device/adtran_bsap1840
  $(Device/adtran_bsap1880)
  DEVICE_MODEL := BSAP-1840
endef
TARGET_DEVICES += adtran_bsap1840

define Device/alfa-network_ap121f
  SOC := ar9331
  DEVICE_VENDOR := ALFA Network
  DEVICE_MODEL := AP121F
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-chipidea2 kmod-usb-storage -swconfig
  IMAGE_SIZE := 16064k
  SUPPORTED_DEVICES += ap121f
endef
TARGET_DEVICES += alfa-network_ap121f

define Device/aruba_ap-105
  SOC := ar7161
  DEVICE_VENDOR := Aruba
  DEVICE_MODEL := AP-105
  IMAGE_SIZE := 16000k
  DEVICE_PACKAGES := kmod-i2c-core kmod-i2c-gpio kmod-tpm-i2c-atmel
endef
TARGET_DEVICES += aruba_ap-105

define Device/avm_fritz300e
  SOC := ar7242
  DEVICE_VENDOR := AVM
  DEVICE_MODEL := FRITZ!WLAN Repeater 300E
  KERNEL := kernel-bin | append-dtb | lzma | eva-image
  KERNEL_INITRAMFS := $$(KERNEL)
  IMAGE_SIZE := 15232k
  IMAGE/sysupgrade.bin := append-kernel | pad-to 64k | \
	append-squashfs-fakeroot-be | pad-to 256 | append-rootfs | pad-rootfs | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_PACKAGES := fritz-tffs rssileds -swconfig
endef
TARGET_DEVICES += avm_fritz300e

define Device/avm_fritz4020
  SOC := qca9561
  DEVICE_VENDOR := AVM
  DEVICE_MODEL := FRITZ!Box 4020
  IMAGE_SIZE := 15232k
  KERNEL := kernel-bin | append-dtb | lzma | eva-image
  KERNEL_INITRAMFS := $$(KERNEL)
  IMAGE/sysupgrade.bin := append-kernel | pad-to 64k | \
	append-squashfs-fakeroot-be | pad-to 256 | append-rootfs | pad-rootfs | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_PACKAGES := fritz-tffs
  SUPPORTED_DEVICES += fritz4020
endef
TARGET_DEVICES += avm_fritz4020

define Device/buffalo_bhr-4grv
  SOC := ar7242
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := BHR-4GRV
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  IMAGE_SIZE := 32256k
  IMAGES += factory.bin tftp.bin
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | \
	pad-rootfs | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := $$(IMAGE/default) | buffalo-enc BHR-4GRV 1.99 | \
	buffalo-tag BHR-4GRV 3
  IMAGE/tftp.bin := $$(IMAGE/default) | buffalo-tftp-header
  SUPPORTED_DEVICES += wzr-hp-g450h
endef
TARGET_DEVICES += buffalo_bhr-4grv

define Device/buffalo_bhr-4grv2
  SOC := qca9557
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := BHR-4GRV2
  IMAGE_SIZE := 16000k
endef
TARGET_DEVICES += buffalo_bhr-4grv2

define Device/buffalo_wzr-hp-ag300h
  SOC := ar7161
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WZR-HP-AG300H
  IMAGE_SIZE := 32320k
  IMAGES += factory.bin tftp.bin
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | \
	pad-rootfs | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := $$(IMAGE/default) | buffalo-enc WZR-HP-AG300H 1.99 | \
	buffalo-tag WZR-HP-AG300H 3
  IMAGE/tftp.bin := $$(IMAGE/default) | buffalo-tftp-header
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport \
	kmod-leds-reset kmod-owl-loader
  SUPPORTED_DEVICES += wzr-hp-ag300h
endef
TARGET_DEVICES += buffalo_wzr-hp-ag300h

define Device/buffalo_wzr-hp-g302h-a1a0
  SOC := ar7242
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WZR-HP-G302H
  DEVICE_VARIANT := A1A0
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  IMAGE_SIZE := 32128k
  IMAGES += factory.bin tftp.bin
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | \
	pad-rootfs | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := $$(IMAGE/default) | buffalo-enc WZR-HP-G302H 1.99 | \
	buffalo-tag WZR-HP-G302H 4
  IMAGE/tftp.bin := $$(IMAGE/default) | buffalo-tftp-header
  SUPPORTED_DEVICES += wzr-hp-g300nh2
endef
TARGET_DEVICES += buffalo_wzr-hp-g302h-a1a0

define Device/buffalo_wzr-hp-g450h
  SOC := ar7242
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WZR-HP-G450H/WZR-450HP
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  IMAGE_SIZE := 32256k
  IMAGES += factory.bin tftp.bin
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | \
	pad-rootfs | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := $$(IMAGE/default) | buffalo-enc WZR-HP-G450H 1.99 | \
	buffalo-tag WZR-HP-G450H 3
  IMAGE/tftp.bin := $$(IMAGE/default) | buffalo-tftp-header
  SUPPORTED_DEVICES += wzr-hp-g450h
endef
TARGET_DEVICES += buffalo_wzr-hp-g450h

define Device/comfast_cf-e110n-v2
  SOC := qca9533
  DEVICE_VENDOR := COMFAST
  DEVICE_MODEL := CF-E110N
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := rssileds kmod-leds-gpio -swconfig -uboot-envtools
  IMAGE_SIZE := 16192k
endef
TARGET_DEVICES += comfast_cf-e110n-v2

define Device/comfast_cf-e120a-v3
  SOC := ar9344
  DEVICE_VENDOR := COMFAST
  DEVICE_MODEL := CF-E120A
  DEVICE_VARIANT := v3
  DEVICE_PACKAGES := rssileds kmod-leds-gpio -uboot-envtools
  IMAGE_SIZE := 8000k
endef
TARGET_DEVICES += comfast_cf-e120a-v3

define Device/comfast_cf-e313ac
  SOC := qca9531
  DEVICE_VENDOR := COMFAST
  DEVICE_MODEL := CF-E313AC
  DEVICE_PACKAGES := rssileds kmod-leds-gpio kmod-ath10k-ct-smallbuffers \
	ath10k-firmware-qca9888-ct -swconfig -uboot-envtools
  IMAGE_SIZE := 7936k
endef
TARGET_DEVICES += comfast_cf-e313ac

define Device/comfast_cf-e314n-v2
  SOC := qca9531
  DEVICE_VENDOR := COMFAST
  DEVICE_MODEL := CF-E314N
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := rssileds
  IMAGE_SIZE := 7936k
endef
TARGET_DEVICES += comfast_cf-e314n-v2

define Device/comfast_cf-e5
  SOC := qca9531
  DEVICE_VENDOR := COMFAST
  DEVICE_MODEL := CF-E5/E7
  DEVICE_PACKAGES := rssileds kmod-leds-gpio kmod-usb2 kmod-usb-net \
	kmod-usb-net-qmi-wwan -swconfig -uboot-envtools
  IMAGE_SIZE := 16192k
endef
TARGET_DEVICES += comfast_cf-e5

define Device/comfast_cf-e560ac
  SOC := qca9531
  DEVICE_VENDOR := COMFAST
  DEVICE_MODEL := CF-E560AC
  DEVICE_PACKAGES := kmod-leds-gpio kmod-usb2 kmod-ath10k-ct \
	ath10k-firmware-qca9888-ct
  IMAGE_SIZE := 16128k
endef
TARGET_DEVICES += comfast_cf-e560ac

define Device/comfast_cf-wr650ac-v1
  SOC := qca9558
  DEVICE_VENDOR := COMFAST
  DEVICE_MODEL := CF-WR650AC
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct
  IMAGE_SIZE := 16128k
endef
TARGET_DEVICES += comfast_cf-wr650ac-v1

define Device/comfast_cf-wr650ac-v2
  SOC := qca9558
  DEVICE_VENDOR := COMFAST
  DEVICE_MODEL := CF-WR650AC
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct
  IMAGE_SIZE := 16000k
endef
TARGET_DEVICES += comfast_cf-wr650ac-v2

define Device/devolo_dvl1200e
  SOC := qca9558
  DEVICE_VENDOR := devolo
  DEVICE_MODEL := WiFi pro 1200e
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct
  IMAGE_SIZE := 15936k
endef
TARGET_DEVICES += devolo_dvl1200e

define Device/devolo_dvl1200i
  SOC := qca9558
  DEVICE_VENDOR := devolo
  DEVICE_MODEL := WiFi pro 1200i
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct
  IMAGE_SIZE := 15936k
endef
TARGET_DEVICES += devolo_dvl1200i

define Device/devolo_dvl1750c
  SOC := qca9558
  DEVICE_VENDOR := devolo
  DEVICE_MODEL := WiFi pro 1750c
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct
  IMAGE_SIZE := 15936k
endef
TARGET_DEVICES += devolo_dvl1750c

define Device/devolo_dvl1750e
  SOC := qca9558
  DEVICE_VENDOR := devolo
  DEVICE_MODEL := WiFi pro 1750e
  DEVICE_PACKAGES := kmod-usb2 kmod-ath10k-ct ath10k-firmware-qca988x-ct
  IMAGE_SIZE := 15936k
endef
TARGET_DEVICES += devolo_dvl1750e

define Device/devolo_dvl1750i
  SOC := qca9558
  DEVICE_VENDOR := devolo
  DEVICE_MODEL := WiFi pro 1750i
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct
  IMAGE_SIZE := 15936k
endef
TARGET_DEVICES += devolo_dvl1750i

define Device/devolo_dvl1750x
  SOC := qca9558
  DEVICE_VENDOR := devolo
  DEVICE_MODEL := WiFi pro 1750x
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct
  IMAGE_SIZE := 15936k
endef
TARGET_DEVICES += devolo_dvl1750x

define Device/dlink_dir-505
  SOC := ar9330
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-505
  IMAGE_SIZE := 7680k
  DEVICE_PACKAGES := kmod-usb2
  SUPPORTED_DEVICES += dir-505-a1
endef
TARGET_DEVICES += dlink_dir-505

define Device/dlink_dir-825-b1
  SOC := ar7161
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-825
  DEVICE_VARIANT := B1
  IMAGE_SIZE := 6208k
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport \
	kmod-leds-reset kmod-owl-loader
  SUPPORTED_DEVICES += dir-825-b1
endef
TARGET_DEVICES += dlink_dir-825-b1

define Device/dlink_dir-825-c1
  SOC := ar9344
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-825
  DEVICE_VARIANT := C1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport kmod-leds-reset \
	kmod-owl-loader
  SUPPORTED_DEVICES += dir-825-c1
  IMAGE_SIZE := 15936k
  IMAGES := factory.bin sysupgrade.bin
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | \
	pad-rootfs
  IMAGE/factory.bin := $$(IMAGE/default) | pad-offset $$$$(IMAGE_SIZE) 26 | \
	append-string 00DB120AR9344-RT-101214-00 | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := $$(IMAGE/default) | append-metadata | \
	check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += dlink_dir-825-c1

define Device/dlink_dir-835-a1
  SOC := ar9344
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-835
  DEVICE_VARIANT := A1
  DEVICE_PACKAGES := kmod-usb2 kmod-leds-reset kmod-owl-loader
  SUPPORTED_DEVICES += dir-835-a1
  IMAGE_SIZE := 15936k
  IMAGES := factory.bin sysupgrade.bin
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | \
	pad-rootfs
  IMAGE/factory.bin := $$(IMAGE/default) | pad-offset $$$$(IMAGE_SIZE) 26 | \
	append-string 00DB120AR9344-RT-101214-00 | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := $$(IMAGE/default) | append-metadata | \
	check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += dlink_dir-835-a1

define Device/dlink_dir-842-c
  SOC := qca9563
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-842
  KERNEL := kernel-bin | append-dtb | relocate-kernel | lzma
  KERNEL_INITRAMFS := $$(KERNEL) | seama
  IMAGES += factory.bin
  SEAMA_MTDBLOCK := 5
  SEAMA_SIGNATURE := wrgac65_dlink.2015_dir842
  # 64 bytes offset:
  # - 28 bytes seama_header
  # - 36 bytes of META data (4-bytes aligned)
  IMAGE/default := append-kernel | uImage lzma | \
	pad-offset $$$$(BLOCKSIZE) 64 | append-rootfs
  IMAGE/sysupgrade.bin := $$(IMAGE/default) | seama | pad-rootfs | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := $$(IMAGE/default) | pad-rootfs -x 64 | seama | \
	seama-seal | check-size $$$$(IMAGE_SIZE)
  IMAGE_SIZE := 15680k
endef

define Device/dlink_dir-842-c1
  $(Device/dlink_dir-842-c)
  DEVICE_VARIANT := C1
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca9888-ct
endef
TARGET_DEVICES += dlink_dir-842-c1

define Device/dlink_dir-842-c2
  $(Device/dlink_dir-842-c)
  DEVICE_VARIANT := C2
  DEVICE_PACKAGES := kmod-usb2 kmod-ath10k-ct ath10k-firmware-qca9888-ct
endef
TARGET_DEVICES += dlink_dir-842-c2

define Device/dlink_dir-842-c3
  $(Device/dlink_dir-842-c)
  DEVICE_VARIANT := C3
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca9888-ct
endef
TARGET_DEVICES += dlink_dir-842-c3

define Device/dlink_dir-859-a1
  $(Device/seama)
  SOC := qca9563
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-859
  DEVICE_VARIANT := A1
  IMAGE_SIZE := 15872k
  DEVICE_PACKAGES :=  kmod-usb2 kmod-ath10k-ct-smallbuffers ath10k-firmware-qca988x-ct
  SEAMA_SIGNATURE := wrgac37_dlink.2013gui_dir859
endef
TARGET_DEVICES += dlink_dir-859-a1

define Device/elecom_wrc-1750ghbk2-i
  SOC := qca9563
  DEVICE_VENDOR := ELECOM
  DEVICE_MODEL := WRC-1750GHBK2-I/C
  IMAGE_SIZE := 15808k
  KERNEL_INITRAMFS := $$(KERNEL) | pad-to 2 | \
	add-elecom-factory-initramfs RN68 WRC-1750GHBK2
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct
endef
TARGET_DEVICES += elecom_wrc-1750ghbk2-i

define Device/elecom_wrc-300ghbk2-i
  SOC := qca9563
  DEVICE_VENDOR := ELECOM
  DEVICE_MODEL := WRC-300GHBK2-I
  IMAGE_SIZE := 7616k
  KERNEL_INITRAMFS := $$(KERNEL) | pad-to 2 | \
	add-elecom-factory-initramfs RN51 WRC-300GHBK2-I
endef
TARGET_DEVICES += elecom_wrc-300ghbk2-i

define Device/embeddedwireless_dorin
  SOC := ar9331
  DEVICE_VENDOR := Embedded Wireless
  DEVICE_MODEL := Dorin
  DEVICE_PACKAGES := kmod-usb-chipidea2
  IMAGE_SIZE := 16000k
endef
TARGET_DEVICES += embeddedwireless_dorin

define Device/engenius_ecb1750
  SOC := qca9558
  DEVICE_VENDOR := EnGenius
  DEVICE_MODEL := ECB1750
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct
  IMAGE_SIZE := 15680k
  IMAGE/factory.bin := append-kernel | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE) | \
	senao-header -r 0x101 -p 0x6d -t 2
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += engenius_ecb1750

define Device/engenius_epg5000
  SOC := qca9558
  DEVICE_VENDOR := EnGenius
  DEVICE_MODEL := EPG5000
  DEVICE_PACKAGES := ath10k-firmware-qca988x-ct kmod-ath10k-ct kmod-usb2
  IMAGE_SIZE := 14656k
  IMAGES += factory.dlf
  IMAGE/factory.dlf := append-kernel | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE) | \
	senao-header -r 0x101 -p 0x71 -t 2
  SUPPORTED_DEVICES += epg5000
endef
TARGET_DEVICES += engenius_epg5000

define Device/engenius_ews511ap
  SOC := qca9531
  DEVICE_VENDOR := EnGenius
  DEVICE_MODEL := EWS511AP
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca9887-ct
  IMAGE_SIZE := 16000k
endef
TARGET_DEVICES += engenius_ews511ap

define Device/etactica_eg200
  SOC := ar9331
  DEVICE_VENDOR := eTactica
  DEVICE_MODEL := EG200
  DEVICE_PACKAGES := kmod-usb-chipidea2 kmod-ledtrig-oneshot \
	kmod-usb-serial kmod-usb-serial-ftdi kmod-usb-storage kmod-fs-ext4
  IMAGE_SIZE := 16000k
  SUPPORTED_DEVICES += rme-eg200
endef
TARGET_DEVICES += etactica_eg200

define Device/glinet_6408
  $(Device/tplink-8mlzma)
  SOC := ar9331
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := 6408
  DEVICE_PACKAGES := kmod-usb2
  IMAGE_SIZE := 8000k
  TPLINK_HWID := 0x08000001
  IMAGES := sysupgrade.bin
  SUPPORTED_DEVICES += gl-inet
endef
TARGET_DEVICES += glinet_6408

define Device/glinet_6416
  $(Device/tplink-16mlzma)
  SOC := ar9331
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := 6416
  DEVICE_PACKAGES := kmod-usb2
  IMAGE_SIZE := 16192k
  TPLINK_HWID := 0x08000001
  IMAGES := sysupgrade.bin
  SUPPORTED_DEVICES += gl-inet
endef
TARGET_DEVICES += glinet_6416

define Device/glinet_gl-ar150
  SOC := ar9330
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := GL-AR150
  DEVICE_PACKAGES := kmod-usb-chipidea2
  IMAGE_SIZE := 16000k
  SUPPORTED_DEVICES += gl-ar150
endef
TARGET_DEVICES += glinet_gl-ar150

define Device/glinet_gl-ar300m-common-nor
  SOC := qca9531
  DEVICE_VENDOR := GL.iNet
  DEVICE_PACKAGES := kmod-usb2
  IMAGE_SIZE := 16000k
  SUPPORTED_DEVICES += gl-ar300m
endef

define Device/glinet_gl-ar300m-lite
  $(Device/glinet_gl-ar300m-common-nor)
  DEVICE_MODEL := GL-AR300M
  DEVICE_VARIANT := Lite
endef
TARGET_DEVICES += glinet_gl-ar300m-lite

define Device/glinet_gl-ar300m16
  $(Device/glinet_gl-ar300m-common-nor)
  DEVICE_MODEL := GL-AR300M16
endef
TARGET_DEVICES += glinet_gl-ar300m16

define Device/glinet_gl-ar750
  SOC := qca9531
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := GL-AR750
  DEVICE_PACKAGES := kmod-usb2 kmod-ath10k-ct ath10k-firmware-qca9887-ct
  IMAGE_SIZE := 16000k
  SUPPORTED_DEVICES += gl-ar750
endef
TARGET_DEVICES += glinet_gl-ar750

define Device/glinet_gl-x750
  SOC := qca9531
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := GL-X750
  DEVICE_PACKAGES := kmod-usb2 kmod-ath10k-ct ath10k-firmware-qca9887-ct
  IMAGE_SIZE := 16000k
endef
TARGET_DEVICES += glinet_gl-x750

define Device/iodata_etg3-r
  SOC := ar9342
  DEVICE_VENDOR := I-O DATA
  DEVICE_MODEL := ETG3-R
  IMAGE_SIZE := 7680k
  DEVICE_PACKAGES := -iwinfo -kmod-ath9k -wpad
endef
TARGET_DEVICES += iodata_etg3-r

define Device/iodata_wn-ac1167dgr
  SOC := qca9557
  DEVICE_VENDOR := I-O DATA
  DEVICE_MODEL := WN-AC1167DGR
  IMAGE_SIZE := 14656k
  IMAGES += factory.bin
  IMAGE/factory.bin := append-kernel | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE) | \
	senao-header -r 0x30a -p 0x61 -t 2
  DEVICE_PACKAGES := kmod-usb2 kmod-ath10k-ct ath10k-firmware-qca988x-ct
endef
TARGET_DEVICES += iodata_wn-ac1167dgr

define Device/iodata_wn-ac1600dgr
  SOC := qca9557
  DEVICE_VENDOR := I-O DATA
  DEVICE_MODEL := WN-AC1600DGR
  IMAGE_SIZE := 14656k
  IMAGES += factory.bin
  IMAGE/factory.bin := append-kernel | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE) | \
	senao-header -r 0x30a -p 0x60 -t 2 -v 200
  DEVICE_PACKAGES := kmod-usb2 kmod-ath10k-ct ath10k-firmware-qca988x-ct
endef
TARGET_DEVICES += iodata_wn-ac1600dgr

define Device/iodata_wn-ac1600dgr2
  SOC := qca9557
  DEVICE_VENDOR := I-O DATA
  DEVICE_MODEL := WN-AC1600DGR2/DGR3
  IMAGE_SIZE := 14656k
  IMAGES += dgr2-dgr3-factory.bin
  IMAGE/dgr2-dgr3-factory.bin := append-kernel | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE) | \
	senao-header -r 0x30a -p 0x60 -t 2 -v 200
  DEVICE_PACKAGES := kmod-usb2 kmod-ath10k-ct ath10k-firmware-qca988x-ct
endef
TARGET_DEVICES += iodata_wn-ac1600dgr2

define Device/iodata_wn-ag300dgr
  SOC := ar1022
  DEVICE_VENDOR := I-O DATA
  DEVICE_MODEL := WN-AG300DGR
  IMAGE_SIZE := 15424k
  IMAGES += factory.bin
  IMAGE/factory.bin := append-kernel | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE) | \
	senao-header -r 0x30a -p 0x47 -t 2
  DEVICE_PACKAGES := kmod-usb2
endef
TARGET_DEVICES += iodata_wn-ag300dgr

define Device/jjplus_ja76pf2
  SOC := ar7161
  DEVICE_VENDOR := jjPlus
  DEVICE_MODEL := JA76PF2
  DEVICE_PACKAGES += -kmod-ath9k -swconfig -wpad-mini -uboot-envtools fconfig
  IMAGES := kernel.bin rootfs.bin
  IMAGE/kernel.bin := append-kernel
  IMAGE/rootfs.bin := append-rootfs | pad-rootfs
  KERNEL := kernel-bin | append-dtb | lzma | pad-to $$(BLOCKSIZE)
  KERNEL_INITRAMFS := kernel-bin | append-dtb
  IMAGE_SIZE := 16000k
  SUPPORTED_DEVICES += ja76pf2
endef
TARGET_DEVICES += jjplus_ja76pf2

define Device/librerouter_librerouter-v1
  SOC := qca9558
  DEVICE_VENDOR := Librerouter
  DEVICE_MODEL := LibreRouter
  DEVICE_VARIANT := v1
  IMAGE_SIZE := 7936k
  DEVICE_PACKAGES := kmod-usb2
endef
TARGET_DEVICES += librerouter_librerouter-v1

define Device/mikrotik_routerboard-wap-g-5hact2hnd
  $(Device/mikrotik)
  SOC := qca9556
  DEVICE_MODEL := RouterBOARD wAP G-5HacT2HnD (wAP AC)
  IMAGE_SIZE := 16256k
  IMAGE/sysupgrade.bin := append-kernel | kernel2minor -s 1024 -e | \
	pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_PACKAGES += kmod-ath10k-ct-smallbuffers ath10k-firmware-qca988x-ct
  SUPPORTED_DEVICES += rb-wapg-5hact2hnd
endef
TARGET_DEVICES += mikrotik_routerboard-wap-g-5hact2hnd

define Device/nec_wg1200cr
  SOC := qca9563
  DEVICE_VENDOR := NEC
  DEVICE_MODEL := Aterm WG1200CR
  IMAGE_SIZE := 7616k
  SEAMA_MTDBLOCK := 6
  SEAMA_SIGNATURE := wrgac72_necpf.2016gui_wg1200cr
  IMAGES += factory.bin
  IMAGE/default := append-kernel | pad-offset $$$$(BLOCKSIZE) 64 | append-rootfs
  IMAGE/sysupgrade.bin := $$(IMAGE/default) | seama | pad-rootfs | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := $$(IMAGE/default) | pad-rootfs -x 64 | seama | \
	seama-seal | nec-enc 9gsiy9nzep452pad | check-size $$$$(IMAGE_SIZE)
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca9888-ct
endef
TARGET_DEVICES += nec_wg1200cr

define Device/nec_wg800hp
  SOC := qca9563
  DEVICE_VENDOR := NEC
  DEVICE_MODEL := Aterm WG800HP
  IMAGE_SIZE := 7104k
  IMAGES += factory.bin
  IMAGE/factory.bin := append-kernel | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE) | \
	xor-image -p 6A57190601121E4C004C1E1201061957 -x | nec-fw LASER_ATERM
  DEVICE_PACKAGES := kmod-ath10k-ct-smallbuffers ath10k-firmware-qca9887-ct-htt
endef
TARGET_DEVICES += nec_wg800hp

define Device/netgear_ex6400_ex7300
  SOC := qca9558
  NETGEAR_KERNEL_MAGIC := 0x27051956
  NETGEAR_BOARD_ID := EX7300series
  NETGEAR_HW_ID := 29765104+16+0+128
  IMAGE_SIZE := 15552k
  IMAGE/default := append-kernel | pad-offset $$$$(BLOCKSIZE) 64 | \
	netgear-rootfs | pad-rootfs
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca99x0-ct
  $(Device/netgear_ath79)
endef

define Device/netgear_ex6400
  $(Device/netgear_ex6400_ex7300)
  DEVICE_MODEL := EX6400
endef
TARGET_DEVICES += netgear_ex6400

define Device/netgear_ex7300
  $(Device/netgear_ex6400_ex7300)
  DEVICE_MODEL := EX7300
endef
TARGET_DEVICES += netgear_ex7300

define Device/netgear_wndr3x00
  SOC := ar7161
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | netgear-squashfs | \
	append-rootfs | pad-rootfs
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport \
	kmod-leds-reset kmod-owl-loader
  $(Device/netgear_ath79)
endef

define Device/netgear_wndr3700
  $(Device/netgear_wndr3x00)
  DEVICE_MODEL := WNDR3700
  DEVICE_VARIANT := v1
  NETGEAR_KERNEL_MAGIC := 0x33373030
  NETGEAR_BOARD_ID := WNDR3700
  IMAGE_SIZE := 7680k
  IMAGES += factory-NA.img
  IMAGE/factory-NA.img := $$(IMAGE/default) | netgear-dni NA | \
	check-size $$$$(IMAGE_SIZE)
  SUPPORTED_DEVICES += wndr3700
endef
TARGET_DEVICES += netgear_wndr3700

define Device/netgear_wndr3700-v2
  $(Device/netgear_wndr3x00)
  DEVICE_MODEL := WNDR3700
  DEVICE_VARIANT := v2
  NETGEAR_KERNEL_MAGIC := 0x33373031
  NETGEAR_BOARD_ID := WNDR3700v2
  NETGEAR_HW_ID := 29763654+16+64
  IMAGE_SIZE := 15872k
  SUPPORTED_DEVICES += wndr3700 netgear,wndr3700v2
endef
TARGET_DEVICES += netgear_wndr3700-v2

define Device/netgear_wndr3800
  $(Device/netgear_wndr3x00)
  DEVICE_MODEL := WNDR3800
  NETGEAR_KERNEL_MAGIC := 0x33373031
  NETGEAR_BOARD_ID := WNDR3800
  NETGEAR_HW_ID := 29763654+16+128
  IMAGE_SIZE := 15872k
  SUPPORTED_DEVICES += wndr3700
endef
TARGET_DEVICES += netgear_wndr3800

define Device/netgear_wndr3800ch
  $(Device/netgear_wndr3x00)
  DEVICE_MODEL := WNDR3800CH
  NETGEAR_KERNEL_MAGIC := 0x33373031
  NETGEAR_BOARD_ID := WNDR3800CH
  NETGEAR_HW_ID := 29763654+16+128
  IMAGE_SIZE := 15872k
  SUPPORTED_DEVICES += wndr3700
endef
TARGET_DEVICES += netgear_wndr3800ch

define Device/netgear_wnr2200_common
  SOC := ar7241
  DEVICE_MODEL := WNR2200
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  NETGEAR_KERNEL_MAGIC := 0x32323030
  NETGEAR_BOARD_ID := wnr2200
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | netgear-squashfs | \
	append-rootfs | pad-rootfs
  $(Device/netgear_ath79)
endef

define Device/netgear_wnr2200-8m
  $(Device/netgear_wnr2200_common)
  DEVICE_VARIANT := 8M
  NETGEAR_HW_ID := 29763600+08+64
  IMAGE_SIZE := 7808k
  IMAGES += factory-NA.img
  IMAGE/factory-NA.img := $$(IMAGE/default) | netgear-dni NA | \
	check-size $$$$(IMAGE_SIZE)
  SUPPORTED_DEVICES += wnr2200
endef
TARGET_DEVICES += netgear_wnr2200-8m

define Device/netgear_wnr2200-16m
  $(Device/netgear_wnr2200_common)
  DEVICE_VARIANT := 16M
  DEVICE_ALT0_VENDOR := NETGEAR
  DEVICE_ALT0_MODEL := WNR2200
  DEVICE_ALT0_VARIANT := CN/RU
  NETGEAR_HW_ID :=
  IMAGE_SIZE := 16000k
endef
TARGET_DEVICES += netgear_wnr2200-16m

define Device/ocedo_koala
  SOC := qca9558
  DEVICE_VENDOR := Ocedo
  DEVICE_MODEL := Koala
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct
  SUPPORTED_DEVICES += koala
  IMAGE_SIZE := 7424k
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += ocedo_koala

define Device/ocedo_raccoon
  SOC := ar9344
  DEVICE_VENDOR := Ocedo
  DEVICE_MODEL := Raccoon
  IMAGE_SIZE := 7424k
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += ocedo_raccoon

define Device/ocedo_ursus
  SOC := qca9558
  DEVICE_VENDOR := Ocedo
  DEVICE_MODEL := Ursus
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct
  IMAGE_SIZE := 7424k
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += ocedo_ursus

define Device/openmesh_om5p-ac-v2
  SOC := qca9558
  DEVICE_VENDOR := OpenMesh
  DEVICE_MODEL := OM5P-AC
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct om-watchdog
  IMAGE_SIZE := 7808k
  SUPPORTED_DEVICES += om5p-acv2
endef
TARGET_DEVICES += openmesh_om5p-ac-v2

define Device/pcs_cap324
  SOC := ar9344
  DEVICE_VENDOR := PowerCloud Systems
  DEVICE_MODEL := CAP324
  IMAGE_SIZE := 16000k
  SUPPORTED_DEVICES += cap324
endef
TARGET_DEVICES += pcs_cap324

define Device/pcs_cr3000
  SOC := ar9341
  DEVICE_VENDOR := PowerCloud Systems
  DEVICE_MODEL := CR3000
  IMAGE_SIZE := 7808k
  SUPPORTED_DEVICES += cr3000
endef
TARGET_DEVICES += pcs_cr3000

define Device/pcs_cr5000
  SOC := ar9344
  DEVICE_VENDOR := PowerCloud Systems
  DEVICE_MODEL := CR5000
  DEVICE_PACKAGES := kmod-usb2
  IMAGE_SIZE := 7808k
  SUPPORTED_DEVICES += cr5000
endef
TARGET_DEVICES += pcs_cr5000

define Device/phicomm_k2t
  SOC := qca9563
  DEVICE_VENDOR := Phicomm
  DEVICE_MODEL := K2T
  IMAGE_SIZE := 15744k
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_PACKAGES := kmod-leds-reset kmod-ath10k-ct-smallbuffers ath10k-firmware-qca9888-ct
endef
TARGET_DEVICES += phicomm_k2t

define Device/pisen_ts-d084
  $(Device/tplink-8mlzma)
  SOC := ar9331
  DEVICE_VENDOR := PISEN
  DEVICE_MODEL := TS-D084
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-chipidea2
  TPLINK_HWID := 0x07030101
endef
TARGET_DEVICES += pisen_ts-d084

define Device/pisen_wmb001n
  SOC := ar9341
  DEVICE_VENDOR := PISEN
  DEVICE_MODEL := WMB001N
  IMAGE_SIZE := 14080k
  DEVICE_PACKAGES := kmod-i2c-core kmod-i2c-gpio kmod-usb2
  LOADER_TYPE := bin
  LOADER_FLASH_OFFS := 0x20000
  COMPILE := loader-$(1).bin loader-$(1).uImage
  COMPILE/loader-$(1).bin := loader-okli-compile
  COMPILE/loader-$(1).uImage := append-loader-okli $(1) | pad-to 64k | lzma | \
	uImage lzma
  KERNEL := kernel-bin | append-dtb | lzma | uImage lzma -M 0x4f4b4c49
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(IMAGE/sysupgrade.bin) | pisen_wmb001n-factory $(1)
endef
TARGET_DEVICES += pisen_wmb001n

define Device/pisen_wmm003n
  $(Device/tplink-8mlzma)
  SOC := ar9331
  DEVICE_VENDOR := PISEN
  DEVICE_MODEL := Cloud Easy Power (WMM003N)
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-chipidea2
  TPLINK_HWID := 0x07030101
endef
TARGET_DEVICES += pisen_wmm003n

define Device/qihoo_c301
  $(Device/seama)
  SOC := ar9344
  DEVICE_VENDOR := Qihoo
  DEVICE_MODEL := C301
  DEVICE_PACKAGES := kmod-usb2 kmod-ath10k-ct ath10k-firmware-qca988x-ct \
	uboot-envtools
  IMAGE_SIZE := 15744k
  SEAMA_SIGNATURE := wrgac26_qihoo360_360rg
  SUPPORTED_DEVICES += qihoo-c301
endef
TARGET_DEVICES += qihoo_c301

define Device/rosinson_wr818
  SOC := qca9563
  DEVICE_VENDOR := Rosinson
  DEVICE_MODEL := WR818
  IMAGE_SIZE := 15872k
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += rosinson_wr818

define Device/sitecom_wlr-7100
  SOC := ar1022
  DEVICE_VENDOR := Sitecom
  DEVICE_MODEL := WLR-7100
  DEVICE_VARIANT := v1 002
  DEVICE_PACKAGES := ath10k-firmware-qca988x-ct kmod-ath10k-ct-smallbuffers kmod-usb2
  IMAGES += factory.dlf
  IMAGE/factory.dlf := append-kernel | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE) | \
	senao-header -r 0x222 -p 0x53 -t 2
  IMAGE_SIZE := 7488k
endef
TARGET_DEVICES += sitecom_wlr-7100

define Device/trendnet_tew-823dru
  SOC := qca9558
  DEVICE_VENDOR := Trendnet
  DEVICE_MODEL := TEW-823DRU
  DEVICE_VARIANT := v1.0R
  DEVICE_PACKAGES := kmod-usb2 kmod-ath10k-ct ath10k-firmware-qca988x-ct
  SUPPORTED_DEVICES += tew-823dru
  IMAGE_SIZE := 15296k
  IMAGES := factory.bin sysupgrade.bin
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | \
	pad-rootfs
  IMAGE/factory.bin := $$(IMAGE/default) | pad-offset $$$$(IMAGE_SIZE) 26 | \
	append-string 00AP135AR9558-RT-131129-00 | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := $$(IMAGE/default) | append-metadata | \
	check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += trendnet_tew-823dru

define Device/wd_mynet-n750
  $(Device/seama)
  SOC := ar9344
  DEVICE_VENDOR := Western Digital
  DEVICE_MODEL := My Net N750
  IMAGE_SIZE := 15872k
  DEVICE_PACKAGES := kmod-usb2
  SEAMA_SIGNATURE := wrgnd13_wd_av
  SUPPORTED_DEVICES += mynet-n750
endef
TARGET_DEVICES += wd_mynet-n750

define Device/wd_mynet-wifi-rangeextender
  SOC := ar9344
  DEVICE_VENDOR := Western Digital
  DEVICE_MODEL := My Net Wi-Fi Range Extender
  DEVICE_PACKAGES := rssileds nvram -swconfig
  IMAGE_SIZE := 7808k
  ADDPATTERN_ID := mynet-rext
  ADDPATTERN_VERSION := 1.00.01
  IMAGE/sysupgrade.bin := append-rootfs | pad-rootfs | cybertan-trx | \
	addpattern | append-metadata
  SUPPORTED_DEVICES += mynet-rext
endef
TARGET_DEVICES += wd_mynet-wifi-rangeextender

define Device/winchannel_wb2000
  SOC := ar9344
  DEVICE_VENDOR := Winchannel
  DEVICE_MODEL := WB2000
  IMAGE_SIZE := 15872k
  DEVICE_PACKAGES := kmod-i2c-core kmod-i2c-gpio kmod-rtc-ds1307 kmod-usb2 \
	kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += winchannel_wb2000

define Device/xiaomi_mi-router-4q
  SOC := qca9561
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Mi Router 4Q
  IMAGE_SIZE := 14336k
endef
TARGET_DEVICES += xiaomi_mi-router-4q

define Device/yuncore_a770
  SOC := qca9531
  DEVICE_VENDOR := YunCore
  DEVICE_MODEL := A770
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca9887-ct
  IMAGE_SIZE := 16000k
  IMAGES += tftp.bin
  IMAGE/tftp.bin := $$(IMAGE/sysupgrade.bin) | yuncore-tftp-header-16m
endef
TARGET_DEVICES += yuncore_a770

define Device/yuncore_a782
  SOC := qca9563
  DEVICE_VENDOR := YunCore
  DEVICE_MODEL := A782
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca9888-ct
  IMAGE_SIZE := 16000k
  IMAGES += tftp.bin
  IMAGE/tftp.bin := $$(IMAGE/sysupgrade.bin) | yuncore-tftp-header-16m
endef
TARGET_DEVICES += yuncore_a782

define Device/yuncore_xd4200
  SOC := qca9563
  DEVICE_VENDOR := YunCore
  DEVICE_MODEL := XD4200
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca9888-ct
  IMAGE_SIZE := 16000k
  IMAGES += tftp.bin
  IMAGE/tftp.bin := $$(IMAGE/sysupgrade.bin) | yuncore-tftp-header-16m
endef
TARGET_DEVICES += yuncore_xd4200

define Device/zbtlink_zbt-wd323
  SOC := ar9344
  DEVICE_VENDOR := ZBT
  DEVICE_MODEL := WD323
  IMAGE_SIZE := 16000k
  DEVICE_PACKAGES := kmod-usb2 kmod-i2c-core kmod-i2c-gpio kmod-rtc-pcf8563 \
	kmod-usb-serial kmod-usb-serial-cp210x uqmi
endef
TARGET_DEVICES += zbtlink_zbt-wd323
