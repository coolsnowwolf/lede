# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (C) 2019 ElonH <elonhhuang@gmail.com>

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-rclone
PKG_VERSION:=1.4.1
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3.0+
PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_rclone-webui \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_rclone-ng

LUCI_TITLE:=LuCI support for Rclone
LUCI_PKGARCH:=all
LUCI_DEPENDS:=+rclone \
	+PACKAGE_$(PKG_NAME)_INCLUDE_rclone-webui:rclone-webui-react \
	+PACKAGE_$(PKG_NAME)_INCLUDE_rclone-ng:rclone-ng
		
define Package/$(PKG_NAME)/conffiles
/etc/config/rclone
endef

define Package/$(PKG_NAME)/config
config PACKAGE_$(PKG_NAME)_INCLUDE_rclone-webui
	bool "Include rclone-webui"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_rclone-ng
	bool "Include rclone-ng (another webui)"
	default y
endef

define Package/$(PKG_NAME)/conffiles
/etc/config/rclone
endef

define Package/$(PKG_NAME)/description
	LuCI support for Rclone. 
	Rclone ("rsync for cloud storage") is a command line program to sync root/usr/bin and directories to and from different cloud storage providers.
	Cloud storage providers:
		1Fichier, Alibaba Cloud (Aliyun) Object Storage System (OSS), Amazon Drive, Amazon S3, 
		Backblaze B2, Box, Ceph, C14, DigitalOcean Spaces, Dreamhost, Dropbox, FTP, 
		Google Cloud Storage, Google Drive, Google Photos, HTTP, Hubic, Jottacloud, 
		IBM COS S3, Koofr, Memset Memstore, Mega, Microsoft Azure Blob Storage, 
		Microsoft OneDrive, Minio, Nextcloud, OVH, OpenDrive, Openstack Swift, 
		Oracle Cloud Storage, ownCloud, pCloud, premiumize.me, put.io, QingStor, 
		Rackspace Cloud root/usr/bin, rsync.net, Scaleway, SFTP, Wasabi, WebDAV, 
		Yandex Disk, The local root/usr/binystem.
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
