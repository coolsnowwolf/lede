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

# Use ath10k-bdencoder from https://github.com/qca/qca-swiss-army-knife.git
# to generate the board-* files here.
#
# This is intended to be used on an interim basis until device-specific
# board data for new devices is available through the upstream compilation
#
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
	devolo_magic-2-wifi-next \
	dlink_dap2610 \
	edgecore_ecw5410 \
	edgecore_oap100 \
	engenius_eap2200 \
	engenius_emd1 \
	engenius_emr3500 \
	ezviz_cs-w3-wd1200g-eup \
	glinet_gl-ap1300 \
	glinet_gl-s1300 \
	hiwifi_c526a \
	linksys_ea8300 \
	linksys_mr8300-v0 \
	luma_wrtq-329acn \
	mikrotik_hap-ac2 \
	mikrotik_sxtsq-5-ac \
	mobipromo_cm520-79f \
	nec_wg2600hp3 \
	netgear_sxr80 \
	netgear_wac510 \
	plasmacloud_pa1200 \
	plasmacloud_pa2200 \
	p2w_r619ac \
	qxwlan_e2600ac \
	redmi_ax6 \
	xiaomi_ax3600 \
	xiaomi_ax9000 \
	zte_mf263 \
	zte_mf269

ALLWIFIPACKAGES:=$(foreach BOARD,$(ALLWIFIBOARDS),ipq-wifi-$(BOARD))

define Package/ipq-wifi-default
  SUBMENU:=ath10k Board-Specific Overrides
  SECTION:=firmware
  CATEGORY:=Firmware
  DEPENDS:=@(TARGET_ipq40xx||TARGET_ipq806x||TARGET_ipq807x)
  TITLE:=Custom Board
endef

define ipq-wifi-install-one-to
  $(INSTALL_DIR)  $(2)/lib/firmware/ath10k/$(3)/
  $(INSTALL_DATA) $(1) $(2)/lib/firmware/ath10k/$(3)/board-2.bin
endef

define ipq-wifi-install-ath11-one-to
  $(INSTALL_DIR)  $(2)/lib/firmware/ath11k/$(3)/
  $(INSTALL_DATA) $(1) $(2)/lib/firmware/ath11k/$(3)/board-2.bin
endef

define ipq-wifi-install-one
  $(if $(filter $(suffix $(1)),.QCA4019 .qca4019),\
    $(call ipq-wifi-install-one-to,$(1),$(2),QCA4019/hw1.0),\
  $(if $(filter $(suffix $(1)),.QCA9888 .qca9888),\
    $(call ipq-wifi-install-one-to,$(1),$(2),QCA9888/hw2.0),\
  $(if $(filter $(suffix $(1)),.QCA9984 .qca9984),\
    $(call ipq-wifi-install-one-to,$(1),$(2),QCA9984/hw1.0),\
  $(if $(filter $(suffix $(1)),.QCA9889 .qca9889),\
    $(call ipq-wifi-install-one-to,$(1),$(2),QCA9887/hw1.0),\
  $(if $(filter $(suffix $(1)),.IPQ8074 .ipq8074),\
    $(call ipq-wifi-install-ath11-one-to,$(1),$(2),IPQ8074/hw2.0),\
    $(error Unrecognized board-file suffix '$(suffix $(1))' for '$(1)')\
  )))))

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
$(eval $(call generate-ipq-wifi-package,devolo_magic-2-wifi-next,devolo Magic 2 WiFi next))
$(eval $(call generate-ipq-wifi-package,dlink_dap2610,D-Link DAP-2610))
$(eval $(call generate-ipq-wifi-package,edgecore_ecw5410,Edgecore ECW5410))
$(eval $(call generate-ipq-wifi-package,edgecore_oap100,Edgecore OAP100))
$(eval $(call generate-ipq-wifi-package,engenius_eap2200,EnGenius EAP2200))
$(eval $(call generate-ipq-wifi-package,engenius_emd1,EnGenius EMD1))
$(eval $(call generate-ipq-wifi-package,engenius_emr3500,EnGenius EMR3500))
$(eval $(call generate-ipq-wifi-package,ezviz_cs-w3-wd1200g-eup,EZVIZ CS-W3-WD1200G EUP))
$(eval $(call generate-ipq-wifi-package,glinet_gl-ap1300,GL.iNet GL-AP1300))
$(eval $(call generate-ipq-wifi-package,glinet_gl-s1300,GL.iNet GL-S1300))
$(eval $(call generate-ipq-wifi-package,hiwifi_c526a,HiWiFi C526A))
$(eval $(call generate-ipq-wifi-package,linksys_ea8300,Linksys EA8300))
$(eval $(call generate-ipq-wifi-package,linksys_mr8300-v0,Linksys MR8300))
$(eval $(call generate-ipq-wifi-package,luma_wrtq-329acn,Luma WRTQ-329ACN))
$(eval $(call generate-ipq-wifi-package,mikrotik_hap-ac2,Mikrotik hAP ac2))
$(eval $(call generate-ipq-wifi-package,mikrotik_sxtsq-5-ac,MikroTik SXTsq 5 ac))
$(eval $(call generate-ipq-wifi-package,mobipromo_cm520-79f,MobiPromo CM520-79F))
$(eval $(call generate-ipq-wifi-package,nec_wg2600hp3,NEC Platforms WG2600HP3))
$(eval $(call generate-ipq-wifi-package,netgear_sxr80,Netgear SXR80))
$(eval $(call generate-ipq-wifi-package,netgear_wac510,Netgear WAC510))
$(eval $(call generate-ipq-wifi-package,plasmacloud_pa1200,Plasma Cloud PA1200))
$(eval $(call generate-ipq-wifi-package,plasmacloud_pa2200,Plasma Cloud PA2200))
$(eval $(call generate-ipq-wifi-package,p2w_r619ac,P&W R619AC))
$(eval $(call generate-ipq-wifi-package,qxwlan_e2600ac,Qxwlan E2600AC))
$(eval $(call generate-ipq-wifi-package,redmi_ax6,Redmi AX6))
$(eval $(call generate-ipq-wifi-package,xiaomi_ax3600,Xiaomi AX3600))
$(eval $(call generate-ipq-wifi-package,xiaomi_ax9000,Xiaomi AX9000))
$(eval $(call generate-ipq-wifi-package,zte_mf263,ZTE MF263))
$(eval $(call generate-ipq-wifi-package,zte_mf269,ZTE MF269))

$(foreach PACKAGE,$(ALLWIFIPACKAGES),$(eval $(call BuildPackage,$(PACKAGE))))
