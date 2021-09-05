#
# Copyright (C) 2009-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=znc
PKG_VERSION:=1.8.2
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://znc.in/releases \
		https://znc.in/releases/archive
PKG_HASH:=ff238aae3f2ae0e44e683c4aee17dc8e4fdd261ca9379d83b48a7d422488de0d

PKG_MAINTAINER:=Jonas Gorski <jonas.gorski@gmail.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:znc:znc

PKG_USE_MIPS16:=0

PKG_CONFIG_DEPENDS:= CONFIG_ZNC_ICU

define Package/znc/default
  SUBMENU:=Instant Messaging
  SECTION:=net
  CATEGORY:=Network
  TITLE:=ZNC
  URL:=https://znc.in/
  USERID:=znc:znc
endef

define Package/znc
  $(Package/znc/default)
  DEPENDS:=+libopenssl +libstdcpp +ZNC_ICU:icu +zlib
  MENU:=1
endef

define Package/znc/description
	ZNC is an IRC bouncer with many advanced features like detaching,
	multiple users, per channel playback buffer, SSL, IPv6, transparent DCC
	bouncing, and c++ module support to name a few.
endef

define Package/znc/config
	source "$(SOURCE)/Config.in"
endef

define Package/znc/conffiles
/etc/config/znc
endef


define Package/znc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/znc $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/znc.init $(1)/etc/init.d/znc
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/znc.conf $(1)/etc/config/znc
	$(INSTALL_DIR) $(1)/usr/lib/znc/
endef


ZNC_MODULES :=
ZNC_MODULE_TARGETS :=

define module
  define Package/znc-mod-$(strip $(1))
    $(Package/znc/default)
    TITLE+= ($(strip $(1)) plugin)
    DEPENDS:=znc
  endef

  define Package/znc-mod-$(strip $(1))/description
   $(strip $(2))
  endef

  define Package/znc-mod-$(strip $(1))/install
	$(INSTALL_DIR) $$(1)/usr/lib/znc/
	$(INSTALL_BIN) $$(PKG_INSTALL_DIR)/usr/lib/znc/$(subst -,_,$(strip $(1))).so $$(1)/usr/lib/znc/
	# include webadmin page templates if existing
	if [ -d $$(PKG_INSTALL_DIR)/usr/share/znc/modules/$(subst -,_,$(strip $(1))) ]; then \
		$(INSTALL_DIR) $$(1)/usr/share/znc/modules ;\
		$(CP) $$(PKG_INSTALL_DIR)/usr/share/znc/modules/$(subst -,_,$(strip $(1))) $$(1)/usr/share/znc/modules ;\
	fi
  endef

  ZNC_MODULES += znc-mod-$(strip $(1))
  $(if $(CONFIG_PACKAGE_znc-mod-$(strip $(1))),ZNC_MODULE_TARGETS += $(subst -,_,$(strip $(1))).so)
endef

define webadmin
  define Package/znc-mod-webadmin
    $(Package/znc/default)
    TITLE+= (webadmin plugin)
    DEPENDS:=znc
  endef

  define Package/znc-mod-webadmin/description
   Allows you to add/remove/edit users and settings on the fly via a web browser.
  endef

  define Package/znc-mod-webadmin/install
	$(INSTALL_DIR) $$(1)/usr/lib/znc/
	$(INSTALL_BIN) $$(PKG_INSTALL_DIR)/usr/lib/znc/webadmin.so $$(1)/usr/lib/znc/
	$(INSTALL_DIR) $$(1)/usr/share/znc/modules
	$(CP) $$(PKG_INSTALL_DIR)/usr/share/znc/modules/webadmin $$(1)/usr/share/znc/modules
	$(INSTALL_DIR) $$(1)/usr/share/znc/webskins/
	$(CP) $$(PKG_INSTALL_DIR)/usr/share/znc/webskins/_default_ $$(1)/usr/share/znc/webskins/
  endef

  ZNC_MODULES += znc-mod-webadmin
  $(if $(CONFIG_PACKAGE_znc-mod-webadmin),ZNC_MODULE_TARGETS += webadmin.so)
endef

define webskin
  define Package/znc-webskin-$(strip $(1))
    $(Package/znc/default)
    TITLE+= ($(strip $(1)) webskin)
    DEPENDS:=znc-mod-webadmin
  endef

  define Package/znc-webskin-$(strip $(1))/description
    $(strip $(1)) webskin for webadmin
  endef

  define Package/znc-webskin-$(strip $(1))/install
	$(INSTALL_DIR) $$(1)/usr/share/znc/webskins/
	$(CP) $$(PKG_INSTALL_DIR)/usr/share/znc/webskins/$(strip $(1)) $$(1)/usr/share/znc/webskins/
  endef
  ZNC_MODULES += znc-webskin-$(strip $(1))
endef

, := ,

$(eval $(call module,adminlog,Log user connects and disconnects and failed \
	logins to file or syslog.))

$(eval $(call module,alias,Provides bouncer-side command alias support.))

$(eval $(call module,autoattach,Reattaches you to channels on activity.))

$(eval $(call module,autocycle,Cycles a channel when you are the only one in \
	there and you don't have op.))

$(eval $(call module,autoop,Auto op the good guys.))

$(eval $(call module,autoreply,Gives a automatic reply if someone messages you \
	if you are away.))

$(eval $(call module,autovoice,Autovoices everyone who joins some channel.))

$(eval $(call module,awaynick,Change your nick while you are away.))

$(eval $(call module,awaystore,Stores messages while away$(,) also auto away.))

$(eval $(call module,block-motd,This module blocks the server's Message of the \
	Day.))

$(eval $(call module,blockuser,Blocks certain users from using ZNC saying \
	their account was disabled.))

$(eval $(call module,bouncedcc,Bounces dcc transfers through the znc server \
	instead of sending them directly to the user.))

$(eval $(call module,buffextras,Add nick changes$(,) joins$(,) parts$(,) topic \
	changes etc. to your playback buffer.))

$(eval $(call module,cert,Use a SSL certificate for connecting to a server.))

$(eval $(call module,certauth,This module allows users to log in to ZNC via \
	SSL client keys.))

$(eval $(call module,chansaver,Keeping config up to date when user joins and \
	parts.))

$(eval $(call module,clearbufferonmsg,This module keeps the buffer until the \
	next message from the client.))

$(eval $(call module,clientnotify,Notify about new incoming connections to \
	your user.))

$(eval $(call module,controlpanel,Allows you to add/remove/edit users and \
	settings on the fly via IRC messages.))

$(eval $(call module,crypt,Encryption for channel/private messages.))

$(eval $(call module,ctcpflood,This module tries to block ctcp floods.))

$(eval $(call module,dcc,Allows you to transfer files to and from ZNC.))

$(eval $(call module,disconkick,This module will kick your client from all \
	channels where you are$(,) in case if ZNC disconnects from server.))

$(eval $(call module,fail2ban,Block IPs for some time after a failed login.))

$(eval $(call module,flooddetach,This module detaches you from channels which \
	are flooded.))

$(eval $(call module,identfile,Places the ident of a user to a file when they \
	 are trying to connect.))

$(eval $(call module,imapauth,Allow users to authenticate via IMAP.))

$(eval $(call module,keepnick,Tries to get you your primary nick.))

$(eval $(call module,kickrejoin,Implements auto-rejoin-on-kick.))

$(eval $(call module,lastseen,Logs when a user last logged in to ZNC.))

$(eval $(call module,listsockets,This module displays a list of all open \
	sockets in ZNC.))

$(eval $(call module,log,Log conversations to file.))

$(eval $(call module,missingmotd,Sends 422 to clients when they login.))

$(eval $(call module,modules_online,This module fakes the online status of \
	ZNC-*users.))

$(eval $(call module,nickserv,Auths you with NickServ.))

$(eval $(call module,notes,This modules stores and displays short notes using \
	a key/note pairs and shows them to you on connect.))

$(eval $(call module,notify-connect,Sends a notice to all admins when a user \
	logs in or out.))

$(eval $(call module,perform,Performs commands on connect.))

$(eval $(call module,playback,Avoid repetitive playback buffers on re-connect \
	with supported clients (e.g. mutter, colluquy)))

$(eval $(call module,raw,View all of the raw traffic.))

$(eval $(call module,route-replies,Routes back answers to the right client \
	when connected with multiple clients.))

$(eval $(call module,sasl,The SASL module allows you to authenticate to an \
	IRC network via SASL.))

$(eval $(call module,savebuff,Saves your channel buffers into an encrypted \
	file so they can survive restarts and reboots.))

$(eval $(call module,schat,SSL (encrypted) DCC chats.))

$(eval $(call module,send-raw,Allows you to send raw traffic to IRC from \
	other users.))

$(eval $(call module,simple-away,This module will automatically set you away \
	on IRC while you are disconnected from the bouncer.))

$(eval $(call module,shell,Have your unix shell in a query window right inside \
	of your IRC client.))

$(eval $(call module,stickychan,Keeps you sticked to specific channels.))

$(eval $(call module,watch,Monitor activity for specific text patterns from \
	specific users and have the text sent to a special query window.))

$(eval $(call webadmin))

$(eval $(call webskin,dark-clouds))
$(eval $(call webskin,forest))
$(eval $(call webskin,ice))

PKG_CONFIG_DEPENDS += $(patsubst %,CONFIG_PACKAGE_%,$(ZNC_MODULES))

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS += \
	-DWANT_ICU=O$(if $(CONFIG_ZNC_ICU),N,FF) \
	-DWANT_I18N=OFF \
	-DWANT_CYRUS=OFF \
	-DWANT_PERL=OFF \
	-DWANT_SYSTEMD=OFF \
	-DWANT_PYTHON=OFF \
	-DWANT_OPENSSL=ON \
	-DWANT_SWIG=OFF \
	-DWANT_TCL=OFF \
	-DWANT_ZLIB=ON

$(eval $(call BuildPackage,znc))
$(foreach m,$(ZNC_MODULES),$(eval $(call BuildPackage,$(m))))
