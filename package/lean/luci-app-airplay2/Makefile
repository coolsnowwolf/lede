#
# Copyright (C) 2008-2014 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=Luci for Apple Airplay2 Receiver
LUCI_DEPENDS:=@TARGET_x86 +alsa-utils +kmod-sound-hda-core +kmod-sound-hda-codec-realtek +kmod-sound-hda-codec-via +kmod-sound-via82xx +kmod-sound-hda-intel +shairport-sync-openssl
LUCI_PKGARCH:=all
PKG_VERSION:=1
PKG_RELEASE:=9

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature

