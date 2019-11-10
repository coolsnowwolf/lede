define Build/buffalo-tftp-header
	( \
		echo -n -e "# Airstation Public Fmt1" | dd bs=32 count=1 conv=sync; \
		dd if=$@; \
	) > $@.new
  mv $@.new $@
endef

define Build/buffalo-tag
	$(eval product=$(word 1,$(1)))
	$(eval hwver=$(word 2,$(1)))
	$(STAGING_DIR_HOST)/bin/buffalo-tag \
		-c 0x80041000 -d 0x801e8000 -w $(hwver) \
		-a ath -v 1.99 -m 1.01 -f 1 \
		-b $(product) -p $(product) \
		-r M_ -l mlang8 \
		-i $@ -o $@.new
	mv $@.new $@
endef


