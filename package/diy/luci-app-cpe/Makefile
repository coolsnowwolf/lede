#将openwrt顶层目录下的rules.mk文件中的内容导入进来
include $(TOPDIR)/rules.mk
#软件包名
PKG_NAME:=luci-app-cpe
#软件包版本
PKG_VERSION:=5.0.1
#真正编译当前软件包的目录
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)
 
 
 #将$(TOPDIR)/include目录下的package.mk文件中的内容导入进来
include $(INCLUDE_DIR)/package.mk
 
define Package/luci-app-cpe
	PKGARCH:=all
	SECTION:=wrtnode
	CATEGORY:=Daocaoren
	SUBMENU :=CPE
	TITLE:=luci-app-cpe
	DEPENDS:=+sendat + luci-compat +kmod-usb-net  +kmod-usb-net-cdc-ether +kmod-usb-acm \
		+kmod-usb-net-qmi-wwan  +kmod-usb-net-rndis +kmod-usb-serial-qualcomm \
		+kmod-usb-net-sierrawireless +kmod-usb-ohci +kmod-usb-serial \
		+kmod-usb-serial-option +kmod-usb-wdm \
		+kmod-usb2 +kmod-usb3 \
		+quectel-CM-5G +kmod-qmi_wwan_q +kmod-usb-net-cdc-mbim
endef
 
PKG_LICENSE:=GPLv3
PKG_LINCESE_FILES:=LICENSE
PKF_MAINTAINER:=daocaoren <168620188@qq.com>

include $(TOPDIR)/feeds/luci/luci.mk
# call BuildPackage - OpenWrt buildroot signature

