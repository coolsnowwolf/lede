define Device/generic
  DEVICE_TITLE := Generic x86/Geode
  DEVICE_PACKAGES += kmod-crypto-cbc kmod-crypto-hw-geode kmod-ledtrig-gpio \
	kmod-ledtrig-heartbeat kmod-ledtrig-netdev
  GRUB2_VARIANT := legacy
endef
TARGET_DEVICES += generic

define Device/geos
  $(call Device/generic)
  DEVICE_TITLE := Traverse Technologies Geos
  DEVICE_PACKAGES += br2684ctl flashrom kmod-hwmon-lm90 kmod-mppe kmod-pppoa \
	kmod-usb-ohci-pci linux-atm ppp-mod-pppoa pppdump pppstats soloscli tc
endef
TARGET_DEVICES += geos
