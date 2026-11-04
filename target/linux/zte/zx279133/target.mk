# SPDX-License-Identifier: GPL-2.0-only
ARCH:=aarch64
SUBTARGET:=zx279133
BOARDNAME:=ZX279133 based boards
CPU_TYPE:=cortex-a53
KERNELNAME:=Image dtbs

# Stock LEDE LuCI and network services; no external application feed required.
DEFAULT_PACKAGES += \
	kmod-zte-npu kmod-zte-fnoffload kmod-zte-rtl8372n \
	dnsmasq firewall4 nftables-json kmod-nft-offload \
	ppp ppp-mod-pppoe odhcp6c odhcpd-ipv6only \
	luci uhttpd uhttpd-mod-ubus luci-app-zte-fwslot

define Target/Description
	ZTE ZX279133 based routers.
endef
