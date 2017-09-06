ARCH:=mips
SUBTARGET:=xrx200
BOARDNAME:=XRX200
FEATURES:=squashfs atm nand ubifs
CPU_TYPE:=24kc

DEFAULT_PACKAGES+=kmod-leds-gpio \
	kmod-gpio-button-hotplug \
	kmod-ltq-vdsl-vr9-mei \
	kmod-ltq-vdsl-vr9 \
	kmod-ltq-atm-vr9 \
	kmod-ltq-ptm-vr9 \
	kmod-ltq-deu-vr9 \
	ltq-vdsl-app \
	dsl-vrx200-firmware-xdsl-a \
	dsl-vrx200-firmware-xdsl-b-patch \
	ppp-mod-pppoa \
	swconfig

define Target/Description
	Lantiq XRX200
endef
