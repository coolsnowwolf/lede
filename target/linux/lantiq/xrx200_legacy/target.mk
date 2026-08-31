ARCH:=mips
SUBTARGET:=xrx200_legacy
BOARDNAME:=XRX200 Legacy
FEATURES+=atm ramdisk small_flash
CPU_TYPE:=24kc

DEFAULT_PACKAGES+=kmod-leds-gpio \
	kmod-gpio-button-hotplug \
	ltq-vdsl-vr9-vectoring-fw-installer \
	kmod-ltq-vdsl-vr9-mei \
	kmod-ltq-vdsl-vr9 \
	kmod-ltq-atm-vr9 \
	kmod-ltq-deu-vr9 \
	kmod-ltq-ptm-vr9 \
	ltq-vdsl-vr9-app \
	kmod-dsa-gswip \
	dsl-vrx200-firmware-xdsl-a \
	dsl-vrx200-firmware-xdsl-b-patch \
	ppp-mod-pppoa

define Target/Description
	Lantiq XRX200 Legacy for boards with small flash
endef
