define Build/yuncore-tftp-header-16m
	( \
		echo -n -e \
			"YUNCOREsetenv bootcmd \"bootm 0x9f050000 || bootm 0x9fe80000\"" \
			"&& saveenv" \
			"&& erase 0x9f050000 +0xfa0000" \
			"&& cp.b 0x800600c0 0x9f050000 0xfa0000" |\
		dd bs=192 count=1 conv=sync; \
		dd if=$@; \
	) > $@.new
	mv $@.new $@
endef
