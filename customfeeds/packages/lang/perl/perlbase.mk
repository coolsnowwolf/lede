define Package/perlbase-template
SUBMENU:=Perl
SECTION:=lang
CATEGORY:=Languages
URL:=http://www.cpan.org/
DEPENDS:=perl
endef

define Package/perlbase-anydbm-file
$(call Package/perlbase-template)
TITLE:=AnyDBM_File perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-anydbm-file/install
$(call perlmod/Install,$(1),AnyDBM_File.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/AnyDBM_File.t)
endef

$(eval $(call BuildPackage,perlbase-anydbm-file))


define Package/perlbase-app
$(call Package/perlbase-template)
TITLE:=app perl module
DEPENDS+=+perlbase-autouse +perlbase-base +perlbase-config +perlbase-cpan +perlbase-essential +perlbase-file +perlbase-getopt +perlbase-if +perlbase-tap +perlbase-text
endef

define Package/perlbase-app/install
$(call perlmod/Install,$(1),App,)
endef

$(eval $(call BuildPackage,perlbase-app))


define Package/perlbase-archive
$(call Package/perlbase-template)
TITLE:=Archive perl module
DEPENDS+=+perlbase-cwd +perlbase-essential +perlbase-file +perlbase-io
endef

define Package/perlbase-archive/install
$(call perlmod/Install,$(1),Archive,)
$(call perlmod/InstallBaseTests,$(1),cpan/Archive-Tar/bin cpan/Archive-Tar/t)
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/ptar $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/ptardiff $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/ptargrep $(1)/usr/bin/
endef

$(eval $(call BuildPackage,perlbase-archive))


define Package/perlbase-arybase
$(call Package/perlbase-template)
TITLE:=arybase perl module
DEPENDS+=+perlbase-xsloader
endef

define Package/perlbase-arybase/install
$(call perlmod/Install,$(1),arybase.pm auto/arybase,)
$(call perlmod/InstallBaseTests,$(1),ext/arybase/t)
endef

$(eval $(call BuildPackage,perlbase-arybase))


define Package/perlbase-attribute
$(call Package/perlbase-template)
TITLE:=Attribute perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-attribute/install
$(call perlmod/Install,$(1),Attribute,)
$(call perlmod/InstallBaseTests,$(1),dist/Attribute-Handlers/t)
endef

$(eval $(call BuildPackage,perlbase-attribute))


define Package/perlbase-attributes
$(call Package/perlbase-template)
TITLE:=attributes perl module
DEPENDS+=+perlbase-essential +perlbase-xsloader
endef

define Package/perlbase-attributes/install
$(call perlmod/Install,$(1),attributes.pm auto/attributes,)
endef

$(eval $(call BuildPackage,perlbase-attributes))


define Package/perlbase-autodie
$(call Package/perlbase-template)
TITLE:=autodie perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-autodie/install
$(call perlmod/Install,$(1),autodie autodie.pm,)
$(call perlmod/InstallBaseTests,$(1),cpan/autodie/t)
endef

$(eval $(call BuildPackage,perlbase-autodie))


define Package/perlbase-autoloader
$(call Package/perlbase-template)
TITLE:=AutoLoader perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-autoloader/install
$(call perlmod/Install,$(1),AutoLoader.pm,)
$(call perlmod/InstallBaseTests,$(1),cpan/AutoLoader/t)
endef

$(eval $(call BuildPackage,perlbase-autoloader))


define Package/perlbase-autosplit
$(call Package/perlbase-template)
TITLE:=AutoSplit perl module
DEPENDS+=+perlbase-config +perlbase-essential +perlbase-file
endef

define Package/perlbase-autosplit/install
$(call perlmod/Install/NoStrip,$(1),AutoSplit.pm,)
endef

$(eval $(call BuildPackage,perlbase-autosplit))


define Package/perlbase-autouse
$(call Package/perlbase-template)
TITLE:=autouse perl module
endef

define Package/perlbase-autouse/install
$(call perlmod/Install,$(1),autouse.pm,)
$(call perlmod/InstallBaseTests,$(1),dist/autouse/t)
endef

$(eval $(call BuildPackage,perlbase-autouse))


define Package/perlbase-b
$(call Package/perlbase-template)
TITLE:=B perl module
DEPENDS+=+perlbase-config +perlbase-essential +perlbase-feature
endef

define Package/perlbase-b/install
$(call perlmod/Install,$(1),B B.pm auto/B,)
$(call perlmod/InstallBaseTests,$(1),cpan/B-Debug/t ext/B/t lib/B/Deparse-core.t lib/B/Deparse-subclass.t lib/B/Deparse.t)
endef

$(eval $(call BuildPackage,perlbase-b))


define Package/perlbase-base
$(call Package/perlbase-template)
TITLE:=base perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-base/install
$(call perlmod/Install,$(1),base.pm,)
$(call perlmod/InstallBaseTests,$(1),dist/base/t)
endef

$(eval $(call BuildPackage,perlbase-base))


define Package/perlbase-benchmark
$(call Package/perlbase-template)
TITLE:=Benchmark perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-benchmark/install
$(call perlmod/Install,$(1),Benchmark.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/Benchmark.t)
endef

$(eval $(call BuildPackage,perlbase-benchmark))


define Package/perlbase-bigint
$(call Package/perlbase-template)
TITLE:=bigint perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-bigint/install
$(call perlmod/Install,$(1),bigint.pm,)
endef

$(eval $(call BuildPackage,perlbase-bigint))


define Package/perlbase-bignum
$(call Package/perlbase-template)
TITLE:=bignum perl module
DEPENDS+=+perlbase-bigint +perlbase-essential
endef

define Package/perlbase-bignum/install
$(call perlmod/Install,$(1),bignum.pm bigrat.pm,)
$(call perlmod/InstallBaseTests,$(1),dist/bignum/t)
endef

$(eval $(call BuildPackage,perlbase-bignum))


define Package/perlbase-blib
$(call Package/perlbase-template)
TITLE:=blib perl module
DEPENDS+=+perlbase-cwd +perlbase-essential +perlbase-file
endef

define Package/perlbase-blib/install
$(call perlmod/Install,$(1),blib.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/blib.t)
endef

$(eval $(call BuildPackage,perlbase-blib))


define Package/perlbase-bytes
$(call Package/perlbase-template)
TITLE:=bytes perl module
endef

define Package/perlbase-bytes/install
$(call perlmod/Install,$(1),bytes.pm bytes_heavy.pl,)
$(call perlmod/InstallBaseTests,$(1),lib/bytes.t)
endef

$(eval $(call BuildPackage,perlbase-bytes))


define Package/perlbase-charnames
$(call Package/perlbase-template)
TITLE:=charnames perl module
DEPENDS+=+perlbase-bytes +perlbase-essential +perlbase-re +perlbase-unicore
endef

define Package/perlbase-charnames/install
$(call perlmod/Install,$(1),_charnames.pm charnames.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/charnames.t)
endef

$(eval $(call BuildPackage,perlbase-charnames))


define Package/perlbase-class
$(call Package/perlbase-template)
TITLE:=Class perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-class/install
$(call perlmod/Install,$(1),Class,)
$(call perlmod/InstallBaseTests,$(1),lib/Class/Struct.t)
endef

$(eval $(call BuildPackage,perlbase-class))


define Package/perlbase-compress
$(call Package/perlbase-template)
TITLE:=Compress perl module
DEPENDS+=+perlbase-bytes +perlbase-essential +perlbase-io +perlbase-scalar
endef

define Package/perlbase-compress/install
$(call perlmod/Install,$(1),Compress auto/Compress,)
$(call perlmod/InstallBaseTests,$(1),cpan/Compress-Raw-Bzip2/t cpan/Compress-Raw-Zlib/t)
endef

$(eval $(call BuildPackage,perlbase-compress))


define Package/perlbase-config
$(call Package/perlbase-template)
TITLE:=Config perl module
endef

define Package/perlbase-config/install
$(call perlmod/Install,$(1),Config Config.pm Config_git.pl Config_heavy.pl,)
$(call perlmod/InstallBaseTests,$(1),cpan/Config-Perl-V/t lib/Config.t lib/Config/Extensions.t)
endef

$(eval $(call BuildPackage,perlbase-config))


define Package/perlbase-cpan
$(call Package/perlbase-template)
TITLE:=CPAN perl module
DEPENDS+=+perlbase-b +perlbase-config +perlbase-cwd +perlbase-dirhandle +perlbase-essential +perlbase-extutils +perlbase-fcntl +perlbase-file +perlbase-filehandle +perlbase-http-tiny +perlbase-net +perlbase-safe +perlbase-scalar +perlbase-sys +perlbase-text
endef

define Package/perlbase-cpan/install
$(call perlmod/Install,$(1),CPAN CPAN.pm Parse/CPAN/Meta.pm,CPAN/FirstTime.pm)
$(call perlmod/Install/NoStrip,$(1),CPAN/FirstTime.pm,)
$(call perlmod/InstallBaseTests,$(1),cpan/CPAN-Meta-Requirements/t cpan/CPAN-Meta-YAML/t cpan/CPAN-Meta/t cpan/CPAN/t cpan/Parse-CPAN-Meta/t)
endef

$(eval $(call BuildPackage,perlbase-cpan))


define Package/perlbase-cwd
$(call Package/perlbase-template)
TITLE:=Cwd perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-cwd/install
$(call perlmod/Install,$(1),Cwd.pm auto/Cwd,)
endef

$(eval $(call BuildPackage,perlbase-cwd))


define Package/perlbase-data
$(call Package/perlbase-template)
TITLE:=Data perl module
DEPENDS+=+perlbase-bytes +perlbase-essential
endef

define Package/perlbase-data/install
$(call perlmod/Install,$(1),Data auto/Data,)
$(call perlmod/InstallBaseTests,$(1),dist/Data-Dumper/t)
endef

$(eval $(call BuildPackage,perlbase-data))


define Package/perlbase-db
$(call Package/perlbase-template)
TITLE:=DB perl module
DEPENDS+=+libdb47 +perlbase-essential +perlbase-meta-notation
endef

define Package/perlbase-db/install
$(call perlmod/Install,$(1),DB.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/DB.t)
endef

$(eval $(call BuildPackage,perlbase-db))


define Package/perlbase-db-file
$(call Package/perlbase-template)
TITLE:=DB_File perl module
DEPENDS+=+libdb47 +perlbase-essential +perlbase-tie
endef

define Package/perlbase-db-file/install
$(call perlmod/Install,$(1),DB_File.pm auto/DB_File,)
$(call perlmod/InstallBaseTests,$(1),cpan/DB_File/t)
endef

$(eval $(call BuildPackage,perlbase-db-file))


define Package/perlbase-dbm-filter
$(call Package/perlbase-template)
TITLE:=DBM_Filter perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-dbm-filter/install
$(call perlmod/Install,$(1),DBM_Filter DBM_Filter.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/DBM_Filter/t lib/dbm_filter_util.pl)
endef

$(eval $(call BuildPackage,perlbase-dbm-filter))


define Package/perlbase-devel
$(call Package/perlbase-template)
TITLE:=Devel perl module
DEPENDS+=+perlbase-essential +perlbase-file +perlbase-selfloader +perlbase-xsloader
endef

define Package/perlbase-devel/install
$(call perlmod/Install,$(1),Devel auto/Devel,Devel/PPPort.pm)
$(call perlmod/Install/NoStrip,$(1),Devel/PPPort.pm,)
$(call perlmod/InstallBaseTests,$(1),cpan/Devel-PPPort/t dist/Devel-SelfStubber/t ext/Devel-Peek/t)
endef

$(eval $(call BuildPackage,perlbase-devel))


define Package/perlbase-diagnostics
$(call Package/perlbase-template)
TITLE:=diagnostics perl module
DEPENDS+=+perlbase-config +perlbase-essential +perlbase-text
endef

define Package/perlbase-diagnostics/install
$(call perlmod/Install,$(1),diagnostics.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/diagnostics.t pod/perldiag.pod)
endef

$(eval $(call BuildPackage,perlbase-diagnostics))


define Package/perlbase-digest
$(call Package/perlbase-template)
TITLE:=Digest perl module
DEPENDS+=+perlbase-dynaloader +perlbase-essential +perlbase-fcntl +perlbase-integer
endef

define Package/perlbase-digest/install
$(call perlmod/Install,$(1),Digest Digest.pm auto/Digest,)
$(call perlmod/InstallBaseTests,$(1),cpan/Digest-MD5/MD5.xs cpan/Digest-MD5/t cpan/Digest-SHA/t cpan/Digest/t)
	$(INSTALL_DIR) $(1)/$(PERL_TESTSDIR)/cpan/Digest-SHA/src
endef

$(eval $(call BuildPackage,perlbase-digest))


define Package/perlbase-dirhandle
$(call Package/perlbase-template)
TITLE:=DirHandle perl module
DEPENDS+=+perlbase-essential +perlbase-symbol
endef

define Package/perlbase-dirhandle/install
$(call perlmod/Install,$(1),DirHandle.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/DirHandle.t)
endef

$(eval $(call BuildPackage,perlbase-dirhandle))


define Package/perlbase-dumpvalue
$(call Package/perlbase-template)
TITLE:=Dumpvalue perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-dumpvalue/install
$(call perlmod/Install,$(1),Dumpvalue.pm,)
$(call perlmod/InstallBaseTests,$(1),dist/Dumpvalue/t)
endef

$(eval $(call BuildPackage,perlbase-dumpvalue))


define Package/perlbase-dumpvar
$(call Package/perlbase-template)
TITLE:=dumpvar perl module
endef

define Package/perlbase-dumpvar/install
$(call perlmod/Install,$(1),dumpvar.pl,)
$(call perlmod/InstallBaseTests,$(1),lib/dumpvar.t)
endef

$(eval $(call BuildPackage,perlbase-dumpvar))


define Package/perlbase-dynaloader
$(call Package/perlbase-template)
TITLE:=DynaLoader perl module
DEPENDS+=+perlbase-config
endef

define Package/perlbase-dynaloader/install
$(call perlmod/Install,$(1),DynaLoader.pm,)
$(call perlmod/InstallBaseTests,$(1),ext/DynaLoader/t)
endef

$(eval $(call BuildPackage,perlbase-dynaloader))


define Package/perlbase-encode
$(call Package/perlbase-template)
TITLE:=Encode perl module
DEPENDS+=+perlbase-essential +perlbase-mime +perlbase-utf8 +perlbase-xsloader
endef

define Package/perlbase-encode/install
$(call perlmod/Install,$(1),Encode Encode.pm auto/Encode,Encode/PerlIO.pod Encode/Supported.pod)
$(call perlmod/InstallBaseTests,$(1),cpan/Encode/t)
endef

$(eval $(call BuildPackage,perlbase-encode))


define Package/perlbase-encoding
$(call Package/perlbase-template)
TITLE:=encoding perl module
DEPENDS+=+perlbase-encode +perlbase-essential
endef

define Package/perlbase-encoding/install
$(call perlmod/Install,$(1),encoding encoding.pm,)
$(call perlmod/InstallBaseTests,$(1),cpan/encoding-warnings/t)
endef

$(eval $(call BuildPackage,perlbase-encoding))


define Package/perlbase-english
$(call Package/perlbase-template)
TITLE:=English perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-english/install
$(call perlmod/Install,$(1),English.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/English.t)
endef

$(eval $(call BuildPackage,perlbase-english))


define Package/perlbase-env
$(call Package/perlbase-template)
TITLE:=Env perl module
DEPENDS+=+perlbase-config +perlbase-tie
endef

define Package/perlbase-env/install
$(call perlmod/Install,$(1),Env.pm,)
$(call perlmod/InstallBaseTests,$(1),dist/Env/t)
endef

$(eval $(call BuildPackage,perlbase-env))


define Package/perlbase-errno
$(call Package/perlbase-template)
TITLE:=Errno perl module
DEPENDS+=+perlbase-config +perlbase-essential
endef

define Package/perlbase-errno/install
$(call perlmod/Install,$(1),Errno.pm,)
$(call perlmod/InstallBaseTests,$(1),ext/Errno/t)
endef

$(eval $(call BuildPackage,perlbase-errno))


define Package/perlbase-essential
$(call Package/perlbase-template)
TITLE:=essential perl module
DEPENDS+=+perlbase-config
endef

define Package/perlbase-essential/install
$(call perlmod/Install,$(1),Carp Carp.pm Exporter Exporter.pm constant.pm deprecate.pm lib.pm locale.pm overload.pm overloading.pm parent.pm strict.pm subs.pm vars.pm warnings warnings.pm,)
$(call perlmod/Install/NoStrip,$(1),overload/numbers.pm,)
$(call perlmod/InstallBaseTests,$(1),cpan/parent/t dist/Carp/t dist/Exporter/t dist/constant/t dist/lib/t lib/locale.t lib/overload.t lib/overload64.t lib/overloading.t lib/strict.t lib/subs.t lib/vars.t lib/vars_carp.t lib/warnings.t)
endef

$(eval $(call BuildPackage,perlbase-essential))


define Package/perlbase-experimental
$(call Package/perlbase-template)
TITLE:=experimental perl module
DEPENDS+=+perlbase-essential +perlbase-feature +perlbase-version
endef

define Package/perlbase-experimental/install
$(call perlmod/Install,$(1),experimental.pm,)
$(call perlmod/InstallBaseTests,$(1),cpan/experimental/t)
endef

$(eval $(call BuildPackage,perlbase-experimental))


define Package/perlbase-extutils
$(call Package/perlbase-template)
TITLE:=ExtUtils perl module
DEPENDS+=+perlbase-autosplit +perlbase-base +perlbase-config +perlbase-cwd +perlbase-dirhandle +perlbase-encode +perlbase-essential +perlbase-file +perlbase-io +perlbase-ipc +perlbase-ostype +perlbase-symbol +perlbase-text
endef

define Package/perlbase-extutils/install
$(call perlmod/Install,$(1),ExtUtils,ExtUtils/MakeMaker/FAQ.pod ExtUtils/MakeMaker/Tutorial.pod ExtUtils/ParseXS.pm ExtUtils/ParseXS/Utilities.pm)
$(call perlmod/Install/NoStrip,$(1),ExtUtils/ParseXS.pm ExtUtils/ParseXS/Utilities.pm,)
$(call perlmod/InstallBaseTests,$(1),cpan/ExtUtils-Command/t cpan/ExtUtils-Constant/t cpan/ExtUtils-Install/t cpan/ExtUtils-MakeMaker/t cpan/ExtUtils-Manifest/t dist/ExtUtils-CBuilder/t dist/ExtUtils-ParseXS/t lib/ExtUtils/t lib/h2ph.t lib/h2xs.t utils/h2ph utils/h2xs)
endef

$(eval $(call BuildPackage,perlbase-extutils))


define Package/perlbase-fatal
$(call Package/perlbase-template)
TITLE:=Fatal perl module
DEPENDS+=+perlbase-autodie +perlbase-config +perlbase-essential +perlbase-scalar +perlbase-tie
endef

define Package/perlbase-fatal/install
$(call perlmod/Install,$(1),Fatal.pm,)
endef

$(eval $(call BuildPackage,perlbase-fatal))


define Package/perlbase-fcntl
$(call Package/perlbase-template)
TITLE:=Fcntl perl module
DEPENDS+=+perlbase-essential +perlbase-xsloader
endef

define Package/perlbase-fcntl/install
$(call perlmod/Install,$(1),Fcntl.pm auto/Fcntl,)
$(call perlmod/InstallBaseTests,$(1),ext/Fcntl/t)
endef

$(eval $(call BuildPackage,perlbase-fcntl))


define Package/perlbase-feature
$(call Package/perlbase-template)
TITLE:=feature perl module
endef

define Package/perlbase-feature/install
$(call perlmod/Install,$(1),feature.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/feature.t lib/feature/unicode_strings.t)
endef

$(eval $(call BuildPackage,perlbase-feature))


define Package/perlbase-fields
$(call Package/perlbase-template)
TITLE:=fields perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-fields/install
$(call perlmod/Install,$(1),fields.pm,)
endef

$(eval $(call BuildPackage,perlbase-fields))


define Package/perlbase-file
$(call Package/perlbase-template)
TITLE:=File perl module
DEPENDS+=+perlbase-class +perlbase-config +perlbase-cwd +perlbase-errno +perlbase-essential +perlbase-fcntl +perlbase-filehandle +perlbase-io +perlbase-locale +perlbase-params +perlbase-scalar +perlbase-symbol +perlbase-xsloader
endef

define Package/perlbase-file/install
$(call perlmod/Install,$(1),File auto/File,File/Find.pm)
$(call perlmod/Install/NoStrip,$(1),File/Find.pm,)
$(call perlmod/InstallBaseTests,$(1),cpan/File-Fetch/t cpan/File-Path/t cpan/File-Temp/t dist/PathTools/t ext/File-DosGlob/t ext/File-Find/t ext/File-Glob/t lib/File/Basename.t lib/File/Compare.t lib/File/Copy.t lib/File/stat-7896.t lib/File/stat.t)
endef

$(eval $(call BuildPackage,perlbase-file))


define Package/perlbase-filecache
$(call Package/perlbase-template)
TITLE:=FileCache perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-filecache/install
$(call perlmod/Install,$(1),FileCache.pm,)
$(call perlmod/InstallBaseTests,$(1),ext/FileCache/t)
endef

$(eval $(call BuildPackage,perlbase-filecache))


define Package/perlbase-filehandle
$(call Package/perlbase-template)
TITLE:=FileHandle perl module
DEPENDS+=+perlbase-essential +perlbase-io
endef

define Package/perlbase-filehandle/install
$(call perlmod/Install,$(1),FileHandle.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/FileHandle.t)
endef

$(eval $(call BuildPackage,perlbase-filehandle))


define Package/perlbase-filetest
$(call Package/perlbase-template)
TITLE:=filetest perl module
endef

define Package/perlbase-filetest/install
$(call perlmod/Install,$(1),filetest.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/filetest.t)
endef

$(eval $(call BuildPackage,perlbase-filetest))


define Package/perlbase-filter
$(call Package/perlbase-template)
TITLE:=Filter perl module
DEPENDS+=+perlbase-dynaloader +perlbase-essential +perlbase-text
endef

define Package/perlbase-filter/install
$(call perlmod/Install,$(1),Filter auto/Filter,)
$(call perlmod/InstallBaseTests,$(1),cpan/Filter-Util-Call/filter-util.pl cpan/Filter-Util-Call/t dist/Filter-Simple/t)
endef

$(eval $(call BuildPackage,perlbase-filter))


define Package/perlbase-findbin
$(call Package/perlbase-template)
TITLE:=FindBin perl module
DEPENDS+=+perlbase-cwd +perlbase-essential +perlbase-file
endef

define Package/perlbase-findbin/install
$(call perlmod/Install,$(1),FindBin.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/FindBin.t)
endef

$(eval $(call BuildPackage,perlbase-findbin))


define Package/perlbase-gdbm-file
$(call Package/perlbase-template)
TITLE:=GDBM_File perl module
DEPENDS+=+libgdbm +perlbase-essential +perlbase-tie +perlbase-xsloader
endef

define Package/perlbase-gdbm-file/install
$(call perlmod/Install,$(1),GDBM_File.pm auto/GDBM_File,)
$(call perlmod/InstallBaseTests,$(1),ext/GDBM_File/t)
endef

$(eval $(call BuildPackage,perlbase-gdbm-file))


define Package/perlbase-getopt
$(call Package/perlbase-template)
TITLE:=Getopt perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-getopt/install
$(call perlmod/Install,$(1),Getopt,)
$(call perlmod/InstallBaseTests,$(1),cpan/Getopt-Long/t lib/Getopt/Std.t)
endef

$(eval $(call BuildPackage,perlbase-getopt))


define Package/perlbase-hash
$(call Package/perlbase-template)
TITLE:=Hash perl module
DEPENDS+=+perlbase-essential +perlbase-scalar +perlbase-xsloader
endef

define Package/perlbase-hash/install
$(call perlmod/Install,$(1),Hash auto/Hash,)
$(call perlmod/InstallBaseTests,$(1),ext/Hash-Util-FieldHash/t ext/Hash-Util/t)
endef

$(eval $(call BuildPackage,perlbase-hash))


define Package/perlbase-http-tiny
$(call Package/perlbase-template)
TITLE:=http-tiny perl module
DEPENDS+=+perlbase-errno +perlbase-essential +perlbase-io
endef

define Package/perlbase-http-tiny/install
$(call perlmod/Install,$(1),HTTP/Tiny.pm,)
$(call perlmod/InstallBaseTests,$(1),cpan/HTTP-Tiny/corpus cpan/HTTP-Tiny/t)
endef

$(eval $(call BuildPackage,perlbase-http-tiny))


define Package/perlbase-i18n
$(call Package/perlbase-template)
TITLE:=I18N perl module
DEPENDS+=+perlbase-essential +perlbase-posix +perlbase-xsloader
endef

define Package/perlbase-i18n/install
$(call perlmod/Install,$(1),I18N auto/I18N,I18N/LangTags/List.pm)
$(call perlmod/InstallBaseTests,$(1),dist/I18N-Collate/t dist/I18N-LangTags/t ext/I18N-Langinfo/t)
$(call perlmod/Install/NoStrip,$(1),I18N/LangTags/List.pm)
endef

$(eval $(call BuildPackage,perlbase-i18n))


define Package/perlbase-if
$(call Package/perlbase-template)
TITLE:=if perl module
endef

define Package/perlbase-if/install
$(call perlmod/Install,$(1),if.pm,)
$(call perlmod/InstallBaseTests,$(1),dist/if/t)
endef

$(eval $(call BuildPackage,perlbase-if))


define Package/perlbase-integer
$(call Package/perlbase-template)
TITLE:=integer perl module
endef

define Package/perlbase-integer/install
$(call perlmod/Install,$(1),integer.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/integer.t)
endef

$(eval $(call BuildPackage,perlbase-integer))


define Package/perlbase-io
$(call Package/perlbase-template)
TITLE:=IO perl module
DEPENDS+=+perlbase-base +perlbase-bytes +perlbase-config +perlbase-errno +perlbase-essential +perlbase-fcntl +perlbase-list +perlbase-posix +perlbase-scalar +perlbase-selectsaver +perlbase-socket +perlbase-symbol +perlbase-tie +perlbase-xsloader
endef

define Package/perlbase-io/install
$(call perlmod/Install,$(1),IO IO.pm auto/IO,)
$(call perlmod/InstallBaseTests,$(1),cpan/IO-Compress/t cpan/IO-Socket-IP/t cpan/IO-Zlib/t dist/IO/Makefile.PL dist/IO/t)
endef

$(eval $(call BuildPackage,perlbase-io))


define Package/perlbase-ipc
$(call Package/perlbase-template)
TITLE:=IPC perl module
DEPENDS+=+perlbase-config +perlbase-essential +perlbase-file +perlbase-locale +perlbase-params +perlbase-symbol +perlbase-text
endef

define Package/perlbase-ipc/install
$(call perlmod/Install,$(1),IPC auto/IPC,)
$(call perlmod/InstallBaseTests,$(1),cpan/IPC-Cmd/t cpan/IPC-SysV/t ext/IPC-Open3/t)
endef

$(eval $(call BuildPackage,perlbase-ipc))


define Package/perlbase-json-pp
$(call Package/perlbase-template)
TITLE:=json-pp perl module
DEPENDS+=+perlbase-b +perlbase-base +perlbase-essential
endef

define Package/perlbase-json-pp/install
$(call perlmod/Install,$(1),JSON/PP JSON/PP.pm,)
$(call perlmod/InstallBaseTests,$(1),cpan/JSON-PP/t)
endef

$(eval $(call BuildPackage,perlbase-json-pp))


define Package/perlbase-less
$(call Package/perlbase-template)
TITLE:=less perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-less/install
$(call perlmod/Install,$(1),less.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/less.t)
endef

$(eval $(call BuildPackage,perlbase-less))


define Package/perlbase-list
$(call Package/perlbase-template)
TITLE:=List perl module
DEPENDS+=+perlbase-essential +perlbase-xsloader
endef

define Package/perlbase-list/install
$(call perlmod/Install,$(1),List auto/List,)
endef

$(eval $(call BuildPackage,perlbase-list))


define Package/perlbase-locale
$(call Package/perlbase-template)
TITLE:=Locale perl module
DEPENDS+=+perlbase-essential +perlbase-i18n +perlbase-integer +perlbase-utf8
endef

define Package/perlbase-locale/install
$(call perlmod/Install,$(1),Locale,Locale/Constants.pod Locale/Country.pod Locale/Currency.pod Locale/Language.pod Locale/Maketext.pod Locale/Maketext/TPJ13.pod Locale/Script.pod)
$(call perlmod/InstallBaseTests,$(1),cpan/Locale-Codes/t cpan/Locale-Maketext-Simple/t dist/Locale-Maketext/t)
endef

$(eval $(call BuildPackage,perlbase-locale))


define Package/perlbase-math
$(call Package/perlbase-template)
TITLE:=Math perl module
DEPENDS+=+perlbase-config +perlbase-essential +perlbase-scalar +perlbase-xsloader
endef

define Package/perlbase-math/install
$(call perlmod/Install,$(1),Math auto/Math,)
$(call perlmod/InstallBaseTests,$(1),cpan/Math-Complex/t dist/Math-BigInt-FastCalc/t dist/Math-BigInt/t dist/Math-BigRat/t)
endef

$(eval $(call BuildPackage,perlbase-math))


define Package/perlbase-memoize
$(call Package/perlbase-template)
TITLE:=Memoize perl module
DEPENDS+=+perlbase-config +perlbase-essential +perlbase-storable
endef

define Package/perlbase-memoize/install
$(call perlmod/Install,$(1),Memoize Memoize.pm,)
$(call perlmod/InstallBaseTests,$(1),cpan/Memoize/t)
endef

$(eval $(call BuildPackage,perlbase-memoize))


define Package/perlbase-meta-notation
$(call Package/perlbase-template)
TITLE:=meta_notation module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-meta-notation/install
$(call perlmod/Install,$(1),meta_notation.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/meta_notation.t)
endef

$(eval $(call BuildPackage,perlbase-meta-notation))


define Package/perlbase-mime
$(call Package/perlbase-template)
TITLE:=MIME perl module
DEPENDS+=+perlbase-essential +perlbase-xsloader
endef

define Package/perlbase-mime/install
$(call perlmod/Install,$(1),MIME auto/MIME,)
$(call perlmod/InstallBaseTests,$(1),cpan/MIME-Base64/t)
endef

$(eval $(call BuildPackage,perlbase-mime))


define Package/perlbase-module
$(call Package/perlbase-template)
TITLE:=Module perl module
DEPENDS+=+perlbase-essential +perlbase-file +perlbase-filehandle +perlbase-locale +perlbase-params +perlbase-version
endef

define Package/perlbase-module/install
$(call perlmod/Install,$(1),Module,)
$(call perlmod/InstallBaseTests,$(1),cpan/Module-Load-Conditional/t cpan/Module-Load/t cpan/Module-Loaded/t cpan/Module-Metadata/corpus cpan/Module-Metadata/lib cpan/Module-Metadata/t dist/Module-CoreList/t)
endef

$(eval $(call BuildPackage,perlbase-module))


define Package/perlbase-mro
$(call Package/perlbase-template)
TITLE:=mro perl module
DEPENDS+=+perlbase-essential +perlbase-xsloader
endef

define Package/perlbase-mro/install
$(call perlmod/Install,$(1),auto/mro mro.pm,)
endef

$(eval $(call BuildPackage,perlbase-mro))


define Package/perlbase-net
$(call Package/perlbase-template)
TITLE:=Net perl module
DEPENDS+=+perlbase-class +perlbase-errno +perlbase-essential +perlbase-fcntl +perlbase-filehandle +perlbase-io +perlbase-posix +perlbase-socket +perlbase-symbol +perlbase-time
endef

define Package/perlbase-net/install
$(call perlmod/Install,$(1),Net,Net/libnetFAQ.pod)
$(call perlmod/InstallBaseTests,$(1),cpan/libnet/t dist/Net-Ping/t lib/Net/hostent.t lib/Net/netent.t lib/Net/protoent.t lib/Net/servent.t)
endef

$(eval $(call BuildPackage,perlbase-net))


define Package/perlbase-next
$(call Package/perlbase-template)
TITLE:=NEXT perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-next/install
$(call perlmod/Install,$(1),NEXT.pm,)
$(call perlmod/InstallBaseTests,$(1),cpan/NEXT/t)
endef

$(eval $(call BuildPackage,perlbase-next))


define Package/perlbase-o
$(call Package/perlbase-template)
TITLE:=O perl module
DEPENDS+=+perlbase-b +perlbase-essential
endef

define Package/perlbase-o/install
$(call perlmod/Install,$(1),O.pm,)
endef

$(eval $(call BuildPackage,perlbase-o))


define Package/perlbase-opcode
$(call Package/perlbase-template)
TITLE:=Opcode perl module
DEPENDS+=+perlbase-essential +perlbase-xsloader
endef

define Package/perlbase-opcode/install
$(call perlmod/Install,$(1),auto/Opcode,)
$(call perlmod/Install/NoStrip,$(1),Opcode.pm,)
$(call perlmod/InstallBaseTests,$(1),ext/Opcode/t)
endef

$(eval $(call BuildPackage,perlbase-opcode))


define Package/perlbase-open
$(call Package/perlbase-template)
TITLE:=open perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-open/install
$(call perlmod/Install,$(1),open.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/open.t)
endef

$(eval $(call BuildPackage,perlbase-open))


define Package/perlbase-ops
$(call Package/perlbase-template)
TITLE:=ops perl module
DEPENDS+=+perlbase-opcode
endef

define Package/perlbase-ops/install
$(call perlmod/Install,$(1),ops.pm,)
endef

$(eval $(call BuildPackage,perlbase-ops))


define Package/perlbase-ostype
$(call Package/perlbase-template)
TITLE:=OSType perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-ostype/install
$(call perlmod/Install,$(1),Perl/OSType.pm,)
$(call perlmod/InstallBaseTests,$(1),cpan/Perl-OSType/t)
endef

$(eval $(call BuildPackage,perlbase-ostype))


define Package/perlbase-params
$(call Package/perlbase-template)
TITLE:=Params perl module
DEPENDS+=+perlbase-essential +perlbase-locale
endef

define Package/perlbase-params/install
$(call perlmod/Install,$(1),Params,)
$(call perlmod/InstallBaseTests,$(1),cpan/Params-Check/t)
endef

$(eval $(call BuildPackage,perlbase-params))


define Package/perlbase-perl5db
$(call Package/perlbase-template)
TITLE:=perl5db perl module
DEPENDS+=+perlbase-cwd +perlbase-essential
endef

define Package/perlbase-perl5db/install
$(call perlmod/Install,$(1),perl5db.pl,)
$(call perlmod/InstallBaseTests,$(1),lib/perl5db.t lib/perl5db/t)
endef

$(eval $(call BuildPackage,perlbase-perl5db))


define Package/perlbase-perlio
$(call Package/perlbase-template)
TITLE:=PerlIO perl module
DEPENDS+=+perlbase-essential +perlbase-mime +perlbase-xsloader
endef

define Package/perlbase-perlio/install
$(call perlmod/Install,$(1),PerlIO PerlIO.pm auto/PerlIO,)
$(call perlmod/InstallBaseTests,$(1),cpan/PerlIO-via-QuotedPrint/t ext/PerlIO-encoding/t ext/PerlIO-scalar/t ext/PerlIO-via/t)
endef

$(eval $(call BuildPackage,perlbase-perlio))


define Package/perlbase-pod
$(call Package/perlbase-template)
TITLE:=Pod perl module
DEPENDS+=+perlbase-config +perlbase-cwd +perlbase-encode +perlbase-essential +perlbase-fcntl +perlbase-file +perlbase-getopt +perlbase-integer +perlbase-posix +perlbase-symbol +perlbase-term +perlbase-text
endef

define Package/perlbase-pod/install
$(call perlmod/Install,$(1),Pod,Pod/Usage.pm)
$(call perlmod/Install/NoStrip,$(1),Pod/Usage.pm,)
$(call perlmod/InstallBaseTests,$(1),cpan/Pod-Checker/t cpan/Pod-Escapes/t cpan/Pod-Parser/lib cpan/Pod-Parser/scripts cpan/Pod-Parser/t cpan/Pod-Perldoc/t cpan/Pod-Simple/t cpan/Pod-Usage/scripts cpan/Pod-Usage/t cpan/podlators/t ext/Pod-Functions/Functions.pm ext/Pod-Functions/t ext/Pod-Html/t lib/Pod/t)
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/pod2man $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/pod2text $(1)/usr/bin/
endef

$(eval $(call BuildPackage,perlbase-pod))


define Package/perlbase-posix
$(call Package/perlbase-template)
TITLE:=POSIX perl module
DEPENDS+=+perlbase-essential +perlbase-fcntl +perlbase-tie +perlbase-xsloader
endef

define Package/perlbase-posix/install
$(call perlmod/Install,$(1),POSIX.pm auto/POSIX,)
$(call perlmod/InstallBaseTests,$(1),ext/POSIX/Makefile.PL ext/POSIX/POSIX.xs ext/POSIX/t)
endef

$(eval $(call BuildPackage,perlbase-posix))


define Package/perlbase-re
$(call Package/perlbase-template)
TITLE:=re perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-re/install
$(call perlmod/Install,$(1),auto/re re.pm,)
$(call perlmod/InstallBaseTests,$(1),ext/re/t)
endef

$(eval $(call BuildPackage,perlbase-re))


define Package/perlbase-safe
$(call Package/perlbase-template)
TITLE:=Safe perl module
DEPENDS+=+perlbase-b +perlbase-essential +perlbase-opcode +perlbase-scalar +perlbase-utf8
endef

define Package/perlbase-safe/install
$(call perlmod/Install,$(1),Safe.pm,)
$(call perlmod/InstallBaseTests,$(1),dist/Safe/t)
endef

$(eval $(call BuildPackage,perlbase-safe))


define Package/perlbase-scalar
$(call Package/perlbase-template)
TITLE:=Scalar perl module
DEPENDS+=+perlbase-essential +perlbase-list
endef

define Package/perlbase-scalar/install
$(call perlmod/Install,$(1),Scalar Sub,)
$(call perlmod/InstallBaseTests,$(1),cpan/Scalar-List-Utils/t)
	$(INSTALL_DIR) $(1)/$(PERL_TESTSDIR)/cpan/Scalar-List-Utils/blib
endef

$(eval $(call BuildPackage,perlbase-scalar))


define Package/perlbase-sdbm-file
$(call Package/perlbase-template)
TITLE:=SDBM_File perl module
DEPENDS+=+perlbase-essential +perlbase-tie +perlbase-xsloader
endef

define Package/perlbase-sdbm-file/install
$(call perlmod/Install,$(1),SDBM_File.pm auto/SDBM_File,)
$(call perlmod/InstallBaseTests,$(1),ext/SDBM_File/t)
endef

$(eval $(call BuildPackage,perlbase-sdbm-file))


define Package/perlbase-search
$(call Package/perlbase-template)
TITLE:=Search perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-search/install
$(call perlmod/Install,$(1),Search,)
$(call perlmod/InstallBaseTests,$(1),dist/Search-Dict/t)
endef

$(eval $(call BuildPackage,perlbase-search))


define Package/perlbase-selectsaver
$(call Package/perlbase-template)
TITLE:=SelectSaver perl module
DEPENDS+=+perlbase-essential +perlbase-symbol
endef

define Package/perlbase-selectsaver/install
$(call perlmod/Install,$(1),SelectSaver.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/SelectSaver.t)
endef

$(eval $(call BuildPackage,perlbase-selectsaver))


define Package/perlbase-selfloader
$(call Package/perlbase-template)
TITLE:=SelfLoader perl module
DEPENDS+=+perlbase-essential +perlbase-io
endef

define Package/perlbase-selfloader/install
$(call perlmod/Install,$(1),SelfLoader.pm,)
$(call perlmod/InstallBaseTests,$(1),dist/SelfLoader/t)
endef

$(eval $(call BuildPackage,perlbase-selfloader))


define Package/perlbase-sigtrap
$(call Package/perlbase-template)
TITLE:=sigtrap perl module
DEPENDS+=+perlbase-essential +perlbase-meta-notation
endef

define Package/perlbase-sigtrap/install
$(call perlmod/Install,$(1),sigtrap.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/sigtrap.t)
endef

$(eval $(call BuildPackage,perlbase-sigtrap))


define Package/perlbase-socket
$(call Package/perlbase-template)
TITLE:=Socket perl module
DEPENDS+=+perlbase-essential +perlbase-xsloader
endef

define Package/perlbase-socket/install
$(call perlmod/Install,$(1),Socket.pm auto/Socket,)
$(call perlmod/InstallBaseTests,$(1),cpan/Socket/t)
endef

$(eval $(call BuildPackage,perlbase-socket))


define Package/perlbase-sort
$(call Package/perlbase-template)
TITLE:=sort perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-sort/install
$(call perlmod/Install,$(1),sort.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/sort.t)
endef

$(eval $(call BuildPackage,perlbase-sort))


define Package/perlbase-storable
$(call Package/perlbase-template)
TITLE:=Storable perl module
DEPENDS+=+perlbase-essential +perlbase-xsloader
endef

define Package/perlbase-storable/install
$(call perlmod/Install,$(1),Storable.pm auto/Storable,)
$(call perlmod/InstallBaseTests,$(1),dist/Storable/t)
endef

$(eval $(call BuildPackage,perlbase-storable))


define Package/perlbase-symbol
$(call Package/perlbase-template)
TITLE:=Symbol perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-symbol/install
$(call perlmod/Install,$(1),Symbol.pm,)
$(call perlmod/InstallBaseTests,$(1),lib/Symbol.t)
endef

$(eval $(call BuildPackage,perlbase-symbol))


define Package/perlbase-sys
$(call Package/perlbase-template)
TITLE:=Sys perl module
DEPENDS+=+perlbase-essential +perlbase-file +perlbase-posix +perlbase-socket
endef

define Package/perlbase-sys/install
$(call perlmod/Install,$(1),Sys auto/Sys,)
$(call perlmod/InstallBaseTests,$(1),cpan/Sys-Syslog/t ext/Sys-Hostname/t)
endef

$(eval $(call BuildPackage,perlbase-sys))


define Package/perlbase-tap
$(call Package/perlbase-template)
TITLE:=TAP perl module
DEPENDS+=+perlbase-base +perlbase-benchmark +perlbase-config +perlbase-essential +perlbase-file +perlbase-io +perlbase-posix +perlbase-text
endef

define Package/perlbase-tap/install
$(call perlmod/Install,$(1),TAP,)
endef

$(eval $(call BuildPackage,perlbase-tap))


define Package/perlbase-term
$(call Package/perlbase-template)
TITLE:=Term perl module
DEPENDS+=+perlbase-essential
endef

define Package/perlbase-term/install
$(call perlmod/Install,$(1),Term,)
$(call perlmod/InstallBaseTests,$(1),cpan/Term-ANSIColor/t cpan/Term-Cap/test.pl dist/Term-Complete/t dist/Term-ReadLine/t)
endef

$(eval $(call BuildPackage,perlbase-term))


define Package/perlbase-test
$(call Package/perlbase-template)
TITLE:=Test perl module
DEPENDS+=+perlbase-base +perlbase-config +perlbase-essential +perlbase-io +perlbase-symbol +perlbase-tap +perlbase-text
endef

define Package/perlbase-test/install
$(call perlmod/Install,$(1),Test Test.pm ok.pm,Test/Builder.pm Test/Harness/TAP.pod Test/More.pm Test/Tutorial.pod)
$(call perlmod/Install/NoStrip,$(1),Test/Builder.pm Test/More.pm,)
$(call perlmod/InstallBaseTests,$(1),cpan/Test-Harness/t cpan/Test-Simple/t cpan/Test/t)
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/prove $(1)/usr/bin
endef

$(eval $(call BuildPackage,perlbase-test))


define Package/perlbase-text
$(call Package/perlbase-template)
TITLE:=Text perl module
DEPENDS+=+perlbase-essential +perlbase-selfloader
endef

define Package/perlbase-text/install
$(call perlmod/Install,$(1),Text,)
$(call perlmod/InstallBaseTests,$(1),cpan/Text-Balanced/t cpan/Text-ParseWords/t cpan/Text-Tabs/t dist/Text-Abbrev/t)
endef

$(eval $(call BuildPackage,perlbase-text))


define Package/perlbase-thread
$(call Package/perlbase-template)
TITLE:=Thread perl module
DEPENDS+=+perlbase-essential +perlbase-scalar +perlbase-threads
endef

define Package/perlbase-thread/install
$(call perlmod/Install,$(1),Thread Thread.pm,)
$(call perlmod/InstallBaseTests,$(1),dist/Thread-Queue/t dist/Thread-Semaphore/t lib/Thread.t)
endef

$(eval $(call BuildPackage,perlbase-thread))


define Package/perlbase-threads
$(call Package/perlbase-template)
TITLE:=threads perl module
DEPENDS+=+perlbase-config +perlbase-essential +perlbase-scalar +perlbase-xsloader
endef

define Package/perlbase-threads/install
$(call perlmod/Install,$(1),auto/threads threads threads.pm,)
$(call perlmod/InstallBaseTests,$(1),dist/threads-shared/t dist/threads/t)
endef

$(eval $(call BuildPackage,perlbase-threads))


define Package/perlbase-tie
$(call Package/perlbase-template)
TITLE:=Tie perl module
DEPENDS+=+perlbase-essential +perlbase-fcntl +perlbase-xsloader
endef

define Package/perlbase-tie/install
$(call perlmod/Install,$(1),Tie auto/Tie,)
$(call perlmod/InstallBaseTests,$(1),cpan/Tie-RefHash/t dist/Tie-File/t ext/Tie-Hash-NamedCapture/t ext/Tie-Memoize/lib/Tie/Memoize.pm ext/Tie-Memoize/t lib/Tie/Array/push.t lib/Tie/Array/splice.t lib/Tie/Array/std.t lib/Tie/Array/stdpush.t lib/Tie/ExtraHash.t lib/Tie/Handle/stdhandle.t lib/Tie/Handle/stdhandle_from_handle.t lib/Tie/Hash.t lib/Tie/Scalar.t lib/Tie/SubstrHash.t)
endef

$(eval $(call BuildPackage,perlbase-tie))


define Package/perlbase-time
$(call Package/perlbase-template)
TITLE:=Time perl module
DEPENDS+=+perlbase-class +perlbase-config +perlbase-dynaloader +perlbase-essential
endef

define Package/perlbase-time/install
$(call perlmod/Install,$(1),Time auto/Time,)
$(call perlmod/InstallBaseTests,$(1),cpan/Time-HiRes/t cpan/Time-Local/t cpan/Time-Piece/t lib/Time/gmtime.t lib/Time/localtime.t)
endef

$(eval $(call BuildPackage,perlbase-time))


define Package/perlbase-unicode
$(call Package/perlbase-template)
TITLE:=Unicode perl module
DEPENDS+=+perlbase-base +perlbase-charnames +perlbase-dynaloader +perlbase-essential +perlbase-file
endef

define Package/perlbase-unicode/install
$(call perlmod/Install,$(1),Unicode auto/Unicode,)
$(call perlmod/InstallBaseTests,$(1),cpan/Unicode-Collate/t cpan/Unicode-Normalize/t lib/Unicode/UCD.t)
endef

$(eval $(call BuildPackage,perlbase-unicode))


define Package/perlbase-unicore
$(call Package/perlbase-template)
TITLE:=unicore perl module
endef

define Package/perlbase-unicore/install
$(call perlmod/Install,$(1),unicore,)
$(call perlmod/InstallBaseTests,$(1),lib/unicore/NameAliases.txt lib/unicore/PropValueAliases.txt lib/unicore/PropertyAliases.txt lib/unicore/UnicodeData.txt)
endef

$(eval $(call BuildPackage,perlbase-unicore))


define Package/perlbase-universal
$(call Package/perlbase-template)
TITLE:=UNIVERSAL perl module
endef

define Package/perlbase-universal/install
$(call perlmod/Install,$(1),UNIVERSAL.pm,)
endef

$(eval $(call BuildPackage,perlbase-universal))


define Package/perlbase-user
$(call Package/perlbase-template)
TITLE:=User perl module
DEPENDS+=+perlbase-class +perlbase-config +perlbase-essential
endef

define Package/perlbase-user/install
$(call perlmod/Install,$(1),User,)
$(call perlmod/InstallBaseTests,$(1),lib/User/grent.t lib/User/pwent.t)
endef

$(eval $(call BuildPackage,perlbase-user))


define Package/perlbase-utf8
$(call Package/perlbase-template)
TITLE:=utf8 perl module
DEPENDS+=+perlbase-essential +perlbase-re +perlbase-unicore
endef

define Package/perlbase-utf8/install
$(call perlmod/Install,$(1),utf8.pm utf8_heavy.pl,)
$(call perlmod/InstallBaseTests,$(1),lib/utf8.t)
endef

$(eval $(call BuildPackage,perlbase-utf8))


define Package/perlbase-version
$(call Package/perlbase-template)
TITLE:=version perl module
DEPENDS+=+perlbase-config +perlbase-essential
endef

define Package/perlbase-version/install
$(call perlmod/Install,$(1),version version.pm,)
$(call perlmod/InstallBaseTests,$(1),cpan/version/t)
endef

$(eval $(call BuildPackage,perlbase-version))


define Package/perlbase-xsloader
$(call Package/perlbase-template)
TITLE:=XSLoader perl module
endef

define Package/perlbase-xsloader/install
$(call perlmod/Install,$(1),XSLoader.pm,)
$(call perlmod/InstallBaseTests,$(1),dist/XSLoader/t ext/XS-APItest/t ext/XS-Typemap/t)
endef

$(eval $(call BuildPackage,perlbase-xsloader))


# Generated Fri Sep 25 07:47:59 2015
