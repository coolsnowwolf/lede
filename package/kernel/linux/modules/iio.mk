#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

IIO_MENU:=Industrial I/O Modules


define KernelPackage/iio-core
  SUBMENU:=$(IIO_MENU)
  TITLE:=Industrial IO core
  KCONFIG:= \
	CONFIG_IIO \
	CONFIG_IIO_BUFFER=y \
	CONFIG_IIO_TRIGGER=y
  FILES:=$(LINUX_DIR)/drivers/iio/industrialio.ko
  AUTOLOAD:=$(call AutoLoad,55,industrialio)
endef

define KernelPackage/iio-core/description
 The industrial I/O subsystem provides a unified framework for
 drivers for many different types of embedded sensors using a
 number of different physical interfaces (i2c, spi, etc)
endef

$(eval $(call KernelPackage,iio-core))


define KernelPackage/iio-kfifo-buf
  SUBMENU:=$(IIO_MENU)
  TITLE:=Industrial I/O buffering based on kfifo
  DEPENDS:=+kmod-iio-core
  KCONFIG:=CONFIG_IIO_KFIFO_BUF
  FILES:=$(LINUX_DIR)/drivers/iio/buffer/kfifo_buf.ko
  AUTOLOAD:=$(call AutoLoad,55,kfifo_buf)
endef

define KernelPackage/iio-kfifo-buf/description
 A simple fifo based on kfifo.  Note that this currently provides no buffer
 events so it is up to userspace to work out how often to read from the buffer.
endef

$(eval $(call KernelPackage,iio-kfifo-buf))


define KernelPackage/industrialio-triggered-buffer
  SUBMENU:=$(IIO_MENU)
  TITLE:=Provides helper functions for setting up triggered buffers.
  DEPENDS:=+kmod-iio-core +kmod-iio-kfifo-buf
  KCONFIG:=CONFIG_IIO_TRIGGERED_BUFFER
  FILES:=$(LINUX_DIR)/drivers/iio/buffer/industrialio-triggered-buffer.ko
  AUTOLOAD:=$(call AutoLoad,55,industrialio-triggered-buffer)
endef

define KernelPackage/industrialio-triggered-buffer/description
 Provides helper functions for setting up triggered buffers.
endef

$(eval $(call KernelPackage,industrialio-triggered-buffer))


define KernelPackage/iio-ad799x
  SUBMENU:=$(IIO_MENU)
  DEPENDS:=+kmod-i2c-core +kmod-iio-core +kmod-industrialio-triggered-buffer
  TITLE:=Analog Devices AD799x ADC driver
  KCONFIG:= \
	CONFIG_AD799X_RING_BUFFER=y \
	CONFIG_AD799X
  FILES:=$(LINUX_DIR)/drivers/iio/adc/ad799x.ko
  AUTOLOAD:=$(call AutoLoad,56,ad799x)
endef

define KernelPackage/iio-ad799x/description
 support for Analog Devices:
 ad7991, ad7995, ad7999, ad7992, ad7993, ad7994, ad7997, ad7998
 i2c analog to digital converters (ADC).
endef

$(eval $(call KernelPackage,iio-ad799x))

define KernelPackage/iio-hmc5843
  SUBMENU:=$(IIO_MENU)
  DEPENDS:=+kmod-i2c-core +kmod-iio-core +kmod-regmap-i2c +kmod-industrialio-triggered-buffer
  TITLE:=Honeywell HMC58x3 Magnetometer
  KCONFIG:= CONFIG_SENSORS_HMC5843_I2C
  FILES:= \
      $(LINUX_DIR)/drivers/iio/magnetometer/hmc5843_i2c.ko \
      $(LINUX_DIR)/drivers/iio/magnetometer/hmc5843_core.ko
  AUTOLOAD:=$(call AutoLoad,56,hmc5843)
endef

define KernelPackage/iio-hmc5843/description
  Honeywell HMC5843/5883/5883L 3-Axis Magnetometer
endef

$(eval $(call KernelPackage,iio-hmc5843))

define KernelPackage/iio-bh1750
  SUBMENU:=$(IIO_MENU)
  DEPENDS:=+kmod-i2c-core +kmod-iio-core
  TITLE:=ROHM BH1750 ambient light sensor
  KCONFIG:= CONFIG_BH1750
  FILES:=$(LINUX_DIR)/drivers/iio/light/bh1750.ko
  AUTOLOAD:=$(call AutoLoad,56,bh1750)
endef
define KernelPackage/iio-bh1750/description
  ROHM BH1750 ambient light sensor (i2c bus)
endef
$(eval $(call KernelPackage,iio-bh1750))

define KernelPackage/iio-am2315
  SUBMENU:=$(IIO_MENU)
  DEPENDS:=+kmod-i2c-core +kmod-iio-core +kmod-industrialio-triggered-buffer
  TITLE:=Asong AM2315 humidity/temperature sensor
  KCONFIG:= CONFIG_AM2315
  FILES:=$(LINUX_DIR)/drivers/iio/humidity/am2315.ko
  AUTOLOAD:=$(call AutoLoad,56,am2315)
endef
define KernelPackage/iio-am2315/description
  Aosong AM2315 humidity/temperature sensor (I2C bus)
endef
$(eval $(call KernelPackage,iio-am2315))

define KernelPackage/iio-mxs-lradc
  SUBMENU:=$(IIO_MENU)
  DEPENDS:=@TARGET_mxs +kmod-iio-core +kmod-industrialio-triggered-buffer
  TITLE:=Freescale i.MX23/i.MX28 LRADC ADC driver
  KCONFIG:= \
	CONFIG_MXS_LRADC_ADC
  FILES:=$(LINUX_DIR)/drivers/iio/adc/mxs-lradc-adc.ko
  AUTOLOAD:=$(call AutoLoad,56,mxs-lradc-adc)
endef

define KernelPackage/iio-mxs-lradc/description
 Support for Freescale's i.MX23/i.MX28 SoC internal Low-Resolution ADC
endef

$(eval $(call KernelPackage,iio-mxs-lradc))

define KernelPackage/iio-dht11
  SUBMENU:=$(IIO_MENU)
  DEPENDS:=+kmod-iio-core @GPIO_SUPPORT @USES_DEVICETREE
  TITLE:=DHT11 (and compatible) humidity and temperature sensors
  KCONFIG:= \
	CONFIG_DHT11
  FILES:=$(LINUX_DIR)/drivers/iio/humidity/dht11.ko
  AUTOLOAD:=$(call AutoLoad,56,dht11)
endef

define KernelPackage/iio-dht11/description
 support for DHT11 and DHT22 digitial humidity and temperature sensors
 attached at GPIO lines. You will need a custom device tree file to
 specify the GPIO line to use.
endef

$(eval $(call KernelPackage,iio-dht11))


define KernelPackage/iio-bme680
  SUBMENU:=$(IIO_MENU)
  TITLE:=BME680 gas/humidity/pressure/temperature sensor
  DEPENDS:=@!LINUX_4_14 +kmod-iio-core +kmod-regmap-core
  KCONFIG:=CONFIG_BME680
  FILES:=$(LINUX_DIR)/drivers/iio/chemical/bme680_core.ko
endef

define KernelPackage/iio-bme680/description
 This driver adds support for Bosch Sensortec BME680 sensor with gas,
 humidity, pressure and temperatue sensing capability.
endef

$(eval $(call KernelPackage,iio-bme680))

define KernelPackage/iio-bme680-i2c
  SUBMENU:=$(IIO_MENU)
  TITLE:=BME680 gas/humidity/pressure/temperature sensor (I2C)
  DEPENDS:=+kmod-iio-bme680 +kmod-regmap-i2c
  KCONFIG:=CONFIG_BME680_I2C
  FILES:=$(LINUX_DIR)/drivers/iio/chemical/bme680_i2c.ko
  AUTOLOAD:=$(call AutoProbe,bme680-i2c)
endef
define KernelPackage/iio-bme680-i2c/description
 This driver adds support for Bosch Sensortec's BME680 connected via I2C.
endef

$(eval $(call KernelPackage,iio-bme680-i2c))

define KernelPackage/iio-bme680-spi
  SUBMENU:=$(IIO_MENU)
  TITLE:=BME680 gas/humidity/pressure/temperature sensor (SPI)
  DEPENDS:=+kmod-iio-bme680 +kmod-regmap-spi
  KCONFIG:=CONFIG_BME680_SPI
  FILES:=$(LINUX_DIR)/drivers/iio/chemical/bme680_spi.ko
  AUTOLOAD:=$(call AutoProbe,bme680-spi)
endef
define KernelPackage/iio-bme680-spi/description
 This driver adds support for Bosch Sensortec's BME680 connected via SPI.
endef

$(eval $(call KernelPackage,iio-bme680-spi))


define KernelPackage/iio-bmp280
  SUBMENU:=$(IIO_MENU)
  TITLE:=BMP180/BMP280/BME280 pressure/temperatur sensor
  DEPENDS:=+kmod-iio-core +kmod-regmap-core
  KCONFIG:=CONFIG_BMP280
  FILES:=$(LINUX_DIR)/drivers/iio/pressure/bmp280.ko
endef

define KernelPackage/iio-bmp280/description
 This driver adds support for Bosch Sensortec BMP180 and BMP280 pressure and
 temperature sensors. Also supports the BME280 with an additional humidity
 sensor channel.
endef

$(eval $(call KernelPackage,iio-bmp280))


define KernelPackage/iio-bmp280-i2c
  SUBMENU:=$(IIO_MENU)
  TITLE:=BMP180/BMP280/BME280 pressure/temperatur sensor (I2C)
  DEPENDS:=+kmod-iio-bmp280 +kmod-i2c-core +kmod-regmap-i2c
  KCONFIG:=CONFIG_BMP280_I2C
  FILES:=$(LINUX_DIR)/drivers/iio/pressure/bmp280-i2c.ko
  AUTOLOAD:=$(call AutoProbe,bmp280-i2c)
endef
define KernelPackage/iio-bmp280-i2c/description
 This driver adds support for Bosch Sensortec's digital pressure and
 temperature sensor connected via I2C.
endef

$(eval $(call KernelPackage,iio-bmp280-i2c))


define KernelPackage/iio-bmp280-spi
  SUBMENU:=$(IIO_MENU)
  TITLE:=BMP180/BMP280/BME280 pressure/temperatur sensor (SPI)
  DEPENDS:=+kmod-iio-bmp280 +kmod-spi-bitbang
  KCONFIG:=CONFIG_BMP280_SPI
  FILES:=$(LINUX_DIR)/drivers/iio/pressure/bmp280-spi.ko
  AUTOLOAD:=$(call AutoProbe,bmp280-spi)
endef
define KernelPackage/iio-bmp280-spi/description
 This driver adds support for Bosch Sensortec's digital pressure and
 temperature sensor connected via SPI.
endef

$(eval $(call KernelPackage,iio-bmp280-spi))

define KernelPackage/iio-htu21
  SUBMENU:=$(IIO_MENU)
  DEPENDS:=+kmod-i2c-core +kmod-iio-core
  TITLE:=HTU21 humidity & temperature sensor
  KCONFIG:= \
       CONFIG_HTU21 \
       CONFIG_IIO_MS_SENSORS_I2C
  FILES:= \
       $(LINUX_DIR)/drivers/iio/humidity/htu21.ko \
       $(LINUX_DIR)/drivers/iio/common/ms_sensors/ms_sensors_i2c.ko
  AUTOLOAD:=$(call AutoLoad,56,htu21)
endef

define KernelPackage/iio-htu21/description
 support for the Measurement Specialties HTU21 humidity and
 temperature sensor.
 This driver is also used for MS8607 temperature, pressure & humidity
 sensor
endef

$(eval $(call KernelPackage,iio-htu21))


define KernelPackage/iio-ccs811
  SUBMENU:=$(IIO_MENU)
  DEPENDS:=+kmod-i2c-core +kmod-iio-core +kmod-industrialio-triggered-buffer
  TITLE:=AMS CCS811 VOC sensor
  KCONFIG:= \
	CONFIG_CCS811
  FILES:= $(LINUX_DIR)/drivers/iio/chemical/ccs811.ko
  AUTOLOAD:=$(call AutoLoad,56,ccs811)
endef

define KernelPackage/iio-ccs811/description
  Support for the AMS CCS811 VOC (Volatile Organic Compounds) sensor
endef

$(eval $(call KernelPackage,iio-ccs811))


define KernelPackage/iio-si7020
  SUBMENU:=$(IIO_MENU)
  DEPENDS:=+kmod-i2c-core +kmod-iio-core
  TITLE:=Silicon Labs Si7020 sensor
  KCONFIG:= CONFIG_SI7020
  FILES:=$(LINUX_DIR)/drivers/iio/humidity/si7020.ko
  AUTOLOAD:=$(call AutoLoad,56,si7020)
endef

define KernelPackage/iio-si7020/description
 Support for Silicon Labs Si7020 family of relative humidity and
 temperature sensors connected via I2C. Following models are usable:
 Si7013, Si7020, Si7021, Hoperf TH06.
endef

$(eval $(call KernelPackage,iio-si7020))


define KernelPackage/iio-st_accel
  SUBMENU:=$(IIO_MENU)
  TITLE:=STMicroelectronics accelerometer 3-Axis Driver
  DEPENDS:=+kmod-iio-core +kmod-regmap-core +kmod-industrialio-triggered-buffer
  KCONFIG:= \
	CONFIG_IIO_ST_ACCEL_3AXIS \
	CONFIG_IIO_ST_SENSORS_CORE
  FILES:= \
	$(LINUX_DIR)/drivers/iio/accel/st_accel.ko \
	$(LINUX_DIR)/drivers/iio/common/st_sensors/st_sensors.ko
endef

define KernelPackage/iio-st_accel/description
 This package adds support for STMicroelectronics accelerometers:
  LSM303DLH, LSM303DLHC, LIS3DH, LSM330D, LSM330DL, LSM330DLC,
  LIS331DLH, LSM303DL, LSM303DLM, LSM330, LIS2DH12, H3LIS331DL,
  LNG2DM, LIS3DE, LIS2DE12
endef

$(eval $(call KernelPackage,iio-st_accel))


define KernelPackage/iio-st_sensors-i2c
  SUBMENU:=$(IIO_MENU)
  TITLE:=STMicroelectronics accelerometer 3-Axis Driver (I2C)
  DEPENDS:=+kmod-iio-st_accel +kmod-i2c-core +kmod-regmap-i2c
  KCONFIG:= CONFIG_IIO_ST_ACCEL_I2C_3AXIS
  FILES:=$(LINUX_DIR)/drivers/iio/common/st_sensors/st_sensors_i2c.ko
  AUTOLOAD:=$(call AutoLoad,56,st_sensors_i2c)
endef

define KernelPackage/iio-st_sensors-i2c/description
 This package adds support for STMicroelectronics I2C based accelerometers
endef

$(eval $(call KernelPackage,iio-st_sensors-i2c))


define KernelPackage/iio-sps30
  SUBMENU:=$(IIO_MENU)
  DEPENDS:=@!LINUX_4_14 +kmod-i2c-core +kmod-iio-core +kmod-industrialio-triggered-buffer +kmod-lib-crc8
  TITLE:=Sensirion SPS30 particulate matter sensor
  KCONFIG:=CONFIG_SPS30
  FILES:=$(LINUX_DIR)/drivers/iio/chemical/sps30.ko
  AUTOLOAD:=$(call AutoProbe,sps30)
endef

define KernelPackage/iio-sps30/description
 Support for the Sensirion SPS30 particulate matter sensor.
endef

$(eval $(call KernelPackage,iio-sps30))


define KernelPackage/iio-st_sensors-spi
  SUBMENU:=$(IIO_MENU)
  TITLE:=STMicroelectronics accelerometer 3-Axis Driver (SPI)
  DEPENDS:=+kmod-iio-st_accel +kmod-regmap-spi
  KCONFIG:= CONFIG_IIO_ST_ACCEL_SPI_3AXIS
  FILES:=$(LINUX_DIR)/drivers/iio/common/st_sensors/st_sensors_spi.ko
  AUTOLOAD:=$(call AutoLoad,56,st_sensors_spi)
endef

define KernelPackage/iio-st_sensors-spi/description
 This package adds support for STMicroelectronics SPI based accelerometers
endef

$(eval $(call KernelPackage,iio-st_sensors-spi))


define KernelPackage/iio-tsl4531
  SUBMENU:=$(IIO_MENU)
  DEPENDS:=+kmod-i2c-core +kmod-iio-core
  TITLE:=TAOS TSL4531 ambient light sensor
  KCONFIG:= CONFIG_TSL4531
  FILES:=$(LINUX_DIR)/drivers/iio/light/tsl4531.ko
  AUTOLOAD:=$(call AutoLoad,56,tsl4531)
endef

define KernelPackage/iio-tsl4531/description
 Support for TAOS TSL4531x family of ambient light sensors
 connected via I2C. Following models are usable:
 TSL45311, TSL45313, TSL45315, TSL45317.
endef

$(eval $(call KernelPackage,iio-tsl4531))


define KernelPackage/iio-fxos8700
  SUBMENU:=$(IIO_MENU)
  TITLE:=Freescale FXOS8700 3-axis accelerometer driver
  DEPENDS:=+kmod-iio-core +kmod-regmap-core
  KCONFIG:= CONFIG_FXOS8700
  FILES:=$(LINUX_DIR)/drivers/iio/imu/fxos8700_core.ko
  AUTOLOAD:=$(call AutoLoad,56,fxos8700)
endef

define KernelPackage/iio-fxos8700/description
 Support for Freescale FXOS8700 3-axis accelerometer.
endef

$(eval $(call KernelPackage,iio-fxos8700))


define KernelPackage/iio-fxos8700-i2c
  SUBMENU:=$(IIO_MENU)
  TITLE:=Freescale FXOS8700 3-axis acceleromter driver (I2C)
  DEPENDS:=+kmod-iio-fxos8700 +kmod-i2c-core +kmod-regmap-i2c
  KCONFIG:= CONFIG_FXOS8700_I2C
  FILES:=$(LINUX_DIR)/drivers/iio/imu/fxos8700_i2c.ko
  AUTOLOAD:=$(call AutoLoad,56,fxos8700_i2c)
endef

define KernelPackage/iio-fxos8700-i2c/description
 Support for Freescale FXOS8700 3-axis accelerometer
 connected via I2C.
endef


$(eval $(call KernelPackage,iio-fxos8700-i2c))

define KernelPackage/iio-fxos8700-spi
  SUBMENU:=$(IIO_MENU)
  DEPENDS:=+kmod-iio-fxos8700 +kmod-regmap-spi
  TITLE:=Freescale FXOS8700 3-axis accelerometer driver (SPI)
  KCONFIG:= CONFIG_FXOS8700_SPI
  FILES:=$(LINUX_DIR)/drivers/iio/imu/fxos8700_spi.ko
  AUTOLOAD:=$(call AutoLoad,56,fxos8700_spi)
endef

define KernelPackage/iio-fxos8700-spi/description
 Support for Freescale FXOS8700 3-axis accelerometer
 connected via SPI.
endef

$(eval $(call KernelPackage,iio-fxos8700-spi))
