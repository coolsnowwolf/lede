ARCH:=aarch64
CPU_TYPE:=cortex-a53
BOARDNAME:=Amlogic S905x boards (64 bit)

DEFAULT_PACKAGES+= ethtool parted

define Target/Description
	Build firmware image for Amlogic S905x devices.
	This firmware features a 64 bit kernel.
endef
