#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Package/php7-pecl/Default
  SUBMENU:=PHP7
  SECTION:=lang
  CATEGORY:=Languages
  URL:=http://pecl.php.net/
  DEPENDS:=php7
endef

define Build/Prepare
	$(Build/Prepare/Default)
	$(if $(QUILT),,( cd $(PKG_BUILD_DIR); $(STAGING_DIR)/usr/bin/phpize7 ))
endef

define Build/Configure
	$(if $(QUILT),( cd $(PKG_BUILD_DIR); $(STAGING_DIR)/usr/bin/phpize7 ))
	$(Build/Configure/Default)
endef

CONFIGURE_VARS+= \
	ac_cv_c_bigendian_php=$(if $(CONFIG_BIG_ENDIAN),yes,no)

CONFIGURE_ARGS+= \
	--with-php-config=$(STAGING_DIR)/usr/bin/php7-config

define PHP7PECLPackage

  define Package/php7-pecl-$(1)
    $(call Package/php7-pecl/Default)
    TITLE:=$(2)

    ifneq ($(3),)
      DEPENDS+=$(3)
    endif
  endef

  define Package/php7-pecl-$(1)/install
	$(INSTALL_DIR) $$(1)/usr/lib/php
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/modules/$(subst -,_,$(1)).so $$(1)/usr/lib/php/
	$(INSTALL_DIR) $$(1)/etc/php7
    ifeq ($(5),zend)
	echo "zend_extension=/usr/lib/php/$(subst -,_,$(1)).so" > $$(1)/etc/php7/$(if $(4),$(4),20)_$(subst -,_,$(1)).ini
    else
	echo "extension=$(subst -,_,$(1)).so" > $$(1)/etc/php7/$(if $(4),$(4),20)_$(subst -,_,$(1)).ini
    endif
  endef

endef
