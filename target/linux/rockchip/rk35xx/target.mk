ARCH:=aarch64
BOARDNAME:=RK3528/RK3576/RK3588 boards (64 bit)
FEATURES+=pwm emmc nand

KERNEL_PATCHVER=5.10

DEFAULT_PACKAGES+=kmod-r8125 ethtool

define Target/Description
	Build firmware image for Rockchip RK3528/RK3576/RK3588 devices.
	This firmware features a 64 bit kernel.
endef
