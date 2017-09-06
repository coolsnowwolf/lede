ARCH:=mips
SUBTARGET:=falcon
BOARDNAME:=Falcon
FEATURES:=squashfs nand
CPU_TYPE:=24kc

DEFAULT_PACKAGES+= kmod-leds-gpio \
	kmod-gpio-button-hotplug \
	kmod-ledtrig-heartbeat

define Target/Description
	Lantiq Falcon
endef
