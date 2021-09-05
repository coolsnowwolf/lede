# Copyright (C) 2019 Valentín Kivachuk <vk18496@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tessdata
PKG_VERSION:=4.1.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/tesseract-ocr/tessdata/tar.gz/$(PKG_VERSION)?
PKG_HASH:=990fffb9b7a9b52dc9a2d053a9ef6852ca2b72bd8dfb22988b0b990a700fd3c7

PKG_MAINTAINER:=Valentín Kivachuk <vk18496@gmail.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=COPYING

#No need to extract 1,5GB...
PKG_UNPACK:=

include $(INCLUDE_DIR)/package.mk

ALLTESSERACTLANG:=afr amh ara asm aze aze_cyrl bel ben bod bos bre bul cat ceb ces chi_sim chi_sim_vert chi_tra chi_tra_vert chr cos cym dan dan_frak deu deu_frak div dzo ell eng enm epo equ est eus fao fas fil fin fra frk frm fry gla gle glg grc guj hat heb hin hrv hun hye iku ind isl ita ita_old jav jpn jpn_vert kan kat kat_old kaz khm kir kmr kor kor_vert lao lat lav lit ltz mal mar mkd mlt mon mri msa mya nep nld nor oci ori osd pan pol por pus que ron rus san sin slk slk_frak slv snd spa spa_old sqi srp srp_latn sun swa swe syr tam tat tel tgk tgl tha tir ton tur uig ukr urd uzb uzb_cyrl vie yid yor


define Build/Compile
endef

define Build/Install
endef


define Package/tesseract-data-default
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=tesseract
endef

define generate-tesseract-data-package
  define Package/tesseract-data-$(1)
    TITLE:=Training data for $(1) language
    $(call Package/tesseract-data-default)
  endef

  define Package/tesseract-data-$(1)/install
	$(INSTALL_DIR) $$(1)/usr/share/tessdata
	$(TAR) --strip=1 -C $$(1)/usr/share/tessdata/ -xvf $(DL_DIR)/$(PKG_NAME)-$(PKG_VERSION).tar.gz $(PKG_NAME)-$(PKG_VERSION)/$(1).traineddata
  endef

endef


$(foreach LANG,$(ALLTESSERACTLANG),$(eval $(call generate-tesseract-data-package,$(LANG))))
$(foreach LANG,$(ALLTESSERACTLANG),$(eval $(call BuildPackage,tesseract-data-$(LANG))))
