本驱动支持核外编译方式，编译方式如下：
CROSS_COMPILE=/usr/bin/ source drm_configure.sh /lib/modules/$(uname -r)/build 3500 ARM64 && m
生成的驱动名为ftd330-drm-dc.ko


如果想放到核内去编译，需要执行如下操作：

1、cd linux_source_code

2、cp  drivers/gpu/drm/ftd330/include/uapi/drm/ftd330_drm* include/uapi/drm/

3、mv ftd330 ftd330-back && cp -r ftd330-back/phytium leoaprd

4、在drivers/gpu/drm/Makefile文件的末尾加入这一行：
obj-$(CONFIG_DRM_PHYTIUM) += ftd330/

5、在drivers/gpu/drm/Kconfig文件中加入这一行：
source "drivers/gpu/drm/ftd330/Kconfig"

6、make menuconfig，然后对本驱动相关的配置项进行如下配置：
CONFIG_DRM_PHYTIUM=y
# CONFIG_PHYTIUM_REGMAP is not set
CONFIG_PHYTIUM_CHIP_FTD330=y
CONFIG_PHYTIUM_NCC=y
# CONFIG_PHYTIUM_WRITEBACK is not set
CONFIG_PHYTIUM_MMU=y
CONFIG_PHYTIUM_DEC=y
# CONFIG_PHYTIUM_POWER_OPERATION is not set
# CONFIG_PHYTIUM_VIRTUAL_DISPLAY is not set
# CONFIG_PHYTIUM_PVRIC is not set
# CONFIG_PHYTIUM_FBDEV_ON is not set
# CONFIG_PHYTIUM_CHANGE_PIXCLK is not set
# CONFIG_PHYTIUM_LANE_TRAIN is not set
# CONFIG_PHYTIUM_PCIE is not set
# CONFIG_PHYTIUM_EDP_BL is not set
# CONFIG_PHYTIUM_PSR is not set
# CONFIG_PHYTIUM_LOW_FPS is not set

7、建议的设备树：
               dcFTD330@26ca0000 {
                                compatible = "phytium,dc-1.0";
                                reg = <0x00 0x26ca0000 0x00 0x1e000 0x22 0x00 0x00 0x20000000 0x00 0x26fcc080 0x00 0x100>;
                                interrupts = <0x00 0x37 0x04>, <0x00 0x3a 0x04>, <0x00 0x41 0x04>, <0x00 0x42 0x04>, <0x00 0x43 0x04>, <0x00 0x6a 0x04>, <0x00 0x6b 0x04>, <0x00 0x6c 0x04>;
                                pipe_mask = [07];
                                phy_mode = <0x01 0x01 0x01>;
                                edp_mask = [01];
                                water_mark = <0x5666 0x5666 0x5666>;
                                qos = <0xf0 0xf0>;
                                overlay_enable = [01];
                                #address-cells = <0x01>;
                                #size-cells = <0x00>;
                };
其中reg的第二段地址为DC的帧存起始地址。
