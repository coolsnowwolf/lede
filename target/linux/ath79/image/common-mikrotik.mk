define Device/mikrotik
	DEVICE_VENDOR := MikroTik
	LOADER_TYPE := elf
	KERNEL := kernel-bin | append-dtb | lzma | loader-kernel
	KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | loader-kernel
endef
