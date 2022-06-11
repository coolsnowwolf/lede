#
# Copyright © 2020 David Woodhouse <dwmw2@infradead.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mt7623n-preloader
PKG_VERSION:=2020-03-11
PKG_RELEASE:=b27114e184449a33b5d875fda14198f5e6fee2bb

PKG_MAINTAINER:=David Woodhouse <dwmw2@infradead.org>

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)/$(PKG_NAME)-$(PKG_RELEASE)

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

BPI_PRELOADER_URL:=@GITHUB/BPI-SINOVOIP/BPI-files/$(PKG_RELEASE)/SD/100MB/
BPI_PRELOADER_PREFIX:=bpi-preloader-$(PKG_RELEASE)

define Download/BPI-R2-preloader-2k.img.gz
  FILE:=$(BPI_PRELOADER_PREFIX)-BPI-R2-preloader-DDR1600-20191024-2k.img.gz
  URL:=$(BPI_PRELOADER_URL)
  URL_FILE:=BPI-R2-preloader-DDR1600-20191024-2k.img.gz
  HASH:=c731cc166c912c84846e2ed5faf727504e4dec1463754baa6328e9908c84a373
endef
$(eval $(call Download,BPI-R2-preloader-2k.img.gz))

define Download/BPI-R64-preloader-2k.img.gz
  FILE:=$(BPI_PRELOADER_PREFIX)-BPI-R64-preloader-2k.img.gz
  URL:=$(BPI_PRELOADER_URL)
  URL_FILE:=BPI-R64-preloader-2k.img.gz
  HASH:=1a4b55da1717190aa4e790ce93850605e9b15aae4c3248bcf8734aac020ab0e4
endef
$(eval $(call Download,BPI-R64-preloader-2k.img.gz))


define Package/mt7623n-preloader
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_mediatek_mt7623
  TITLE:=mt7623n-preloader
  DEFAULT:=y if TARGET_mediatek
endef

define Package/mt7623n-preloader/description
  Preloader image for mt7623n based boards like Banana Pi R2.
endef

define Build/Prepare
	rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	cp $(DL_DIR)/$(BPI_PRELOADER_PREFIX)-BPI-R2-preloader-DDR1600-20191024-2k.img.gz $(PKG_BUILD_DIR)/mt7623n_bpir2-preloader.bin.gz
endef

define Build/Compile
	true
endef

define Build/InstallDev
	mkdir -p $(STAGING_DIR_IMAGE)
	gunzip -c $(PKG_BUILD_DIR)/mt7623n_bpir2-preloader.bin.gz > $(STAGING_DIR_IMAGE)/mt7623n_bpir2-preloader.bin
endef

$(eval $(call BuildPackage,mt7623n-preloader))
