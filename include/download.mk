# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2012 OpenWrt.org
# Copyright (C) 2016 LEDE project

PROJECT_GIT = https://git.openwrt.org

OPENWRT_GIT = $(PROJECT_GIT)
LEDE_GIT = $(PROJECT_GIT)

ifdef PKG_SOURCE_VERSION
  ifndef PKG_VERSION
    PKG_VERSION := $(if $(PKG_SOURCE_DATE),$(PKG_SOURCE_DATE)-)$(call version_abbrev,$(PKG_SOURCE_VERSION))
  endif
  PKG_SOURCE_SUBDIR ?= $(PKG_NAME)-$(PKG_VERSION)
  PKG_SOURCE ?= $(PKG_SOURCE_SUBDIR).tar.xz
endif

DOWNLOAD_RDEP=$(STAMP_PREPARED) $(HOST_STAMP_PREPARED)

define dl_method_git
$(if $(filter https://github.com/% git://github.com/%,$(1)),github_archive,git)
endef

# Try to guess the download method from the URL
define dl_method
$(strip \
  $(if $(filter git,$(2)),$(call dl_method_git,$(1),$(2)),
    $(if $(2),$(2), \
      $(if $(filter @OPENWRT @APACHE/% @DEBIAN/% @GITHUB/% @GNOME/% @GNU/% @KERNEL/% @SF/% @SAVANNAH/% ftp://% http://% https://% file://%,$(1)),default, \
        $(if $(filter git://%,$(1)),$(call dl_method_git,$(1),$(2)), \
          $(if $(filter svn://%,$(1)),svn, \
            $(if $(filter cvs://%,$(1)),cvs, \
              $(if $(filter hg://%,$(1)),hg, \
                $(if $(filter sftp://%,$(1)),bzr, \
                  unknown \
                ) \
              ) \
            ) \
          ) \
        ) \
      ) \
    ) \
  ) \
)
endef

# code for creating tarballs from cvs/svn/git/bzr/hg/darcs checkouts - useful for mirror support
dl_pack/bz2=bzip2 -c > $(1)
dl_pack/gz=gzip -nc > $(1)
dl_pack/xz=xz -zc -7e > $(1)
dl_pack/zst=zstd -T0 --ultra -20 -c > $(1)
dl_pack/unknown=$(error ERROR: Unknown pack format for file $(1))
define dl_pack
	$(if $(dl_pack/$(call ext,$(1))),$(dl_pack/$(call ext,$(1))),$(dl_pack/unknown))
endef
define dl_tar_pack
	$(TAR) --numeric-owner --owner=0 --group=0 --mode=a-s --sort=name \
		$$$${TAR_TIMESTAMP:+--mtime="$$$$TAR_TIMESTAMP"} -c $(2) | $(call dl_pack,$(1))
endef

gen_sha256sum = $(shell $(MKHASH) sha256 $(DL_DIR)/$(1))

# Used in Build/CoreTargets and HostBuild/Core as an integrity check for
# downloaded files.  It will add a FORCE rule if the sha256 hash does not
# match, so that the download can be more thoroughly handled by download.pl.
define check_download_integrity
  expected_hash:=$(strip $(if $(filter-out x,$(HASH)),$(HASH),$(MIRROR_HASH)))
  $$(if $$(and $(FILE),$$(wildcard $(DL_DIR)/$(FILE)), \
	       $$(filter undefined,$$(flavor DownloadChecked/$(FILE)))), \
    $$(eval DownloadChecked/$(FILE):=1) \
    $$(if $$(filter-out $$(call gen_sha256sum,$(FILE)),$$(expected_hash)), \
      $(DL_DIR)/$(FILE): FORCE) \
  )
endef

ifdef CHECK
check_escape=$(subst ','\'',$(1))
#')

# $(1): suffix of the F_, C_ variables, e.g. hash_deprecated, hash_mismatch, etc.
# $(2): filename
# $(3): expected hash value
# $(4): var name of the the form: {PKG_,Download/<name>:}{,MIRROR_}{HASH,MIRROR_HASH}
check_warn_nofix = $(info $(shell printf "$(_R)WARNING: %s$(_N)" '$(call check_escape,$(call C_$(1),$(2),$(3),$(4)))'))
ifndef FIXUP
  check_warn = $(check_warn_nofix)
else
  check_warn = $(if $(filter-out undefined,$(origin F_$(1))),$(filter ,$(shell $(call F_$(1),$(2),$(3),$(4)) >&2)),$(check_warn_nofix))
endif

ifdef FIXUP
F_hash_deprecated = $(SCRIPT_DIR)/fixup-makefile.pl $(CURDIR)/Makefile fix-hash $(3) $(call gen_sha256sum,$(1)) $(2)
F_hash_mismatch = $(F_hash_deprecated)
F_hash_missing = $(SCRIPT_DIR)/fixup-makefile.pl $(CURDIR)/Makefile add-hash $(3) $(call gen_sha256sum,$(1))
endif

# $(1): filename
# $(2): expected hash value
# $(3): var name of the the form: {PKG_,Download/<name>:}{,MIRROR_}{HASH,MIRROR_HASH}
C_download_missing = $(1) is missing, please run make download before re-running this check
C_hash_mismatch = $(3) does not match $(1) hash $(call gen_sha256sum,$(1))
C_hash_deprecated = $(3) uses deprecated hash, set to $(call gen_sha256sum,$(1))
C_hash_missing = $(3) is missing, set to $(call gen_sha256sum,$(1))

# $(1): filename
# $(2): expected hash value
# $(3): var name of the the form: {PKG_,Download/<name>:}{,MIRROR_}{HASH,MIRROR_HASH}
check_hash = \
  $(if $(wildcard $(DL_DIR)/$(1)), \
    $(if $(filter-out x,$(2)), \
      $(if $(filter 64,$(shell printf '%s' '$(2)' | wc -c)), \
        $(if $(filter $(2),$(call gen_sha256sum,$(1))),, \
          $(call check_warn,hash_mismatch,$(1),$(2),$(3)) \
        ), \
        $(call check_warn,hash_deprecated,$(1),$(2),$(3)), \
      ), \
      $(call check_warn,hash_missing,$(1),$(2),$(3)) \
    ), \
    $(call check_warn,download_missing,$(1),$(2),$(3)) \
  )

ifdef FIXUP
F_md5_deprecated = $(SCRIPT_DIR)/fixup-makefile.pl $(CURDIR)/Makefile rename-var $(2) $(3)
endif

C_md5_deprecated = Use of $(2) is deprecated, switch to $(3)

check_md5 = \
  $(if $(filter-out x,$(1)), \
    $(call check_warn,md5_deprecated,$(1),$(2),$(3)) \
  )

hash_var = $(if $(filter-out x,$(1)),MD5SUM,HASH)
endif

define DownloadMethod/unknown
	echo "ERROR: No download method available"; false
endef

define DownloadMethod/default
	$(SCRIPT_DIR)/download.pl "$(DL_DIR)" "$(FILE)" "$(HASH)" "$(URL_FILE)" $(foreach url,$(URL),"$(url)") \
	$(if $(filter check,$(1)), \
		$(call check_hash,$(FILE),$(HASH),$(2)$(call hash_var,$(MD5SUM))) \
		$(call check_md5,$(MD5SUM),$(2)MD5SUM,$(2)HASH) \
	)
endef

# $(1): "check"
# $(2): "PKG_" if <name> as in Download/<name> is "default", otherwise "Download/<name>:"
# $(3): shell command sequence to do the download
define wrap_mirror
$(if $(if $(MIRROR),$(filter-out x,$(MIRROR_HASH))),$(SCRIPT_DIR)/download.pl "$(DL_DIR)" "$(FILE)" "$(MIRROR_HASH)" "" || ( $(3) ),$(3)) \
$(if $(filter check,$(1)), \
	$(call check_hash,$(FILE),$(MIRROR_HASH),$(2)MIRROR_$(call hash_var,$(MIRROR_MD5SUM))) \
	$(call check_md5,$(MIRROR_MD5SUM),$(2)MIRROR_MD5SUM,$(2)MIRROR_HASH) \
)
endef

define DownloadMethod/cvs
	$(call wrap_mirror,$(1),$(2), \
		echo "Checking out files from the cvs repository..."; \
		mkdir -p $(TMP_DIR)/dl && \
		cd $(TMP_DIR)/dl && \
		rm -rf $(SUBDIR) && \
		[ \! -d $(SUBDIR) ] && \
		cvs -d $(URL) export $(VERSION) $(SUBDIR) && \
		echo "Packing checkout..." && \
		$(call dl_tar_pack,$(TMP_DIR)/dl/$(FILE),$(SUBDIR)) && \
		mv $(TMP_DIR)/dl/$(FILE) $(DL_DIR)/ && \
		rm -rf $(SUBDIR); \
	)
endef

define DownloadMethod/svn
	$(call wrap_mirror,$(1),$(2), \
		echo "Checking out files from the svn repository..."; \
		mkdir -p $(TMP_DIR)/dl && \
		cd $(TMP_DIR)/dl && \
		rm -rf $(SUBDIR) && \
		[ \! -d $(SUBDIR) ] && \
		( svn help export | grep -q trust-server-cert && \
		svn export --non-interactive --trust-server-cert -r$(VERSION) $(URL) $(SUBDIR) || \
		svn export --non-interactive -r$(VERSION) $(URL) $(SUBDIR) ) && \
		echo "Packing checkout..." && \
		export TAR_TIMESTAMP="" && \
		$(call dl_tar_pack,$(TMP_DIR)/dl/$(FILE),$(SUBDIR)) && \
		mv $(TMP_DIR)/dl/$(FILE) $(DL_DIR)/ && \
		rm -rf $(SUBDIR); \
	)
endef

define DownloadMethod/git
	$(call wrap_mirror,$(1),$(2), \
		$(call DownloadMethod/rawgit) \
	)
endef

define DownloadMethod/github_archive
	$(call wrap_mirror,$(1),$(2), \
		$(SCRIPT_DIR)/dl_github_archive.py \
			--dl-dir="$(DL_DIR)" \
			--url="$(URL)" \
			--version="$(VERSION)" \
			--subdir="$(SUBDIR)" \
			--source="$(FILE)" \
			--hash="$(MIRROR_HASH)" \
		|| ( $(call DownloadMethod/rawgit) ); \
	)
endef

# Only intends to be called as a submethod from other DownloadMethod
define DownloadMethod/rawgit
	echo "Checking out files from the git repository..."; \
	mkdir -p $(TMP_DIR)/dl && \
	cd $(TMP_DIR)/dl && \
	rm -rf $(SUBDIR) && \
	[ \! -d $(SUBDIR) ] && \
	git clone $(OPTS) $(URL) $(SUBDIR) && \
	(cd $(SUBDIR) && git checkout $(VERSION) && \
	git submodule update --init --recursive) && \
	echo "Packing checkout..." && \
	export TAR_TIMESTAMP=`cd $(SUBDIR) && git log -1 --format='@%ct'` && \
	rm -rf $(SUBDIR)/.git && \
	$(call dl_tar_pack,$(TMP_DIR)/dl/$(FILE),$(SUBDIR)) && \
	mv $(TMP_DIR)/dl/$(FILE) $(DL_DIR)/ && \
	rm -rf $(SUBDIR);
endef

define DownloadMethod/bzr
	$(call wrap_mirror,$(1),$(2), \
		echo "Checking out files from the bzr repository..."; \
		mkdir -p $(TMP_DIR)/dl && \
		cd $(TMP_DIR)/dl && \
		rm -rf $(SUBDIR) && \
		[ \! -d $(SUBDIR) ] && \
		bzr export --per-file-timestamps -r$(VERSION) $(SUBDIR) $(URL) && \
		echo "Packing checkout..." && \
		export TAR_TIMESTAMP="" && \
		$(call dl_tar_pack,$(TMP_DIR)/dl/$(FILE),$(SUBDIR)) && \
		mv $(TMP_DIR)/dl/$(FILE) $(DL_DIR)/ && \
		rm -rf $(SUBDIR); \
	)
endef

define DownloadMethod/hg
	$(call wrap_mirror,$(1),$(2), \
		echo "Checking out files from the hg repository..."; \
		mkdir -p $(TMP_DIR)/dl && \
		cd $(TMP_DIR)/dl && \
		rm -rf $(SUBDIR) && \
		[ \! -d $(SUBDIR) ] && \
		hg clone -r $(VERSION) $(URL) $(SUBDIR) && \
		export TAR_TIMESTAMP=`cd $(SUBDIR) && hg log --template '@{date}' -l 1` && \
		find $(SUBDIR) -name .hg | xargs rm -rf && \
		echo "Packing checkout..." && \
		$(call dl_tar_pack,$(TMP_DIR)/dl/$(FILE),$(SUBDIR)) && \
		mv $(TMP_DIR)/dl/$(FILE) $(DL_DIR)/ && \
		rm -rf $(SUBDIR); \
	)
endef

define DownloadMethod/darcs
	$(call wrap_mirror, $(1), $(2), \
		echo "Checking out files from the darcs repository..."; \
		mkdir -p $(TMP_DIR)/dl && \
		cd $(TMP_DIR)/dl && \
		rm -rf $(SUBDIR) && \
		[ \! -d $(SUBDIR) ] && \
		darcs get -t $(VERSION) $(URL) $(SUBDIR) && \
		export TAR_TIMESTAMP=`cd $(SUBDIR) && LC_ALL=C darcs log --last 1 | sed -ne 's!^Date: \+!!p'` && \
		find $(SUBDIR) -name _darcs | xargs rm -rf && \
		echo "Packing checkout..." && \
		$(call dl_tar_pack,$(TMP_DIR)/dl/$(FILE),$(SUBDIR)) && \
		mv $(TMP_DIR)/dl/$(FILE) $(DL_DIR)/ && \
		rm -rf $(SUBDIR); \
	)
endef

Validate/cvs=VERSION SUBDIR
Validate/svn=VERSION SUBDIR
Validate/git=VERSION SUBDIR
Validate/bzr=VERSION SUBDIR
Validate/hg=VERSION SUBDIR
Validate/darcs=VERSION SUBDIR

define Download/Defaults
  URL:=
  FILE:=
  URL_FILE:=
  PROTO:=
  HASH=$$(MD5SUM)
  MD5SUM:=x
  SUBDIR:=
  MIRROR:=1
  MIRROR_HASH=$$(MIRROR_MD5SUM)
  MIRROR_MD5SUM:=x
  VERSION:=
  OPTS:=
endef

define Download/default
  FILE:=$(PKG_SOURCE)
  URL:=$(PKG_SOURCE_URL)
  URL_FILE:=$(PKG_SOURCE_URL_FILE)
  SUBDIR:=$(PKG_SOURCE_SUBDIR)
  PROTO:=$(PKG_SOURCE_PROTO)
  $(if $(PKG_SOURCE_MIRROR),MIRROR:=$(filter 1,$(PKG_MIRROR)))
  $(if $(PKG_MIRROR_MD5SUM),MIRROR_MD5SUM:=$(PKG_MIRROR_MD5SUM))
  $(if $(PKG_MIRROR_HASH),MIRROR_HASH:=$(PKG_MIRROR_HASH))
  VERSION:=$(PKG_SOURCE_VERSION)
  $(if $(PKG_MD5SUM),MD5SUM:=$(PKG_MD5SUM))
  $(if $(PKG_HASH),HASH:=$(PKG_HASH))
endef

define Download
  $(eval $(Download/Defaults))
  $(eval $(Download/$(1)))
  $(foreach FIELD,URL FILE $(Validate/$(call dl_method,$(URL),$(PROTO))),
    ifeq ($($(FIELD)),)
      $$(error Download/$(1) is missing the $(FIELD) field.)
    endif
  )

  $(foreach dep,$(DOWNLOAD_RDEP),
    $(dep): $(DL_DIR)/$(FILE)
  )
  download: $(DL_DIR)/$(FILE)

  $(DL_DIR)/$(FILE):
	mkdir -p $(DL_DIR)
	$(call locked, \
		$(if $(DownloadMethod/$(call dl_method,$(URL),$(PROTO))), \
			$(call DownloadMethod/$(call dl_method,$(URL),$(PROTO)),check,$(if $(filter default,$(1)),PKG_,Download/$(1):)), \
			$(DownloadMethod/unknown) \
		),\
		$(FILE))

endef
