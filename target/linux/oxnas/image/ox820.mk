UBIFS_OPTS = -m 2048 -e 126KiB -c 4096
DEVICE_VARS += DTS UBIFS_OPTS
KERNEL_LOADADDR := 0x60008000

define Device/Default
  KERNEL_NAME := zImage
  KERNEL_SUFFIX := -uImage
  KERNEL_INSTALL := 1
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  SUBPAGESIZE := 512
  FILESYSTEMS := squashfs ubifs
  PROFILES = Default $$(DTS)
  KERNEL := kernel-bin | append-dtb | uImage none
  IMAGES := ubinized.bin sysupgrade.tar
  IMAGE/ubinized.bin := append-ubi
  IMAGE/sysupgrade.tar := sysupgrade-tar | append-metadata
endef

define Build/omninas-factory
	rm -rf $@.tmp $@.dummy $@.dummy.gz
	mkdir -p $@.tmp
	$(CP) $@ $@.tmp/uImage
	dd if=/dev/zero bs=64k count=4 of=$@.dummy
	gzip $@.dummy
	mkimage -A arm -T ramdisk -C gzip -n "dummy" \
		-d $@.dummy.gz \
		$@.tmp/rdimg.gz
	echo 2.35.20140102 > $@.tmp/version ; echo >> $@.tmp/version
	chmod 0744 $@.tmp/*
	$(TAR) -C $@.tmp -czvf $@ \
		$(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") .
endef

define Build/encrypt-3des
	openssl enc -des3 -a -k $(1) -in $@ -out $@.new && mv $@.new $@
endef

define Device/akitio-mycloud
  DEVICE_DTS := ox820-akitio-mycloud
  DEVICE_TITLE := Akition myCloud (mini) / SilverStone DC01
  DEVICE_PACKAGES := kmod-usb2-oxnas kmod-ata-oxnas-sata kmod-ledtrig-usbdev \
                     kmod-i2c-gpio kmod-rtc-ds1307
endef
TARGET_DEVICES += akitio-mycloud

define Device/cloudengines-pogoplug-pro
  DEVICE_DTS := ox820-cloudengines-pogoplug-pro
  DEVICE_TITLE := Cloud Engines PogoPlug Pro (with mPCIe)
  DEVICE_PACKAGES := kmod-usb2-oxnas kmod-ledtrig-usbdev
endef
TARGET_DEVICES += cloudengines-pogoplug-pro

define Device/cloudengines-pogoplug-series-3
  DEVICE_DTS := ox820-cloudengines-pogoplug-series-3
  DEVICE_TITLE := Cloud Engines PogoPlug Series V3 (without mPCIe)
  DEVICE_PACKAGES := kmod-usb2-oxnas kmod-ledtrig-usbdev
endef
TARGET_DEVICES += cloudengines-pogoplug-series-3

define Device/shuttle-kd20
  DEVICE_DTS := ox820-shuttle-kd20
  DEVICE_TITLE := Shuttle KD20
  KERNEL := kernel-bin | append-dtb | uImage none
  KERNEL_INITRAMFS_PREFIX = $$(IMAGE_PREFIX)-factory
  KERNEL_INITRAMFS_SUFFIX := .tar.gz
  KERNEL_INITRAMFS = kernel-bin | append-dtb | uImage none | omninas-factory | encrypt-3des sohmuntitnlaes
  DEVICE_PACKAGES := kmod-usb2-oxnas kmod-ata-oxnas-sata kmod-ledtrig-usbdev \
                     kmod-usb3 kmod-i2c-gpio kmod-rtc-pcf8563 kmod-gpio-beeper \
                     kmod-hwmon-core kmod-hwmon-gpiofan \
                     kmod-md-mod kmod-md-raid0 kmod-md-raid1 kmod-fs-ext4 kmod-fs-xfs
endef
TARGET_DEVICES += shuttle-kd20

define Device/mitrastar-stg212
  DEVICE_DTS := ox820-mitrastar-stg212
  DEVICE_TITLE := MitraStar STG-212
  KERNEL := kernel-bin | append-dtb | uImage none
  DEVICE_PACKAGES := kmod-usb2-oxnas kmod-ata-oxnas-sata kmod-ledtrig-usbdev
endef
TARGET_DEVICES += mitrastar-stg212
