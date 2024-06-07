if test "${usbdev}" = ""; then
	setenv devtype "mmc"
	setenv devnum 0
else
	setenv devtype "usb"
	setenv devnum ${usbdev}
fi

if fatload ${devtype} ${devnum} 0x1000000 u-boot.emmc; then go 0x1000000; fi;
setenv dtb_addr 0x1000000
setenv env_addr 0x1040000
setenv kernel_addr 0x11000000
setenv initrd_addr 0x13000000
setenv boot_start booti ${kernel_addr} ${initrd_addr} ${dtb_addr}
setenv addmac 'if printenv mac; then setenv bootargs ${bootargs} mac=${mac}; elif printenv eth_mac; then setenv bootargs ${bootargs} mac=${eth_mac}; elif printenv ethaddr; then setenv bootargs ${bootargs} mac=${ethaddr}; fi'
if fatload ${devtype} ${devnum} ${env_addr} uEnv.txt && env import -t ${env_addr} ${filesize}; setenv bootargs ${APPEND}; then if fatload ${devtype} ${devnum} ${kernel_addr} ${LINUX}; then if fatload ${devtype} ${devnum} ${initrd_addr} ${INITRD}; then if fatload ${devtype} ${devnum} ${dtb_addr} ${FDT}; then run addmac; run boot_start; fi; fi; fi; fi;
