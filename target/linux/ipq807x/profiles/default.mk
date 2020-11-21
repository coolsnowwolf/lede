define Profile/Default
	NAME:=Default Profile (minimum package set)
endef

define Profile/Default/Description
	Default package set compatible with most boards.
endef
$(eval $(call Profile,Default))
