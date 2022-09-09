# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

BOARD:=rockchip
BOARDNAME:=Rockchip
FEATURES:=ext4 audio usb usbgadget display gpio fpu pci pcie rootfs-part boot-part squashfs
SUBTARGETS:=armv8

KERNEL_PATCHVER=5.15
KERNEL_TESTING_PATCHVER=5.19

define Target/Description
	Build firmware image for Rockchip SoC devices.
endef

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += uboot-envtools partx-utils e2fsprogs mkf2fs kmod-button-hotplug kmod-gpio-button-hotplug kmod-fuse \
		    kmod-usb-net-rndis kmod-usb-net-cdc-ether kmod-usb-net-ipheth usbmuxd libimobiledevice kmod-usb-net-asix-ax88179 kmod-usb-net-rtl8152-vendor \
		    iptables-mod-conntrack-extra iptables-mod-ipopt haveged libmbedtls htop autocore-arm automount autosamba iperf3 ipv6helper usb-modeswitch pciutils clash \
		    luci-theme-argon-mod luci-app-diskman hysteria luci-app-openclash luci-proto-bonding luci-app-cpufreq luci-app-usb-printer usbutils \
		    luci-app-adbyby-plus luci-app-hd-idle luci-app-ipsec-server luci-app-openvpn-server luci-app-pptp-server luci-app-zerotier luci-app-wireguard \
		    luci-app-sqm luci-app-unblockmusic luci-app-ramfree luci-app-ttyd 

KERNELNAME:=Image dtbs

$(eval $(call BuildTarget))
