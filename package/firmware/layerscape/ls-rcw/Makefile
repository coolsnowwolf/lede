#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ls-rcw
PKG_VERSION:=LSDK-20.04
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://source.codeaurora.org/external/qoriq/qoriq-components/rcw
PKG_SOURCE_VERSION:=5689bf9c9f087f50aaa0d91b43d8a791fedbedd3
PKG_MIRROR_HASH:=45709aa92f84fea8100b472a29762bfe69a333e6aca7a4fc4edc8cc3e10bf44e

include $(INCLUDE_DIR)/package.mk

define Package/layerscape-rcw
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=NXP Layerscape RCW binaries
  DEPENDS:=@TARGET_layerscape
endef

BOARDS := \
  ls1012ardb \
  ls1012afrdm \
  ls1012afrwy \
  ls1043ardb \
  ls1043ardb-sdboot \
  ls1046ardb \
  ls1046ardb-sdboot \
  ls1088ardb \
  ls1088ardb-sdboot \
  ls2088ardb \
  ls1021atwr

RCW_ls1012ardb        :=ls1012ardb/R_SPNH_3508/rcw_1000_default.bin
RCW_ls1012afrdm       :=ls1012afrdm/N_SSNP_3305/rcw_800.bin
RCW_ls1012afrwy       :=ls1012afrwy/N_SSNP_3305/rcw_1000_default.bin
RCW_ls1043ardb        :=ls1043ardb/RR_FQPP_1455/rcw_1600.bin
RCW_ls1043ardb-sdboot :=ls1043ardb/RR_FQPP_1455/rcw_1600_sdboot.bin
RCW_ls1046ardb        :=ls1046ardb/RR_FFSSPPPH_1133_5559/rcw_1800_qspiboot.bin
RCW_ls1046ardb-sdboot :=ls1046ardb/RR_FFSSPPPH_1133_5559/rcw_1800_sdboot.bin
RCW_ls1088ardb        :=ls1088ardb/FCQQQQQQQQ_PPP_H_0x1d_0x0d/rcw_1600_qspi.bin
RCW_ls1088ardb-sdboot :=ls1088ardb/FCQQQQQQQQ_PPP_H_0x1d_0x0d/rcw_1600_sd.bin
RCW_ls2088ardb        :=ls2088ardb/FFFFFFFF_PP_HH_0x2a_0x41/rcw_1800.bin
RCW_ls1021atwr        :=ls1021atwr/SSR_PNS_30/rcw_1200.bin

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(foreach board,$(BOARDS), \
		$(CP) $(PKG_BUILD_DIR)/$(RCW_$(board)) $(STAGING_DIR_IMAGE)/$(board)-rcw.bin;)
endef

$(eval $(call BuildPackage,layerscape-rcw))
