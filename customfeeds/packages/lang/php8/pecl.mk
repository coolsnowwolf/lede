#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Package/php8-pecl/Default
  SUBMENU:=PHP8
  SECTION:=lang
  CATEGORY:=Languages
  URL:=http://pecl.php.net/
  DEPENDS:=php8
endef

define Build/Prepare
	$(Build/Prepare/Default)
	$(if $(QUILT),,( cd $(PKG_BUILD_DIR); $(STAGING_DIR)/usr/bin/phpize8 ))
endef

define Build/Configure
	$(if $(QUILT),( cd $(PKG_BUILD_DIR); $(STAGING_DIR)/usr/bin/phpize8 ))
	$(Build/Configure/Default)
endef

CONFIGURE_VARS+= \
	ac_cv_c_bigendian_php=$(if $(CONFIG_BIG_ENDIAN),yes,no)

CONFIGURE_ARGS+= \
	--with-php-config=$(STAGING_DIR)/usr/bin/php8-config

define PHP8PECLPackage

  define Package/php8-pecl-$(1)
    $(call Package/php8-pecl/Default)
    TITLE:=$(2)

    ifneq ($(3),)
      DEPENDS+=$(3)
    endif

    VARIANT:=php8
  endef

  define Package/php8-pecl-$(1)/install
	$(INSTALL_DIR) $$(1)/usr/lib/php8
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/modules/$(subst -,_,$(1)).so $$(1)/usr/lib/php8/
	$(INSTALL_DIR) $$(1)/etc/php8
    ifeq ($(5),zend)
	echo "zend_extension=/usr/lib/php8/$(subst -,_,$(1)).so" > $$(1)/etc/php8/$(if $(4),$(4),20)_$(subst -,_,$(1)).ini
    else
	echo "extension=$(subst -,_,$(1)).so" > $$(1)/etc/php8/$(if $(4),$(4),20)_$(subst -,_,$(1)).ini
    endif
  endef

endef
