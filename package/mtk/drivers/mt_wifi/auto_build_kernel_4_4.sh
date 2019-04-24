if [ -d wifi_driver ]; then
    mkdir -p mt_wifi_ap
    mkdir -p mt_wifi_sta
    cp -a  wifi_driver/os/linux/Kconfig.mt_wifi_ap ./mt_wifi_ap/Kconfig
    cp -a  wifi_driver/os/linux/Makefile.mt_wifi_ap ./mt_wifi_ap/Makefile
    cp -a  wifi_driver/os/linux/Kconfig.mt_wifi_sta ./mt_wifi_sta/Kconfig
    cp -a  wifi_driver/os/linux/Makefile.mt_wifi_sta ./mt_wifi_sta/Makefile
    cp -a  wifi_driver/os/linux/Kconfig.mt_wifi wifi_driver/embedded/Kconfig
    cp -rf wifi_driver mt_wifi
    #if [ -d mt_wifi ]; then
    #    rm -rf mt_wifi
    #fi
    #mv wifi_driver mt_wifi
    echo "7622 mt_wifi autobuild"
    RT28xx_DIR=./mt_wifi
    CHIPSET=mt7622
    RT28xx_MODE=AP
    HAS_WOW_SUPPORT=n
    HAS_FPGA_MODE=n
    HAS_RX_CUT_THROUGH=n
    RT28xx_BIN_DIR=.
    export RT28xx_DIR CHIPSET RT28xx_MODE HAS_WOW_SUPPORT HAS_FPGA_MODE HAS_RX_CUT_THROUGH RT28xx_BIN_DIR
    make -C mt_wifi/embedded build_tools
    ./mt_wifi/embedded/tools/bin2h
    make -C mt_wifi/embedded build_sku_tables
    ./mt_wifi/txpwr/dat2h
    echo "7615 mt_wifi autobuild"
    RT28xx_DIR=./mt_wifi
    CHIPSET=mt7615
    RT28xx_MODE=AP
    HAS_WOW_SUPPORT=n
    HAS_FPGA_MODE=n
    HAS_RX_CUT_THROUGH=n
    RT28xx_BIN_DIR=.
    export RT28xx_DIR CHIPSET RT28xx_MODE HAS_WOW_SUPPORT HAS_FPGA_MODE HAS_RX_CUT_THROUGH RT28xx_BIN_DIR
    make -C mt_wifi/embedded build_tools
    ./mt_wifi/embedded/tools/bin2h
    make -C mt_wifi/embedded build_sku_tables
    ./mt_wifi/txpwr/dat2h
    echo "7626 mt_wifi autobuild"
    RT28xx_DIR=./mt_wifi
    CHIPSET=mt7626
    RT28xx_MODE=AP
    HAS_WOW_SUPPORT=n
    HAS_FPGA_MODE=n
    HAS_RX_CUT_THROUGH=n
    RT28xx_BIN_DIR=.
    export RT28xx_DIR CHIPSET RT28xx_MODE HAS_WOW_SUPPORT HAS_FPGA_MODE HAS_RX_CUT_THROUGH RT28xx_BIN_DIR
    make -C mt_wifi/embedded build_tools
    ./mt_wifi/embedded/tools/bin2h
    make -C mt_wifi/embedded build_sku_tables
    ./mt_wifi/txpwr/dat2h
elif [ -d rlt_wifi ]; then
    mkdir -p mt_wifi_ap
    mkdir -p mt_wifi_sta
    cp -a  rlt_wifi/os/linux/Kconfig.mt_wifi_ap ./mt_wifi_ap/Kconfig
    cp -a  rlt_wifi/os/linux/Makefile.mt_wifi_ap ./mt_wifi_ap/Makefile
    cp -a  rlt_wifi/os/linux/Kconfig.mt_wifi_sta ./mt_wifi_sta/Kconfig
    cp -a  rlt_wifi/os/linux/Makefile.mt_wifi_sta ./mt_wifi_sta/Makefile
    cp -a rlt_wifi/os/linux/Kconfig.mt_wifi rlt_wifi/embedded/Kconfig
    if [ -d mt_wifi ]; then
        rm -rf mt_wifi
    fi
    mv rlt_wifi mt_wifi
    echo "7615e rlt_wifi autobuild"
    RT28xx_DIR=./mt_wifi
    CHIPSET=mt7615
    RT28xx_MODE=AP
    HAS_WOW_SUPPORT=n
    HAS_FPGA_MODE=n
    HAS_RX_CUT_THROUGH=n
    RT28xx_BIN_DIR=.
    export RT28xx_DIR CHIPSET RT28xx_MODE HAS_WOW_SUPPORT HAS_FPGA_MODE HAS_RX_CUT_THROUGH RT28xx_BIN_DIR
    make -C mt_wifi/embedded build_tools
    ./mt_wifi/embedded/tools/bin2h
    make -C mt_wifi/embedded build_sku_tables
    ./mt_wifi/txpwr/dat2h
else
    exit 1
fi
