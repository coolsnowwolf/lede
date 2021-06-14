include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/version.mk

PKG_NAME:=ipq-wifi
PKG_RELEASE:=1
PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile
endef

# Please send a mail with your device-specific board files upstream.
# You can find instructions and examples on the linux-wireless wiki:
# <https://wireless.wiki.kernel.org/en/users/drivers/ath10k/boardfiles>

ALLWIFIBOARDS:= \
	8dev_habanero-dvk \
	aruba_ap-303 \
	asus_rt-acrh17 \
	avm_fritzrepeater-1200 \
	buffalo_wtr-m2133hp \
	cellc_rtl30vw \
	century_wr142ac \
	dlink_dap2610 \
	edgecore_ecw5410 \
	engenius_eap2200 \
	engenius_emd1 \
	engenius_emr3500 \
	ezviz_cs-w3-wd1200g-eup \
	glinet_gl-s1300 \
	hiwifi_c526a \
	hugo_ac1200 \
	linksys_ea8300 \
	mobipromo_cm520-79f \
	nec_wg2600hp3 \
	p2w_r619ac \
	qxwlan_e2600ac

ALLWIFIPACKAGES:=$(foreach BOARD,$(ALLWIFIBOARDS),ipq-wifi-$(BOARD))

define Package/ipq-wifi-default
  SUBMENU:=ath10k Board-Specific Overrides
  SECTION:=firmware
  CATEGORY:=Firmware
  DEPENDS:=@(TARGET_ipq40xx||TARGET_ipq806x)
  TITLE:=Custom Board
endef

define ipq-wifi-install-one-to
  $(INSTALL_DIR)  $(2)/lib/firmware/ath10k/$(3)/
  $(INSTALL_DATA) $(1) $(2)/lib/firmware/ath10k/$(3)/board-2.bin
endef

define ipq-wifi-install-one
  $(if $(filter $(suffix $(1)),.QCA4019 .qca4019),\
    $(call ipq-wifi-install-one-to,$(1),$(2),QCA4019/hw1.0),\
  $(if $(filter $(suffix $(1)),.QCA9888 .qca9888),\
    $(call ipq-wifi-install-one-to,$(1),$(2),QCA9888/hw2.0),\
  $(if $(filter $(suffix $(1)),.QCA9984 .qca9984),\
    $(call ipq-wifi-install-one-to,$(1),$(2),QCA9984/hw1.0),\
    $(error Unrecognized board-file suffix '$(suffix $(1))' for '$(1)')\
  )))

endef
# Blank line required at end of above define due to foreach context

define generate-ipq-wifi-package
  define Package/ipq-wifi-$(1)
    $(call Package/ipq-wifi-default)
    TITLE:=board-2.bin Overrides for $(2)
    CONFLICTS:=$(PREV_BOARD)
  endef

  define Package/ipq-wifi-$(1)/description
The $(2) requires board-specific, reference ("cal") data
that is not yet present in the upstream wireless firmware distribution.

This package supplies board-2.bin file(s) that, in the interim,
overwrite those supplied by the ath10k-firmware-* packages.

This is package is only necessary for the $(2).

Do not install it for any other device!
  endef

  define Package/ipq-wifi-$(1)/install-overlay
    $$$$(foreach IPQ_WIFI_BOARD_FILE,$$$$(wildcard board-$(1).*),\
      $$$$(call ipq-wifi-install-one,$$$$(IPQ_WIFI_BOARD_FILE),$$(1)))
  endef

  PREV_BOARD+=ipq-wifi-$(1)
endef

# Add board name to ALLWIFIBOARDS
# Place files in this directory as board-<devicename>.<qca4019|qca9888|qca9984>
# Add $(eval $(call generate-ipq-wifi-package,<devicename>,<display name>))

$(eval $(call generate-ipq-wifi-package,8dev_habanero-dvk,8devices Habanero DVK))
$(eval $(call generate-ipq-wifi-package,aruba_ap-303,Aruba AP-303))
$(eval $(call generate-ipq-wifi-package,asus_rt-acrh17,ASUS RT-ACRH17/RT-AC82U/RT-AC42U))
$(eval $(call generate-ipq-wifi-package,avm_fritzrepeater-1200,AVM FRITZRepeater 1200))
$(eval $(call generate-ipq-wifi-package,buffalo_wtr-m2133hp,Buffalo WTR-M2133HP))
$(eval $(call generate-ipq-wifi-package,cellc_rtl30vw, Cell C RTL30VW))
$(eval $(call generate-ipq-wifi-package,century_wr142ac,Century WR142AC))
$(eval $(call generate-ipq-wifi-package,dlink_dap2610,D-Link DAP-2610))
$(eval $(call generate-ipq-wifi-package,edgecore_ecw5410,Edgecore ECW5410))
$(eval $(call generate-ipq-wifi-package,engenius_eap2200,EnGenius EAP2200))
$(eval $(call generate-ipq-wifi-package,engenius_emd1,EnGenius EMD1))
$(eval $(call generate-ipq-wifi-package,engenius_emr3500,EnGenius EMR3500))
$(eval $(call generate-ipq-wifi-package,ezviz_cs-w3-wd1200g-eup,EZVIZ CS-W3-WD1200G EUP))
$(eval $(call generate-ipq-wifi-package,glinet_gl-s1300,GL.iNet GL-S1300))
$(eval $(call generate-ipq-wifi-package,hiwifi_c526a,HiWiFi C526A))
$(eval $(call generate-ipq-wifi-package,hugo_ac1200,Hugo AC1200))
$(eval $(call generate-ipq-wifi-package,linksys_ea8300,Linksys EA8300))
$(eval $(call generate-ipq-wifi-package,mobipromo_cm520-79f,MobiPromo CM520-79F))
$(eval $(call generate-ipq-wifi-package,nec_wg2600hp3,NEC Platforms WG2600HP3))
$(eval $(call generate-ipq-wifi-package,p2w_r619ac,P&W R619AC))
$(eval $(call generate-ipq-wifi-package,qxwlan_e2600ac,Qxwlan E2600AC))

$(foreach PACKAGE,$(ALLWIFIPACKAGES),$(eval $(call BuildPackage,$(PACKAGE))))
