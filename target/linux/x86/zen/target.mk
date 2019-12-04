ARCH:=x86_64
BOARDNAME:=AMD Ryzen (The Zen Core Architecture)
CPU_TYPE :=znver1
DEFAULT_PACKAGES += kmod-button-hotplug kmod-e1000e kmod-e1000 kmod-r8169 kmod-igb kmod-bnx2

define Target/Description
        Build images for 64 bit systems including virtualized guests.
endef
