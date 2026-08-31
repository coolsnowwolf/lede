DTS_DIR := $(DTS_DIR)/microchip

define Build/lan969x-fip
	cat $(STAGING_DIR_IMAGE)/$1-fip.bin >> $@
endef

define Build/lan969x-gpt-emmc
	cp $@ $@.tmp 2>/dev/null || true
	ptgen -g -o $@.tmp -a 1 -l 1024 \
			-t 0x83	-N fip		-r	-p 8M@1M \
			-t 0x83	-N fip.bak	-r	-p 8M@9M \
			-t 0x83	-N Env		-r	-p 2M@17M \
		$(if $(findstring flash,$1), \
			-t 0x2e -N kernel		-p $(CONFIG_TARGET_KERNEL_PARTSIZE)M@19M \
			-t 0x2e -N rootfs		-p $(CONFIG_TARGET_ROOTFS_PARTSIZE)M
		)
	cat $@.tmp >> $@
	rm $@.tmp
endef

define Build/tactical-1000-gpt-emmc
	cp $@ $@.tmp 2>/dev/null || true
	ptgen -g -o $@.tmp -a 1 -l 1024 \
			-t 0x83	-N fip		-r	-p 128M@1M \
			-t 0x83	-N fip.bak	-r	-p 128M@129M \
			-t 0x83	-N Env		-r	-p 2M@257M \
		$(if $(findstring flash,$1), \
			-t 0x2e -N kernel		-p $(CONFIG_TARGET_KERNEL_PARTSIZE)M@259M \
			-t 0x2e -N rootfs		-p $(CONFIG_TARGET_ROOTFS_PARTSIZE)M
		)
	cat $@.tmp >> $@
	rm $@.tmp
endef

define Build/append-kernel-part
	dd if=$(IMAGE_KERNEL) bs=$(CONFIG_TARGET_KERNEL_PARTSIZE)M conv=sync >> $@
endef

define Device/microchip_ev23x71a
	$(call Device/FitImage)
	DEVICE_VENDOR := Microchip
	DEVICE_MODEL := EV23X71A
	SOC := lan9696
	DEVICE_DTS_DIR := $(DTS_DIR)
	DEVICE_PACKAGES := kmod-i2c-mux-gpio
	IMAGES += emmc-atf-gpt.gz emmc-gpt.img.gz
	IMAGE/emmc-atf-gpt.gz := lan969x-gpt-emmc |\
		pad-to 1M | lan969x-fip ev23x71a |\
		pad-to 9M | lan969x-fip ev23x71a |\
		gzip
	IMAGE/emmc-gpt.img.gz := lan969x-gpt-emmc flash |\
		pad-to 1M | lan969x-fip ev23x71a |\
		pad-to 9M | lan969x-fip ev23x71a |\
		pad-to 19M | append-kernel-part |\
		append-rootfs |\
		gzip
endef
TARGET_DEVICES += microchip_ev23x71a

define Device/novarq_tactical-1000
	$(call Device/FitImage)
	DEVICE_VENDOR := Novarq
	DEVICE_MODEL := Tactical 1000
	SOC := lan9696
	DEVICE_PACKAGES := kmod-i2c-mux-gpio \
		kmod-gpio-pwm kmod-hwmon-pwmfan kmod-hwmon-gpiofan \
		kmod-rtc-ds1307 kmod-hwmon-lm75
	IMAGES += emmc-gpt-table.gz emmc-atf-gpt.gz emmc-gpt.img.gz
	IMAGE/emmc-gpt-table.gz := tactical-1000-gpt-emmc | gzip
	IMAGE/emmc-atf-gpt.gz := tactical-1000-gpt-emmc |\
		pad-to 1M | lan969x-fip tactical-1000 |\
		pad-to 129M | lan969x-fip tactical-1000 |\
		gzip
	IMAGE/emmc-gpt.img.gz := tactical-1000-gpt-emmc flash |\
		pad-to 1M | lan969x-fip tactical-1000 |\
		pad-to 129M | lan969x-fip tactical-1000 |\
		pad-to 259M | append-kernel-part |\
		append-rootfs |\
		gzip
	SUPPORTED_DEVICES += novarq,tactical-1000-v3
endef
TARGET_DEVICES += novarq_tactical-1000
