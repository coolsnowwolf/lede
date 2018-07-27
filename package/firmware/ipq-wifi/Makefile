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

#ALLWIFIBOARDS:=<devicename>
ALLWIFIBOARDS:=rt-acrh17
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

#$(eval $(call generate-ipq-wifi-package,<devicename>,<filename>,<displayname>))
$(eval $(call generate-ipq-wifi-package,rt-acrh17,board-rt-acrh17.bin,ASUS RT-ACRH17/RT-AC82U/RT-AC42U))

$(foreach PACKAGE,$(ALLWIFIPACKAGES),$(eval $(call BuildPackage,$(PACKAGE))))
