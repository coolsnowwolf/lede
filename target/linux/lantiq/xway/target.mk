ARCH:=mips
SUBTARGET:=xway
BOARDNAME:=XWAY
FEATURES+=atm nand ramdisk
CPU_TYPE:=24kc

DEFAULT_PACKAGES+=kmod-leds-gpio kmod-gpio-button-hotplug swconfig

define Target/Description
	Lantiq XWAY
endef
