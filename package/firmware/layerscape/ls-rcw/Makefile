#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ls-rcw
PKG_VERSION:=lsdk-1806
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://source.codeaurora.org/external/qoriq/qoriq-components/rcw
PKG_SOURCE_VERSION:=1f43bef4b4475d8e81c9b3b8c5bdd6c1ce8cfa6c
PKG_MIRROR_HASH:=a3b455bcc1f070c8e51f520e32b62cec5b05d772d7ae4a876d88fd5f278a7b1c

include $(INCLUDE_DIR)/package.mk

define Package/layerscape-rcw/Config
  define Package/layerscape-rcw-$(1)
    SECTION:=firmware
    CATEGORY:=Firmware
    TITLE:=$(2)
    DEPENDS:=@TARGET_layerscape
    CONFIG:=$(3)
  endef
endef

define Package/layerscape-rcw/ls1012ardb
  TITLE:=NXP LS1012ARDB RCW binary
  CONFIG:=ls1012ardb/R_SPNH_3508/rcw_1000_default.bin.swapped
endef

define Package/layerscape-rcw/ls1012afrwy
  TITLE:=NXP LS1012AFRWY RCW binary
  CONFIG:=ls1012afrwy/N_SSNP_3305/rcw_1000_default.bin.swapped
endef

define Package/layerscape-rcw/ls1043ardb
  TITLE:=NXP LS1043ARDB RCW binary
  CONFIG:=ls1043ardb/RR_FQPP_1455/rcw_1600.bin
endef

define Package/layerscape-rcw/ls1046ardb
  TITLE:=NXP LS1046ARDB RCW binary
  CONFIG:=ls1046ardb/RR_FFSSPPPH_1133_5559/rcw_1800_qspiboot.bin.swapped
endef

define Package/layerscape-rcw/ls1088ardb
  TITLE:=NXP LS1088ARDB RCW binary
  CONFIG:=ls1088ardb/FCQQQQQQQQ_PPP_H_0x1d_0x0d/rcw_1600_qspi.bin
endef

define Package/layerscape-rcw/ls1088ardb-sdboot
  TITLE:=NXP LS1088ARDB SD card boot RCW binary
  CONFIG:=ls1088ardb/FCQQQQQQQQ_PPP_H_0x1d_0x0d/rcw_1600_sd.bin
endef

define Package/layerscape-rcw/ls2088ardb
  TITLE:=NXP LS2088ARDB RCW binary
  CONFIG:=ls2088ardb/FFFFFFFF_PP_HH_0x2a_0x41/rcw_1800.bin
endef

define Package/layerscape-rcw/ls1021atwr
  TITLE:=NXP LS1021ATWR RCW binary
  CONFIG:=ls1021atwr/SSR_PNS_30/rcw_1200.bin
endef

define Package/layerscape-rcw/Install
  define Package/layerscape-rcw-$(1)/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(CONFIG) $(STAGING_DIR_IMAGE)/$(1)-rcw.bin
  endef
endef

RCWS := \
  ls1012ardb \
  ls1012afrwy \
  ls1043ardb \
  ls1046ardb \
  ls1088ardb \
  ls1088ardb-sdboot \
  ls2088ardb \
  ls1021atwr

$(foreach rcw,$(RCWS), \
	$(eval $(Package/layerscape-rcw/$(rcw))) \
	$(eval $(call Package/layerscape-rcw/Config,$(rcw),$(TITLE),$(CONFIG))) \
	$(eval $(call Package/layerscape-rcw/Install,$(rcw))) \
	$(eval $(call BuildPackage,layerscape-rcw-$(rcw))) \
)
