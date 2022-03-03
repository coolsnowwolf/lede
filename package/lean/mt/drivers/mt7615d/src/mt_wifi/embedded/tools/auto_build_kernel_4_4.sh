if [ "${1}" == "AP" ]; then
    echo "Wifi-Prebuild: wifi mode is AP"
    wifi_mode=AP
elif [ "${1}" == "STA" ]; then
    echo "Wifi-Prebuild: wifi mode is STA"
    wifi_mode=STA
else
    echo "Wifi-Prebuild: wifi mode is not specified, default wifi mode is AP"
    wifi_mode=AP
fi

if [ -d wifi_driver ]; then
    echo "Wifi-Prebuild: kernel-4.4.x wifi driver pre-build"
    mkdir -p mt_wifi_ap
    mkdir -p mt_wifi_sta
    cp -a  wifi_driver/os/linux/Kconfig.mt_wifi_ap ./mt_wifi_ap/Kconfig
    cp -a  wifi_driver/os/linux/Makefile.mt_wifi_ap ./mt_wifi_ap/Makefile
    cp -a  wifi_driver/os/linux/Kconfig.mt_wifi_sta ./mt_wifi_sta/Kconfig
    cp -a  wifi_driver/os/linux/Makefile.mt_wifi_sta ./mt_wifi_sta/Makefile
    cp -a wifi_driver/os/linux/Kconfig.mt_wifi_4_4 wifi_driver/embedded/Kconfig
#    ln -sf  ../../../../../proprietary_driver/drivers/wifi_utility wifi_utility
    if [ -d mt_wifi ]; then
        rm -rf mt_wifi
    fi
    mv wifi_driver mt_wifi
    echo "Wifi-Prebuild: build 7622 binary"
    RT28xx_DIR=./mt_wifi
    CHIPSET=mt7622
    RT28xx_MODE=${wifi_mode}
    HAS_WOW_SUPPORT=n
    HAS_FPGA_MODE=n
    HAS_RX_CUT_THROUGH=n
    RT28xx_BIN_DIR=.
    export RT28xx_DIR CHIPSET RT28xx_MODE HAS_WOW_SUPPORT HAS_FPGA_MODE HAS_RX_CUT_THROUGH RT28xx_BIN_DIR
    make -C mt_wifi/embedded build_tools
    ./mt_wifi/embedded/tools/bin2h
    make -C mt_wifi/embedded build_sku_tables
    ./mt_wifi/txpwr/dat2h
    echo "Wifi-Prebuild: build 7615 binary"
    RT28xx_DIR=./mt_wifi
    CHIPSET=mt7615
    RT28xx_MODE=${wifi_mode}
    HAS_WOW_SUPPORT=n
    HAS_FPGA_MODE=n
    HAS_RX_CUT_THROUGH=n
    RT28xx_BIN_DIR=.
    export RT28xx_DIR CHIPSET RT28xx_MODE HAS_WOW_SUPPORT HAS_FPGA_MODE HAS_RX_CUT_THROUGH RT28xx_BIN_DIR
    ./mt_wifi/embedded/tools/bin2h
#Makefile modify for drivers/net/wireless/mediatek/Makefile
if grep -q CONFIG_MT_AP_SUPPORT Makefile; then
    echo "Wifi-Prebuild: Makefile already modified. Skip."
else
#    echo "obj-y += wifi_utility/" >> Makefile
    echo "obj-\$(CONFIG_MT_AP_SUPPORT) += mt_wifi_ap/" >> Makefile
    echo "obj-\$(CONFIG_MT_STA_SUPPORT) += mt_wifi_sta/" >> Makefile
fi
#Kconfig modify for drivers/net/wireless/mediatek/Kconfig
if grep -q WIFI_DRIVER Kconfig; then
    echo "Wifi-Prebuild: Kconfig already modified. Skip."
else
sed -i 's/endif # WL_MEDIATEK/\
menuconfig WIFI_DRIVER\
	bool "WiFi Driver Support"\
\
if WIFI_DRIVER\
\
choice\
	prompt "Choose First WiFi Interface"\
	config FIRST_IF_NONE\
	bool "None"\
\
	config FIRST_IF_MT7615E\
	bool "MT7615E"\
	select WIFI_MT_MAC\
	select MT_MAC\
	select CHIP_MT7615E\
\
	config FIRST_IF_MT7622\
	bool "MT7622"\
	select WIFI_MT_MAC\
	select MT_MAC\
	select CHIP_MT7622\
\
endchoice\
\
choice\
	prompt "Choose Second WiFi Interface"\
	config SECOND_IF_NONE\
	bool "None"\
\
	config SECOND_IF_MT7615E\
	bool "MT7615E"\
	select WIFI_MT_MAC\
	select CHIP_MT7615E\
	select MULTI_INF_SUPPORT\
\
endchoice\
\
choice\
	prompt "Choose Third WiFi Interface"\
	config THIRD_IF_NONE\
	bool "None"\
\
	config THIRD_IF_MT7615E\
	bool "MT7615E"\
	select WIFI_MT_MAC\
	select CHIP_MT7615E\
	select MULTI_INF_SUPPORT\
\
endchoice\
\
config  RT_FIRST_CARD\
        int\
        depends on ! FIRST_IF_NONE\
	default 7615 if FIRST_IF_MT7615E\
        default 7622 if FIRST_IF_MT7622\
\
config  RT_SECOND_CARD\
        int\
        depends on ! SECOND_IF_NONE\
        default 7615 if SECOND_IF_MT7615E\
\
config  RT_THIRD_CARD\
        int\
        depends on ! THIRD_IF_NONE\
        default 7615 if THIRD_IF_MT7615E\
\
config  RT_FIRST_IF_RF_OFFSET\
        hex\
        depends on ! FIRST_IF_NONE\
        default 0xc0000\
\
config  RT_SECOND_IF_RF_OFFSET\
        hex\
        depends on ! SECOND_IF_NONE\
        default 0xc8000\
\
config  RT_THIRD_IF_RF_OFFSET\
        hex\
        depends on ! THIRD_IF_NONE\
        default 0xd0000\
\
source \"drivers\/net\/wireless\/mediatek\/mt_wifi\/embedded\/Kconfig\"\
endif # WIFI_DRIVER\
endif # WL_MEDIATEK\
\
/g' Kconfig
fi

#source "drivers/net/wireless/mediatek/mt_wifi/embedded/Kconfig"\
else
    exit 1
fi
