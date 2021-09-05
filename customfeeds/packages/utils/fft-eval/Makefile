include $(TOPDIR)/rules.mk

PKG_NAME:=fft-eval
PKG_VERSION:=2019-11-27
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_VERSION:=4d3b6faee428e3bd9f44ab6a3d70585ec50484a1
PKG_SOURCE:=FFT_eval-$(PKG_SOURCE_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/simonwunderlich/FFT_eval/tar.gz/$(PKG_SOURCE_VERSION)?
PKG_HASH:=fb34ed5691af6c3ec09c41a6689ae64a1a12d770347a4effd48e4b30aef1eb22
PKG_BUILD_DIR:=$(BUILD_DIR)/FFT_eval-$(PKG_SOURCE_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	MAINTAINER:=Kirill Lukonin <klukonin@gmail.com>
	URL:=https://github.com/simonwunderlich/FFT_eval
	TITLE:=Evaluates FFT samples from ath9k, ath10k or ath11k driver
	DEPENDS:=@PACKAGE_ATH_DEBUG @PACKAGE_ATH_SPECTRAL
endef

define Package/$(PKG_NAME)/description
	Evaluates FFT samples from diferent wifi boards drivers
endef

TARGET_CFLAGS  += -Wall -W -std=gnu99 -fno-strict-aliasing -MD -MP
TARGET_CPPFLAGS += -D_DEFAULT_SOURCE


define Build/Compile
	$(call Build/Compile/Default,CONFIG_fft_eval_sdl=n)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fft_eval_json $(1)/usr/bin/fft_eval
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
