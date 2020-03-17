ifndef DUMP
  ifdef __package_mk
    $(error uclibc++.mk must be included before package.mk)
  endif
endif

PKG_PREPARED_DEPENDS += CONFIG_USE_UCLIBCXX CONFIG_USE_LIBCXX
CXX_DEPENDS = +USE_UCLIBCXX:uclibcxx +USE_LIBCXX:libcxx +USE_LIBSTDCXX:libstdcpp

ifneq ($(CONFIG_USE_UCLIBCXX),)
 ifneq ($(CONFIG_CCACHE),)
  TARGET_CXX_NOCACHE=g++-uc
 else
  TARGET_CXX=g++-uc
 endif
endif

ifneq ($(CONFIG_USE_LIBCXX),)
 ifneq ($(CONFIG_CCACHE),)
  TARGET_CXX_NOCACHE=g++-libcxx
 else
  TARGET_CXX=g++-libcxx
 endif
endif
