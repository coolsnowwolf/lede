include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/version.mk

PKG_NAME:=ath11k-wifi
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
  edgecore-eap102 \
	gl-ax1800 \
	gl-axt1800 \
	qihoo_v6

ALLWIFIPACKAGES:=$(foreach BOARD,$(ALLWIFIBOARDS),ath11k-wifi-$(BOARD))

define Package/ath11k-wifi-default
  SUBMENU:=ath11k Board-Specific Overrides
  SECTION:=firmware
  CATEGORY:=Firmware
  DEPENDS:=@TARGET_ipq60xx
  TITLE:=Custom Board
endef

define ath11k-wifi-install-one-to
  $(INSTALL_DIR)  $(2)/lib/firmware/ath11k/$(3)/
  $(INSTALL_DATA) $(1) $(2)/lib/firmware/ath11k/$(3)/board-2.bin
endef

define ath11k-wifi-install-one
  $(if $(filter $(suffix $(1)),.IPQ6018 .ipq6018),\
    $(call ath11k-wifi-install-one-to,$(1),$(2),IPQ6018/hw1.0),\
    $(error Unrecognized board-file suffix '$(suffix $(1))' for '$(1)')\
  )

endef
# Blank line required at end of above define due to foreach context

define generate-ath11k-wifi-package
  define Package/ath11k-wifi-$(1)
    $(call Package/ath11k-wifi-default)
    TITLE:=board-2.bin Overrides for $(2)
    CONFLICTS:=$(PREV_BOARD)
  endef

  define Package/ath11k-wifi-$(1)/description
The $(2) requires board-specific, reference ("cal") data
that is not yet present in the upstream wireless firmware distribution.

This package supplies board-2.bin file(s) that, in the interim,
overwrite those supplied by the ath10k-firmware-* packages.

This is package is only necessary for the $(2).

Do not install it for any other device!
  endef

  define Package/ath11k-wifi-$(1)/install-overlay
    $$$$(foreach IPQ_WIFI_BOARD_FILE,$$$$(wildcard board-$(1).*),\
      $$$$(call ath11k-wifi-install-one,$$$$(IPQ_WIFI_BOARD_FILE),$$(1)))
  endef

  PREV_BOARD+=ath11k-wifi-$(1)
endef

# Add board name to ALLWIFIBOARDS
# Place files in this directory as board-<devicename>.<qca4019|qca9888|qca9984>
# Add $(eval $(call generate-ath11k-wifi-package,<devicename>,<display name>))

$(eval $(call generate-ath11k-wifi-package,edgecore-eap102,Edgecore EAP102))
$(eval $(call generate-ath11k-wifi-package,gl-ax1800,Gl.iNET AX1800))
$(eval $(call generate-ath11k-wifi-package,gl-axt1800,Gl.iNET AXT1800))
$(eval $(call generate-ath11k-wifi-package,qihoo_v6,QIHOO 360V6))


$(foreach PACKAGE,$(ALLWIFIPACKAGES),$(eval $(call BuildPackage,$(PACKAGE))))
