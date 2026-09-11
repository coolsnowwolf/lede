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
setenv boot_direct booti ${kernel_addr} - ${dtb_addr}
setenv addmac 'if printenv mac; then setenv bootargs ${bootargs} mac=${mac}; elif printenv eth_mac; then setenv bootargs ${bootargs} mac=${eth_mac}; elif printenv ethaddr; then setenv bootargs ${bootargs} mac=${ethaddr}; fi'
if fatload ${devtype} ${devnum} ${env_addr} uEnv.txt && env import -t ${env_addr} ${filesize}; setenv bootargs ${APPEND}; then if fatload ${devtype} ${devnum} ${kernel_addr} ${LINUX}; then if fatload ${devtype} ${devnum} ${initrd_addr} ${INITRD}; then if fatload ${devtype} ${devnum} ${dtb_addr} ${FDT}; then run addmac; run boot_start; fi; fi; fi; fi;
setenv rootpart
setenv uuid
if test -z "${rootpart}"; then if part uuid ${devtype} ${devnum}:2 uuid; then setenv rootpart "PARTUUID=${uuid}"; else if test "${devtype}" = "usb"; then setenv rootpart "/dev/sda2"; else setenv rootpart "/dev/mmcblk0p2"; fi; fi; fi
setenv bootargs "console=tty0 no_console_suspend consoleblank=0 console=ttyAML0,115200n8 root=${rootpart} rw rootwait"
if fatload ${devtype} ${devnum} ${kernel_addr} kernel.img; then if fatload ${devtype} ${devnum} ${dtb_addr} amlogic.dtb; then run addmac; run boot_direct; fi; fi;
