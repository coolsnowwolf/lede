####
 #  File: /Makefile
 #  Project: rclone
 #  File Created: Friday, 11th October 2019 4:50:49 pm
 #  Author: ElonH[EH](elonhhuang@gmail.com)
 #  License: GNU General Public License v3.0 or later(http://www.gnu.org/licenses/gpl-3.0-standalone.html)
 #  Copyright (C) 2019 [ElonH]
####

include $(TOPDIR)/rules.mk

PKG_NAME:=rclone
PKG_VERSION:=1.52.2
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/rclone/rclone.git
PKG_SOURCE_DATE:=2020-06-24
PKG_SOURCE_VERSION:=d8144a7e84dc3fb4975adf1bce707a5be672fada
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_MIRROR_HASH:=8a6f3fd465e57f5f3b9cfe3958eeb020f72b4940bfbf4993b9580659041c3735

PKG_LICENSE:=GPLv3
PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0 # https://github.com/openwrt/packages/issues/8498
GO_PKG:=github.com/rclone/rclone
GO_PKG_EXCLUDES:=test

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

# GO_PKG_LDFLAGS:=
GO_PKG_LDFLAGS_X:=\
	github.com/rclone/rclone/fs.Version=v$(PKG_VERSION)_$(PKG_SOURCE_DATE)\
	main.Version=v$(PKG_VERSION) \
	main.BuildUser=openwrt \
	main.BuildHost=openwrt \
	main.BuildStamp=$(SOURCE_DATE_EPOCH)

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=rsync for cloud storage.
	URL:=https://rclone.org
  	DEPENDS:=$(GO_ARCH_DEPENDS) 
endef

define Package/$(PKG_NAME)/description
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

define Package/$(PKG_NAME)/install
	# echo "++++++++++++++++++"
	# echo "$(PKG_INSTALL_DIR)"
	# echo "$(1)"
	# echo "$(GO_PKG_BUILD_BIN_DIR)"
	# echo "++++++++++++++++++"

	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/rclone $(1)/usr/bin/
endef

$(eval $(call GoBinPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
