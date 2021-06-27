ifndef DUMP
  ifdef __package_mk
    $(error uclibc++.mk must be included before package.mk)
  endif
endif

PKG_PREPARED_DEPENDS += CONFIG_USE_UCLIBCXX
CXX_DEPENDS = +USE_UCLIBCXX:uclibcxx +USE_LIBSTDCXX:libstdcpp

ifneq ($(CONFIG_USE_UCLIBCXX),)
 ifneq ($(CONFIG_CCACHE),)
  TARGET_CXX_NOCACHE=g++-uc
 else
  TARGET_CXX=g++-uc
 endif
endif
