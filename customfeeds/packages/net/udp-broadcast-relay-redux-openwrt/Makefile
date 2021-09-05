include $(TOPDIR)/rules.mk

PKG_NAME:=udp-broadcast-relay-redux
PKG_RELEASE:=$(AUTORELEASE)
PKG_LICENSE:=GPL-2.0

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/udp-redux/udp-broadcast-relay-redux
PKG_SOURCE_DATE:=2021-04-05
PKG_SOURCE_VERSION:=671372938b55a186625a80516f86e8b9948c977a
PKG_MIRROR_HASH:=11cf8728f2b8e966f4f57032d817a889f680ed8e61afff35b52ca9c6789a03c6

include $(INCLUDE_DIR)/package.mk

define Package/udp-broadcast-relay-redux
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Routing and Redirection
  TITLE:=listens for packets on a specified UDP broadcast port and replays them
  URL:=https://github.com/udp-redux/udp-broadcast-relay-redux
endef

define Package/udp-broadcast-relay-redux/description
	This program listens for packets on a specified UDP broadcast port.
	When a packet is received, it sends that packet to all specified interfaces but
	the one it came from as though it originated from the original sender.
	The primary purpose of this is to allow games on machines on separated
	local networks (Ethernet, WLAN) that use udp broadcasts to find each other to do so.
	It also works on ppp links, so you can log in from windows boxes (e.g. using pptp)
	and play LAN-based games together. Currently, you have to care about upcoming or
	downgoing interfaces yourself.
endef

define Package/udp-broadcast-relay-redux/conffiles
/etc/config/udp_broadcast_relay_redux
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(PKG_BUILD_DIR)/main.c -o $(PKG_BUILD_DIR)/$(PKG_NAME)
endef

define Package/udp-broadcast-relay-redux/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/config $(1)/etc/init.d
	$(CP) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/sbin/
	$(INSTALL_CONF) ./files/udp_broadcast_relay_redux.config $(1)/etc/config/udp_broadcast_relay_redux
	$(INSTALL_BIN) ./files/udp-broadcast-relay-redux.init $(1)/etc/init.d/udp-broadcast-relay-redux
endef

$(eval $(call BuildPackage,udp-broadcast-relay-redux))
