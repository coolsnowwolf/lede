#
# Copyright (C) 2015-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libupm
PKG_VERSION:=2.0.0
PKG_RELEASE:=6

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/intel-iot-devkit/upm/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=7dd2f4165b71e071d100b58d6a392f3cf57b0f257c82ffabf49e931b5ed6bc23
PKG_BUILD_DIR:=$(BUILD_DIR)/upm-$(PKG_VERSION)

PKG_MAINTAINER:=John Crispin <blogic@openwrt.org>, Hirokazu MORIKAWA <morikw2@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

CMAKE_INSTALL:=1
CMAKE_BINARY_SUBDIR:=build
PKG_USE_MIPS16:=0
PYTHON3_PKG_BUILD:=0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk
include ../../lang/python/python3-package.mk

UPM_MODULES:= \
	a110x abp ad8232 adafruitms1438 adafruitss adc121c021 adis16448 ads1x15 adxl335 adxl345 \
	adxrs610 am2315 apa102 apds9002 apds9930 at42qt1070 bh1749 bh1750 bh1792 biss0001 bma220 \
	bma250e bmg160 bmi160 bmm150 bmp280 bmpx8x bmx055 bno055 button buzzer cjq4435 collision \
	cwlsxxa dfrec dfrorp dfrph ds1307 ds1808lc ds18b20 ds2413 ecezo ecs1030 \
	ehr eldriver electromagnet emg enc03r flex gas gp2y0a gprs \
	gsr guvas12d h3lis331dl hcsr04 hdc1000 hdxxvxta hka5 hlg150h hm11 hmc5883l hmtrp hp20x \
	ht9170 htu21d hx711 ili9341 ims ina132 interfaces isd1820 itg3200 jhd1313m1 joystick12 kx122 \
	kxcjk1013 kxtj3 l298 l3gd20 lcd lcdks lcm1602 ldt0028 led lidarlitev3 light linefinder lis2ds12 \
	lis3dh lm35 lol loudness lp8860 lpd8806 lsm303agr lsm303d lsm303dlh lsm6ds3h lsm6dsl lsm9ds0 \
	m24lr64e mag3110 max30100 max31723 max31855 max44000 max44009 max5487 maxds3231m maxsonarez \
	mb704x mcp2515 mcp9808 md mg811 mhz16 mic micsv89 mlx90614 mma7361 mma7455 mma7660 mma8x5x \
	mmc35240 moisture mpl3115a2 mpr121 mpu9150 mq303a ms5611 ms5803 my9221 nlgpio16 nmea_gps \
	nrf24l01 nrf8001 nunchuck o2 otp538u p9813 pca9685 pn532 ppd42ns pulsensor relay rf22 \
	rfr359f rgbringcoder rhusb rn2903 rotary rotaryencoder rpr220 rsc scam sensortemplate \
	servo sht1x si1132 si114x si7005 slide sm130 smartdrive speaker ssd1351 st7735 stepmotor \
	sx1276 sx6119 t6713 ta12200 tca9548a tcs3414cs tcs37727 teams temperature \
	tex00 th02 tm1637 tmp006 tsl2561 ttp223 uartat uln200xa ultrasonic urm37 utilities vdiv \
	veml6070 water waterlevel wfs wheelencoder wt5001 xbee yg1006 zfm20 \
	vcap t3311 hwxpxx h803x ozw curieimu
# (require libbacnet) tb7300 t8100 e50hx bacnetmstp
# (require libtinyb) 2jciebu01_ble 2jciebu01_usb

CMAKE_OPTIONS += \
	-DBUILDSWIGNODE=OFF \
	-DWERROR=OFF \
	-DPYTHON2LIBS_FOUND=FALSE \
	-DPYTHON2INTERP_FOUND=FALSE \

define Package/libupm/Default
  SECTION:=libs
  CATEGORY:=Libraries
  SUBMENU:=IoT
  URL:=https://github.com/intel-iot-devkit/upm
  DEPENDS:=+libmraa +librt
endef

define Package/libupm/Default/description
  UPM is a high level repository that provides software drivers for a wide variety
of commonly used sensors and actuators. These software drivers interact with the
underlying hardware platform through calls to MRAA APIs.
  MRAA that allows you to use various sensors on platforms such as the Intel Galileo,
Intel Edison, Minnowboard MAX and others.
endef

define Package/libupm
  $(call Package/libupm/Default)
  TITLE:=Intel IoT sensor library - Full
  DEPENDS+=+libjpeg +libmodbus +openzwave
  DEPENDS+= $(foreach module, $(UPM_MODULES), +libupm-$(module))
  DEPENDS+= $(foreach module, $(UPM_MODULES), +libupm-$(module)-python3)
endef

define Package/libupm/description
$(call Package/libupm/Default/description)

This package contains sensor libraries
endef

define Package/libupm/install/Default
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libupm*-$(2).so* $(1)/usr/lib/ ;
endef

define Package/libupm/install/Default-python3
	$(INSTALL_DIR) $(1)/usr/lib/python$(PYTHON3_VERSION)/site-packages/upm
	$(CP) $(CMAKE_BINARY_DIR)/$(if $(filter interfaces, $(2)),,src/)$(2)/python$(PYTHON3_VERSION)/pyupm_$(2).py \
		$(1)/usr/lib/python$(PYTHON3_VERSION)/site-packages/upm/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/python$(PYTHON3_VERSION)/site-packages/upm/_pyupm_$(2).so \
		$(1)/usr/lib/python$(PYTHON3_VERSION)/site-packages/upm/ ;
endef

define Package/libupm/install
	$(foreach module, $(UPM_MODULES), \
		$(call Package/libupm/install/Default,$(1),$(module)) \
		$(call Package/libupm/install/Default-python3,$(1),$(module)))
endef

define UpmPackage
define UpmPackage/depends
  ifeq ($$(1),adafruitms1438)
    DEPENDS+=+libupm-pca9685
  endif
  ifeq ($$(1),ads1x15)
    DEPENDS+=+libupm-interfaces
  endif
  ifeq ($$(1),apa102)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($(1),bh1749)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),bh1750)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($(1),bh1792)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),bma250e)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),bmg160)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),bmi160)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),bmm150)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),bmp280)
    DEPENDS+=+libupm-interfaces +libupm-utilities
  endif
  ifeq ($$(1),bmpx8x)
    DEPENDS+=+libupm-interfaces +libupm-utilities
  endif
  ifeq ($$(1),bmx055)
    DEPENDS+=+libupm-utilities +libupm-bma250e +libupm-bmg160 +libupm-bmm150
  endif
  ifeq ($$(1),bno055)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),buzzer)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),cjq4435)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),dfrec)
    DEPENDS+=+libupm-utilities +libupm-ds18b20
  endif
  ifeq ($$(1),ds18b20)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),dfrorp)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),ds1808lc)
    DEPENDS+=+libupm-interfaces
  endif
  ifeq ($$(1),ecezo)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),enc03r)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),guvas12d)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),hcsr04)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),hka5)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),hlg150h)
    DEPENDS+=+libupm-interfaces
  endif
  ifeq ($$(1),ims)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),jhd1313m1)
    DEPENDS+=+libupm-utilities +libupm-lcm1602
  endif
  ifeq ($$(1),kxtj3)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),lcm1602)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),lcdks)
    DEPENDS+=+libupm-utilities +libupm-lcm1602
  endif
  ifeq ($$(1),lis2ds12)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),lis3dh)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),lp8860)
    DEPENDS+=+libupm-interfaces
  endif
  ifeq ($$(1),lsm303agr)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),lsm303d)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),lsm6ds3h)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),lsm6dsl)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),m24lr64e)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),max30100)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),max44009)
    DEPENDS+=+libupm-interfaces
  endif
  ifeq ($$(1),mb704x)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),mcp2515)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),md)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),mma7361)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),ms5611)
    DEPENDS+=+libupm-interfaces
  endif
  ifeq ($$(1),ms5803)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),my9221)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),nmea_gps)
    DEPENDS+=+libupm-utilities +libatomic
  endif
  ifeq ($$(1),nunchuck)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),otp538u)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),p9813)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),ppd42ns)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),rn2903)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),rsc)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),servo)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),sht1x)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),si1132)
    DEPENDS+=+libupm-interfaces
  endif
  ifeq ($$(1),si7005)
    DEPENDS+=+libupm-interfaces
  endif
  ifeq ($$(1),speaker)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),t6713)
    DEPENDS+=+libupm-interfaces
  endif
  ifeq ($$(1),tsl2561)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),uartat)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),uln200xa)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),urm37)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),wfs)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),zfm20)
    DEPENDS+=+libupm-utilities
  endif
  ifeq ($$(1),vcap)
    DEPENDS+=+libjpeg
  endif
  ifeq ($$(1),t3311)
    DEPENDS+=+libmodbus
  endif
  ifeq ($$(1),ozw)
    DEPENDS+=+openzwave
  endif
  ifeq ($$(1),hwxpxx)
    DEPENDS+=+libmodbus
  endif
  ifeq ($$(1),h803x)
    DEPENDS+=+libmodbus
  endif
endef

define Package/libupm-$(1)
  $(call Package/libupm/Default)
  $(call UpmPackage/depends,$(1))
  TITLE:=$(1) C/C++ library
endef

define Package/libupm-$(1)/description
$(call Package/libupm/Default/description)

This package contains $(1) sensor C/C++ library
endef

define Package/libupm-$(1)-python3
  $(call Package/libupm/Default)
  $(call UpmPackage/depends,$(1))
  TITLE:=$(1) Python3 library
  DEPENDS+=+libupm-$(1) +libmraa-python3 +python3-light
endef

define Package/libupm-$(1)-python3/description
$(call Package/libupm/Default/description)

This package contains $(1) sensor Python3 library
endef

define Package/libupm-$(1)/install
	$(call Package/libupm/install/Default,$$(1),$(1))
endef
define Package/libupm-$(1)-python3/install
	$(call Package/libupm/install/Default-python3,$$(1),$(1))
endef
endef

$(eval $(call BuildPackage,libupm))
$(foreach module, $(UPM_MODULES), \
	$(eval $(call UpmPackage,$(module))) \
	$(eval $(call BuildPackage,libupm-$(module))) \
	$(eval $(call BuildPackage,libupm-$(module)-python3)))
