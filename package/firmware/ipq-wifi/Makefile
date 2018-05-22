include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/version.mk

PKG_NAME:=ipq-wifi
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile
endef

ALLWIFIBOARDS:=8dev_jalapeno asus_rt-ac58u avm_fritzbox-4040 glinet_gl-b1300 meraki_mr33 netgear_ex61x0v2
ALLWIFIPACKAGES:=$(foreach BOARD,$(ALLWIFIBOARDS),ipq-wifi-$(BOARD))

define Package/ipq-wifi-default
  SUBMENU:=ath10k IPQ4019 Boarddata
  SECTION:=firmware
  CATEGORY:=Firmware
  DEPENDS:=@TARGET_ipq40xx
  TITLE:=Custom Board
endef

define generate-ipq-wifi-package
  define Package/ipq-wifi-$(1)
    $(call Package/ipq-wifi-default)
    TITLE:=Board for $(3)
    CONFLICTS:=$(PREV_BOARD)
  endef

  define Package/ipq-wifi-$(1)/description
This device custom package board-2.bin overwrites the board-2.bin
file which is supplied by the ath10k-firmware-qca4019 package.

This is package is only necessary for the $(3).
Don't install it for any other device!
  endef

  define Package/ipq-wifi-$(1)/install-overlay
	$(INSTALL_DIR) $$(1)/lib/firmware/ath10k/QCA4019/hw1.0
	$(INSTALL_DATA) ./$(2) $$(1)/lib/firmware/ath10k/QCA4019/hw1.0/board-2.bin
  endef

  PREV_BOARD+=ipq-wifi-$(1)
endef

$(eval $(call generate-ipq-wifi-package,8dev_jalapeno,board-8dev_jalapeno.bin,8devices Jalapeno))
$(eval $(call generate-ipq-wifi-package,asus_rt-ac58u,board-asus_rt-ac58u.bin,ASUS RT-AC58U))
$(eval $(call generate-ipq-wifi-package,avm_fritzbox-4040,board-avm_fritzbox-4040.bin,AVM FRITZ!Box 4040))
$(eval $(call generate-ipq-wifi-package,glinet_gl-b1300,board-glinet_gl-b1300.bin,GL.iNet GL-B1300))
$(eval $(call generate-ipq-wifi-package,meraki_mr33,board-meraki_mr33.bin,Cisco Meraki MR33))
$(eval $(call generate-ipq-wifi-package,netgear_ex61x0v2,board-netgear_ex61x0v2.bin,Netgear EX61x0v2))

$(foreach PACKAGE,$(ALLWIFIPACKAGES),$(eval $(call BuildPackage,$(PACKAGE))))
