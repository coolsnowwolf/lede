define Build/MultiImage
        rm -rf $@.fakerd $@.new

        dd if=/dev/zero of=$@.fakerd bs=32 count=1 conv=sync

        -$(STAGING_DIR_HOST)/bin/mkimage -A $(LINUX_KARCH) -O linux -T multi -C $(1)  \
		-a $(KERNEL_LOADADDR) -e $(KERNEL_ENTRY) -n '$(BOARD_NAME) initramfs' \
		-d $@:$@.fakerd:$(KDIR)/image-$(firstword $(DEVICE_DTS)).dtb $@.new
        mv $@.new $@
        rm -rf $@.fakerd
endef

define Device/aerohive_hiveap-330
  DEVICE_VENDOR := Aerohive
  DEVICE_MODEL := HiveAP-330
  DEVICE_ALT0_VENDOR := Aerohive
  DEVICE_ALT0_MODEL := HiveAP-350
  DEVICE_PACKAGES := kmod-tpm-i2c-atmel kmod-hwmon-lm70 kmod-phy-at803x
  BLOCKSIZE := 128k
  KERNEL := kernel-bin | uImage none
  KERNEL_INITRAMFS := kernel-bin | uImage none
  KERNEL_NAME := simpleImage.hiveap-330
  KERNEL_SIZE := 16m
  IMAGES := sysupgrade.bin
  KERNEL_ENTRY := 0x1500000
  KERNEL_LOADADDR := 0x1500000
  # append-dtb is still needed, as otherwise u-boot bootm complains
  # about not having a FDT to edit.
  IMAGE/sysupgrade.bin := append-dtb | pad-to 256k | append-kernel | \
	append-rootfs | pad-rootfs | check-size | append-metadata
  IMAGE_SIZE = 63m
  DEVICE_COMPAT_VERSION := 2.0
  DEVICE_COMPAT_MESSAGE := \n$\
    !The partitioning of the HiveAP 330 has changed! \n$\
    To upgrade, please take a look at the install instructions over \
    at the device's wiki: <https://openwrt.org/toh/aerohive/hiveap-330> \n$\
    An abridged version for the console is provided here for comfort. \n$\
    Run the following script into a shell on the device and retry this \
    sysupgrade again: \n$\
    cat <<- "EOF" > /tmp/uboot-fix.sh; sh /tmp/uboot-fix.sh \n$\
    . /lib/functions.sh \n$\
    . /lib/functions/system.sh \n$\
    opkg update && opkg install uboot-envtools kmod-mtd-rw || exit 2 \n$\
    insmod mtd-rw i_want_a_brick=y || exit 3 \n$\
    echo "/dev/mtd$$$$(find_mtd_index u-boot-env) 0x0 0x20000 0x10000" > "/etc/fw_env.config" \n$\
    fw_setenv owrt_boot 'setenv bootargs console=ttyS0,9600;bootm 0xEC040000 - 0xEC000000' \n$\
    cp "/dev/mtd$$$$(find_mtd_index 'u-boot')" /tmp/uboot \n$\
    cp /tmp/uboot /tmp/uboot_patched \n$\
    strings -td < /tmp/uboot | grep '^ *[0-9]* *\\(run owrt_boot\\|setenv bootargs\\).*cp\\.l' | \n$\
       awk '{print $$$$1}' | \n$\
       while read offset; do \n$\
         echo -n "run owrt_boot;            " | dd of=/tmp/uboot_patched bs=1 seek=$$$${offset} conv=notrunc \n$\
       done \n$\
       mtd write /tmp/uboot_patched u-boot \n$\
       uci set system.@system[0].compat_version=2.0; uci commit; \n$\
    EOF \n$\
    \n$\
    Note that if this fails, you will need to use the serial console \n$\
    to re-install OpenWrt. \n$\
    Note that after this sysupgrade, the AP will be unavailable for 7 \n$\
    minutes to reformat flash."

endef
TARGET_DEVICES += aerohive_hiveap-330

define Device/enterasys_ws-ap3710i
  DEVICE_VENDOR := Enterasys
  DEVICE_MODEL := WS-AP3710i
  DEVICE_PACKAGES := kmod-phy-at803x
  BLOCKSIZE := 128k
  KERNEL_NAME := simpleImage.ws-ap3710i
  KERNEL_ENTRY := 0x3000000
  KERNEL_LOADADDR := 0x3000000
  KERNEL = kernel-bin | uImage none
  KERNEL_INITRAMFS := kernel-bin | uImage none
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += enterasys_ws-ap3710i

define Device/extreme-networks_ws-ap3825i
  DEVICE_VENDOR := Extreme Networks
  DEVICE_MODEL := WS-AP3825i
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct kmod-phy-at803x
  BLOCKSIZE := 128k
  KERNEL_NAME := simpleImage.ws-ap3825i
  KERNEL_ENTRY := 0x3000000
  KERNEL_LOADADDR := 0x3000000
  KERNEL = kernel-bin | fit none $(KDIR)/image-$$(DEVICE_DTS).dtb
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += extreme-networks_ws-ap3825i

define Device/hpe_msm460
  DEVICE_VENDOR := Hewlett-Packard
  DEVICE_MODEL := MSM460
  DEVICE_ALT0_VENDOR := Hewlett-Packard
  DEVICE_ALT0_MODEL := MSM430
  DEVICE_ALT1_VENDOR := Hewlett-Packard
  DEVICE_ALT1_MODEL := MSM466
  DEVICE_PACKAGES := kmod-phy-marvell
  KERNEL = kernel-bin | fit none $(KDIR)/image-$$(DEVICE_DTS).dtb
  KERNEL_NAME := zImage.la3000000
  KERNEL_ENTRY := 0x3000000
  KERNEL_LOADADDR := 0x3000000
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  SUBPAGESIZE := 2048
  KERNEL_IN_UBI := 1
  UBINIZE_OPTS := -E 5
  IMAGES := factory.bin sysupgrade.bin
  IMAGE/factory.bin := append-ubi
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += hpe_msm460

define Device/ocedo_panda
  DEVICE_VENDOR := OCEDO
  DEVICE_MODEL := Panda
  DEVICE_PACKAGES := kmod-rtc-ds1307 kmod-dsa-b53-mdio kmod-phy-broadcom
  KERNEL = kernel-bin | libdeflate-gzip | fit gzip $(KDIR)/image-$$(DEVICE_DTS).dtb
  PAGESIZE := 2048
  SUBPAGESIZE := 512
  BLOCKSIZE := 128k
  IMAGES := fdt.bin sysupgrade.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/fdt.bin := append-dtb
  DEVICE_COMPAT_VERSION := 1.1
  DEVICE_COMPAT_MESSAGE := Config cannot be migrated from swconfig to DSA
endef
TARGET_DEVICES += ocedo_panda
