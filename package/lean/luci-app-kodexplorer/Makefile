# Copyright (C) 2018-2022 Lienol <lawlienol@gmail.com>
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for KodExplorer
LUCI_DEPENDS:=+nginx-ssl +unzip +zoneinfo-asia +php8 +php8-fastcgi +php8-fpm +php8-mod-curl +php8-mod-dom +php8-mod-gd +php8-mod-iconv +php8-mod-mbstring +php8-mod-opcache +php8-mod-session +php8-mod-zip +php8-mod-sqlite3 +php8-mod-pdo +php8-mod-pdo-sqlite +php8-mod-pdo-mysql +php8-mod-xml +php8-mod-xmlreader +php8-mod-xmlwriter
LUCI_PKGARCH:=all
PKG_VERSION:=20220109
PKG_RELEASE:=1

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
