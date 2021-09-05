#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openssh
PKG_VERSION:=8.6p1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://ftp.openbsd.org/pub/OpenBSD/OpenSSH/portable/ \
		https://ftp.spline.de/pub/OpenBSD/OpenSSH/portable/
PKG_HASH:=c3e6e4da1621762c850d03b47eed1e48dff4cc9608ddeb547202a234df8ed7ae

PKG_LICENSE:=BSD ISC
PKG_LICENSE_FILES:=LICENCE
PKG_CPE_ID:=cpe:/a:openssh:openssh

PKG_REMOVE_FILES:=
PKG_CONFIG_DEPENDS := \
	CONFIG_OPENSSH_LIBFIDO2

PKG_BUILD_DEPENDS += OPENSSH_LIBFIDO2:libfido2

include $(INCLUDE_DIR)/package.mk

define Package/openssh/Default
	SECTION:=net
	CATEGORY:=Network
	DEPENDS:=+libopenssl +zlib
	TITLE:=OpenSSH
	MAINTAINER:=Peter Wagner <tripolar@gmx.at>
	URL:=http://www.openssh.com/
	SUBMENU:=SSH
	VARIANT:=without-pam
endef

define Package/openssh-moduli
	$(call Package/openssh/Default)
	DEPENDS+= +openssh-keygen
	TITLE+= moduli file
endef

define Package/openssh-moduli/description
OpenSSH server moduli file.
endef

define Package/openssh-client
	$(call Package/openssh/Default)
	TITLE+= client
	ALTERNATIVES:=\
		200:/usr/bin/ssh:/usr/libexec/ssh-openssh \
		200:/usr/bin/scp:/usr/libexec/scp-openssh
endef

define Package/openssh-client/description
OpenSSH client.
endef

define Package/openssh-client/conffiles
/etc/ssh/ssh_config
endef

define Package/openssh-client-utils
	$(call Package/openssh/Default)
	DEPENDS+= +openssh-client +openssh-keygen
	TITLE+= client utilities
endef

define Package/openssh-client-utils/description
OpenSSH client utilities.
endef

define Package/openssh-keygen
	$(call Package/openssh/Default)
	TITLE+= keygen
endef

define Package/openssh-keygen/description
OpenSSH keygen.
endef

define Package/openssh-server
	$(call Package/openssh/Default)
	DEPENDS+= +openssh-keygen +OPENSSH_LIBFIDO2:libfido2
	TITLE+= server
	USERID:=sshd=22:sshd=22
endef

define Package/openssh-server/config
	source "$(SOURCE)/Config.in"
endef

define Package/openssh-server/description
OpenSSH server.
endef

define Package/openssh-server/conffiles
/etc/ssh/sshd_config
/etc/ssh/ssh_host_ed25519_key
/etc/ssh/ssh_host_ed25519_key.pub
/etc/ssh/ssh_host_rsa_key
/etc/ssh/ssh_host_rsa_key.pub
endef

define Package/openssh-server-pam
	$(call Package/openssh/Default)
	DEPENDS+= +libpthread +openssh-keygen +libpam
	TITLE+= server (with PAM support)
	VARIANT:=with-pam
	USERID:=sshd=22:sshd=22
endef

define Package/openssh-server-pam/description
OpenSSH server (with PAM support).
endef

define Package/openssh-server-pam/conffiles
/etc/pam.d/sshd
/etc/security/access-sshd-local.conf
/etc/ssh/sshd_config
endef

define Package/openssh-sftp-client
	$(call Package/openssh/Default)
	TITLE+= SFTP client
endef

define Package/openssh-sftp-client/description
OpenSSH SFTP client.
endef

define Package/openssh-sftp-server
	$(call Package/openssh/Default)
	TITLE+= SFTP server
endef

define Package/openssh-sftp-server/description
OpenSSH SFTP server.
endef

define Package/openssh-sftp-avahi-service
	$(call Package/openssh/Default)
	TITLE+= (SFTP Avahi service)
	DEPENDS:=+openssh-sftp-server +avahi-daemon
endef

define Package/openssh-sftp-avahi-service/description
 This package contains the service definition for announcing
 SFTP support via mDNS/DNS-SD.
endef

define Package/openssh-sftp-avahi-service/conffiles
/etc/avahi/services/sftp-ssh.service
endef

CONFIGURE_ARGS += \
	--sysconfdir=/etc/ssh \
	--with-privsep-user=sshd \
	--with-privsep-path=/var/empty \
	--disable-strip \
	--disable-etc-default-login \
	--disable-lastlog \
	--disable-utmp \
	--disable-utmpx \
	--disable-wtmp \
	--disable-wtmpx \
	--without-bsd-auth \
	--without-kerberos5 \
	--with-stackprotect \
	--with$(if $(CONFIG_OPENSSL_ENGINE),,out)-ssl-engine \
	--with$(if $(CONFIG_OPENSSH_LIBFIDO2),,out)-security-key-builtin
	
ifeq ($(BUILD_VARIANT),with-pam)
CONFIGURE_ARGS += \
	--with-pam
else
CONFIGURE_ARGS += \
	--without-pam
endif

CONFIGURE_VARS += LD="$(TARGET_CC)"

ifeq ($(BUILD_VARIANT),with-pam)
TARGET_LDFLAGS += -lpthread
endif

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		STRIP_OPT="" \
		all install
endef

define Package/openssh-moduli/install
	$(INSTALL_DIR) $(1)/etc/ssh
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/etc/ssh/moduli $(1)/etc/ssh/
endef

define Package/openssh-client/install
	$(INSTALL_DIR) $(1)/etc/ssh
	chmod 0700 $(1)/etc/ssh
	$(CP) $(PKG_INSTALL_DIR)/etc/ssh/ssh_config $(1)/etc/ssh/
	$(INSTALL_DIR) $(1)/usr/libexec
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ssh $(1)/usr/libexec/ssh-openssh
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/scp $(1)/usr/libexec/scp-openssh
endef

define Package/openssh-client-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(foreach bin,add agent keyscan keysign,$(PKG_BUILD_DIR)/ssh-$(bin)) $(1)/usr/bin/
endef

define Package/openssh-keygen/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ssh-keygen $(1)/usr/bin/
endef

define Package/openssh-server/install
	$(INSTALL_DIR) $(1)/etc/ssh
	chmod 0700 $(1)/etc/ssh
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/etc/ssh/sshd_config $(1)/etc/ssh/
	sed -r -i 's,^#(HostKey /etc/ssh/ssh_host_(rsa|ed25519)_key)$$$$,\1,' $(1)/etc/ssh/sshd_config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/sshd.init $(1)/etc/init.d/sshd
	$(INSTALL_DIR) $(1)/lib/preinit
	$(INSTALL_BIN) ./files/sshd.failsafe $(1)/lib/preinit/99_10_failsafe_sshd
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/sshd $(1)/usr/sbin/
endef

define Package/openssh-server-pam/install
	$(call Package/openssh-server/install,$(1))
	sed -i 's,#PasswordAuthentication yes,PasswordAuthentication no,g' $(1)/etc/ssh/sshd_config
	sed -i 's,#UsePAM no,UsePAM yes,g' $(1)/etc/ssh/sshd_config
	$(INSTALL_DIR) $(1)/etc/pam.d
	$(INSTALL_DATA) ./files/sshd.pam $(1)/etc/pam.d/sshd
	$(INSTALL_DIR) $(1)/etc/security
	$(INSTALL_DATA) ./files/sshd.pam-access $(1)/etc/security/access-sshd-local.conf
endef

define Package/openssh-sftp-client/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/sftp $(1)/usr/bin/
endef

define Package/openssh-sftp-server/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/sftp-server $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/libexec
	ln -sf ../lib/sftp-server $(1)/usr/libexec/sftp-server
endef

define Package/openssh-sftp-avahi-service/install
	$(INSTALL_DIR) $(1)/etc/avahi/services
	$(INSTALL_DATA) ./files/sftp-ssh.service $(1)/etc/avahi/services/
endef

$(eval $(call BuildPackage,openssh-client))
$(eval $(call BuildPackage,openssh-moduli))
$(eval $(call BuildPackage,openssh-client-utils))
$(eval $(call BuildPackage,openssh-keygen))
$(eval $(call BuildPackage,openssh-server))
$(eval $(call BuildPackage,openssh-server-pam))
$(eval $(call BuildPackage,openssh-sftp-client))
$(eval $(call BuildPackage,openssh-sftp-server))
$(eval $(call BuildPackage,openssh-sftp-avahi-service))
