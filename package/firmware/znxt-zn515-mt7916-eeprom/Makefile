include $(TOPDIR)/rules.mk

PKG_NAME:=znxt-zn515-mt7916-eeprom
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/znxt-zn515-mt7916-eeprom
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=ZNXT ZN515XG-D MT7916D EEPROM template
endef

define Package/znxt-zn515-mt7916-eeprom/description
  MT7916D EEPROM template for ZN515XG-D. mt7915e merges per-device iCal
  fields from the chip eFuse.
endef

define Build/Compile
endef

define Package/znxt-zn515-mt7916-eeprom/install
	$(INSTALL_DIR) $(1)/lib/firmware/mediatek
	$(INSTALL_DATA) ./files/mt7916_eeprom.bin \
		$(1)/lib/firmware/mediatek/mt7916_eeprom.bin
endef

$(eval $(call BuildPackage,znxt-zn515-mt7916-eeprom))
