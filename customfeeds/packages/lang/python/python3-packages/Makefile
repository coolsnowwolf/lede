#
# Copyright (C) 2019 Yousong Zhou <yszhou4tech@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python3-packages
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_MAINTAINER:=Yousong Zhou <yszhou4tech@gmail.com>

#
# NOTE: move the host module installation to Host/Compile when
# HOST_CONFIG_DEPENDS is supported
#
# NOTE: PKG_CONFIG_DEPENDS cannot correctly track changes of string type config
# options, so you may want to do manual cleanup on config change.
#
PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_python3-packages-list-host \
	CONFIG_PACKAGE_python3-packages-list \
	CONFIG_PACKAGE_python3-packages-list-cleanup \
	CONFIG_PACKAGE_python3-packages-envs \
	CONFIG_PACKAGE_python3-packages-extra-deps \
	CONFIG_PACKAGE_python3-packages-index-url \
	CONFIG_PACKAGE_python3-packages-pip-opts \

PKG_BUILD_DEPENDS:=python3 python3/host

include $(INCLUDE_DIR)/package.mk
include ../python3-host.mk
include ../python3-package.mk

define Package/python3-packages
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=A dummy package for packaging python3 modules with pip
  DEPENDS:=@DEVEL +python3
endef

define Package/python3-packages/config
if PACKAGE_python3-packages
config PACKAGE_python3-packages-list-host
	string "List of python3 packages to install on host"
config PACKAGE_python3-packages-list
	string "List of python3 packages to install on target"
config PACKAGE_python3-packages-list-cleanup
	string "List of python3 packages to cleanup to avoid clash with existing packages"
config PACKAGE_python3-packages-envs
	string "Extra environment variables to pass on to pip and its children on target build"
config PACKAGE_python3-packages-extra-deps
	string "List of deps fulfilled but not tracked by the build system"
config PACKAGE_python3-packages-index-url
	string "Index URL passed to pip with --index-url"
config PACKAGE_python3-packages-pip-opts
	string "Additional arguments to pip command line"
endif
endef

CONFIG_PACKAGE_python3-packages-list-host:=$(call qstrip,$(CONFIG_PACKAGE_python3-packages-list-host))
CONFIG_PACKAGE_python3-packages-list:=$(call qstrip,$(CONFIG_PACKAGE_python3-packages-list))
CONFIG_PACKAGE_python3-packages-list-cleanup:=$(call qstrip,$(CONFIG_PACKAGE_python3-packages-list-cleanup))
CONFIG_PACKAGE_python3-packages-envs:=$(call qstrip,$(CONFIG_PACKAGE_python3-packages-envs))
CONFIG_PACKAGE_python3-packages-extra-deps:=$(call qstrip,$(CONFIG_PACKAGE_python3-packages-extra-deps))
CONFIG_PACKAGE_python3-packages-index-url:=$(call qstrip,$(CONFIG_PACKAGE_python3-packages-index-url))
CONFIG_PACKAGE_python3-packages-pip-opts:=$(call qstrip,$(CONFIG_PACKAGE_python3-packages-pip-opts))

decr=$(word $(1),0 1 2 3 4 5 6 7 8 9 10)
recur=$(if $(subst 0,,$(2)),$(call recur,$(1),$(call decr,$(2)),$(call $(1)$(2),$(3))),$(3))
_req2dir1=$(subst >,gt,$(1))
_req2dir2=$(subst <,lt,$(1))
_req2dir3=$(subst >=,geq,$(1))
_req2dir4=$(subst <=,leq,$(1))
_req2dir5=$(subst ://,:::,$(1))
_req2dir6=$(subst *,_,$(1))
_req2dir7=$(subst ?,_,$(1))
req2dir=$(call recur,_req2dir,7,$(1))

# --ignore-installed, it may happen that host pip will ignore target install if
#  it was already installed as host module, e.g. cffi deps of cryptograph
#
# --no-build-isolation, this is a new addition in pip3 and is needed when build
#  depends on host modules
HOST_PYTHON3_PIP_INSTALL=$(HOST_PYTHON3_PIP) install \
	--root=$(1) \
	--prefix=$(2) \
	--ignore-installed \
	--no-build-isolation \
	--no-compile \
	$(if $(CONFIG_PACKAGE_python3-packages-index-url), --index-url "$(CONFIG_PACKAGE_python3-packages-index-url)") \
	$(if $(CONFIG_PACKAGE_python3-packages-pip-opts), $(CONFIG_PACKAGE_python3-packages-pip-opts)) \

HOST_PYTHON3_PIP_INSTALL_HOST:=$(call HOST_PYTHON3_PIP_INSTALL,$(STAGING_DIR_HOSTPKG),"")
HOST_PYTHON3_PIP_INSTALL_TARGET=$(call HOST_PYTHON3_PIP_INSTALL,$(PKG_INSTALL_DIR)/$(call req2dir,$(pkg)),/usr)
HOST_PYTHON3_PIP_INSTALL_CLEANUP:=$(call HOST_PYTHON3_PIP_INSTALL,$(PKG_INSTALL_DIR)/_cleanup,/usr)

define Build/Compile
	$(foreach pkg,$(CONFIG_PACKAGE_python3-packages-list-host),
		$(call HostPython3/Run,,$(HOST_PYTHON3_PIP_INSTALL_HOST) $(pkg))
	)
	$(foreach pkg,$(CONFIG_PACKAGE_python3-packages-list),
		$(call Python3/Run,,$(call HOST_PYTHON3_PIP_INSTALL_TARGET,$(pkg)) $(pkg),$(CONFIG_PACKAGE_python3-packages-envs))
	)
	$(foreach pkg,$(CONFIG_PACKAGE_python3-packages-list-cleanup),
		$(call Python3/Run,,$(HOST_PYTHON3_PIP_INSTALL_CLEANUP) $(pkg),$(CONFIG_PACKAGE_python3-packages-envs))
	)
endef

define Package/python3-packages/install
	$(foreach pkg,$(CONFIG_PACKAGE_python3-packages-list),
		$(CP) "$(PKG_INSTALL_DIR)/$(call req2dir,$(pkg))"/* $(1)
	)

	find "$(PKG_INSTALL_DIR)/_cleanup" -mindepth 1 -depth | while read sf; do \
		tf="$$$${sf#$(PKG_INSTALL_DIR)/_cleanup/}"; \
		tf="$(1)/$$$$tf"; \
		if [ -f "$$$$tf" -o -L "$$$$tf" ]; then \
			rm -vf "$$$$tf"; \
		elif [ -d "$$$$tf" ]; then \
			rmdir -v -p "$$$$tf" || true; \
		fi \
	done
endef

define Package/python3-packages/extra_provides
	echo $(CONFIG_PACKAGE_python3-packages-extra-deps) | tr ' ' '\n'
endef

$(eval $(call BuildPackage,python3-packages))
