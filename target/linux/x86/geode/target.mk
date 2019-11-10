BOARDNAME:=AMD Geode based systems
FEATURES:=squashfs ext4 pci usb gpio
DEFAULT_PACKAGES += \
			kmod-crypto-hw-geode kmod-crypto-cbc \
			kmod-ath5k kmod-ath9k \
			kmod-button-hotplug \
			kmod-ledtrig-heartbeat kmod-ledtrig-gpio \
			kmod-ledtrig-netdev hwclock wpad-basic
# Geos
DEFAULT_PACKAGES += \
		soloscli linux-atm br2684ctl ppp-mod-pppoa pppdump pppstats \
		hwclock flashrom tc kmod-pppoa kmod-8139cp kmod-mppe \
		kmod-usb-ohci-pci kmod-hwmon-lm90

define Target/Description
	Build firmware images for AMD Geode GX/LX based systems (net5501, alix, geos)
endef
