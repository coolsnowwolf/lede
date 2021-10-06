# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2020 OpenWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/prereq.mk

SHELL:=sh
PKG_NAME:=Build dependency


# Required for the toolchain
$(eval $(call TestHostCommand,working-make, \
	Please install GNU make v4.1 or later., \
	$(MAKE) -v | grep -E 'Make (4\.[1-9]|[5-9]\.)'))

$(eval $(call TestHostCommand,case-sensitive-fs, \
	OpenWrt can only be built on a case-sensitive filesystem, \
	rm -f $(TMP_DIR)/test.*; touch $(TMP_DIR)/test.fs; \
		test ! -f $(TMP_DIR)/test.FS))

$(eval $(call TestHostCommand,proper-umask, \
	Please build with umask 022 - other values produce broken packages, \
	umask | grep -xE 0?0[012][012]))

ifndef IB
$(eval $(call SetupHostCommand,gcc, \
	Please install the GNU C Compiler (gcc) 6 or later, \
	$(CC) -dumpversion | grep -E '^([6-9]\.?|1[0-9]\.?)', \
	gcc -dumpversion | grep -E '^([6-9]\.?|1[0-9]\.?)', \
	gcc --version | grep -E 'Apple.(LLVM|clang)' ))

$(eval $(call TestHostCommand,working-gcc, \
	Please reinstall the GNU C Compiler (6 or later) - \
	it appears to be broken, \
	echo 'int main(int argc, char **argv) { return 0; }' | \
		gcc -x c -o $(TMP_DIR)/a.out -))

$(eval $(call SetupHostCommand,g++, \
	Please install the GNU C++ Compiler (g++) 6 or later, \
	$(CXX) -dumpversion | grep -E '^([6-9]\.?|1[0-9]\.?)', \
	g++ -dumpversion | grep -E '^([6-9]\.?|1[0-9]\.?)', \
	g++ --version | grep -E 'Apple.(LLVM|clang)' ))

$(eval $(call TestHostCommand,working-g++, \
	Please reinstall the GNU C++ Compiler (6 or later) - \
	it appears to be broken, \
	echo 'int main(int argc, char **argv) { return 0; }' | \
		g++ -x c++ -o $(TMP_DIR)/a.out - -lstdc++ && \
		$(TMP_DIR)/a.out))

$(eval $(call TestHostCommand,ncurses, \
	Please install ncurses. (Missing libncurses.so or ncurses.h), \
	echo 'int main(int argc, char **argv) { initscr(); return 0; }' | \
		gcc -include ncurses.h -x c -o $(TMP_DIR)/a.out - -lncurses))
endif # IB

ifeq ($(HOST_OS),Linux)
  zlib_link_flags := -Wl,-Bstatic -lz -Wl,-Bdynamic
else
  zlib_link_flags := -lz
endif

$(eval $(call TestHostCommand,perl-data-dumper, \
	Please install the Perl Data::Dumper module, \
	perl -MData::Dumper -e 1))

$(eval $(call TestHostCommand,perl-findbin, \
	Please install the Perl FindBin module, \
	perl -MFindBin -e 1))

$(eval $(call TestHostCommand,perl-file-copy, \
	Please install the Perl File::Copy module, \
	perl -MFile::Copy -e 1))

$(eval $(call TestHostCommand,perl-file-compare, \
	Please install the Perl File::Compare module, \
	perl -MFile::Compare -e 1))

$(eval $(call TestHostCommand,perl-thread-queue, \
	Please install the Perl Thread::Queue module, \
	perl -MThread::Queue -e 1))

$(eval $(call SetupHostCommand,tar,Please install GNU 'tar', \
	gtar --version 2>&1 | grep GNU, \
	gnutar --version 2>&1 | grep GNU, \
	tar --version 2>&1 | grep GNU))

$(eval $(call SetupHostCommand,find,Please install GNU 'find', \
	gfind --version 2>&1 | grep GNU, \
	find --version 2>&1 | grep GNU))

$(eval $(call SetupHostCommand,bash,Please install GNU 'bash', \
	bash --version 2>&1 | grep GNU))

$(eval $(call SetupHostCommand,xargs, \
	Please install 'xargs' that supports '-r/--no-run-if-empty', \
	gxargs -r --version, \
	xargs -r --version))

$(eval $(call SetupHostCommand,patch,Please install GNU 'patch', \
	gpatch --version 2>&1 | grep 'Free Software Foundation', \
	patch --version 2>&1 | grep 'Free Software Foundation'))

$(eval $(call SetupHostCommand,diff,Please install GNU diffutils, \
	gdiff --version 2>&1 | grep GNU, \
	diff --version 2>&1 | grep GNU))

$(eval $(call SetupHostCommand,cp,Please install GNU fileutils, \
	gcp --help 2>&1 | grep 'Copy SOURCE', \
	cp --help 2>&1 | grep 'Copy SOURCE'))

$(eval $(call SetupHostCommand,seq,Please install seq, \
	gseq --version, \
	seq --version 2>&1 | grep seq))

$(eval $(call SetupHostCommand,awk,Please install GNU 'awk', \
	gawk --version 2>&1 | grep GNU, \
	awk --version 2>&1 | grep GNU))

$(eval $(call SetupHostCommand,grep,Please install GNU 'grep', \
	ggrep --version 2>&1 | grep GNU, \
	grep --version 2>&1 | grep GNU))

$(eval $(call SetupHostCommand,egrep,Please install GNU 'grep', \
	gegrep --version 2>&1 | grep GNU, \
	egrep --version 2>&1 | grep GNU))

$(eval $(call SetupHostCommand,getopt, \
	Please install an extended getopt version that supports --long, \
	gnugetopt -o t --long test -- --test | grep '^ *--test *--', \
	getopt -o t --long test -- --test | grep '^ *--test *--', \
	/usr/local/opt/gnu-getopt/bin/getopt -o t --long test -- --test | grep '^ *--test *--'))

$(eval $(call SetupHostCommand,stat,Cannot find a file stat utility, \
	gnustat -c%s $(TOPDIR)/Makefile, \
	gstat -c%s $(TOPDIR)/Makefile, \
	stat -c%s $(TOPDIR)/Makefile))

$(eval $(call SetupHostCommand,unzip,Please install 'unzip', \
	unzip 2>&1 | grep zipfile, \
	unzip))

$(eval $(call SetupHostCommand,bzip2,Please install 'bzip2', \
	bzip2 --version </dev/null))

$(eval $(call SetupHostCommand,wget,Please install GNU 'wget', \
	wget --version | grep GNU))

$(eval $(call SetupHostCommand,install,Please install GNU 'install', \
	install --version | grep GNU, \
	ginstall --version | grep GNU))

$(eval $(call SetupHostCommand,perl,Please install Perl 5.x, \
	perl --version | grep "perl.*v5"))

$(eval $(call CleanupPython2))

$(eval $(call SetupHostCommand,python,Please install Python >= 3.6, \
	python3.9 -V 2>&1 | grep 'Python 3', \
	python3.8 -V 2>&1 | grep 'Python 3', \
	python3.7 -V 2>&1 | grep 'Python 3', \
	python3.6 -V 2>&1 | grep 'Python 3', \
	python3 -V 2>&1 | grep -E 'Python 3\.[6-9]\.?'))

$(eval $(call SetupHostCommand,python3,Please install Python >= 3.6, \
	python3.9 -V 2>&1 | grep 'Python 3', \
	python3.8 -V 2>&1 | grep 'Python 3', \
	python3.7 -V 2>&1 | grep 'Python 3', \
	python3.6 -V 2>&1 | grep 'Python 3', \
	python3 -V 2>&1 | grep -E 'Python 3\.[6-9]\.?'))

$(eval $(call SetupHostCommand,git,Please install Git (git-core) >= 1.7.12.2, \
	git --exec-path | xargs -I % -- grep -q -- --recursive %/git-submodule))

$(eval $(call SetupHostCommand,file,Please install the 'file' package, \
	file --version 2>&1 | grep file))

$(eval $(call SetupHostCommand,rsync,Please install 'rsync', \
	rsync --version </dev/null))

$(eval $(call SetupHostCommand,which,Please install 'which', \
	which which | grep which))

$(STAGING_DIR_HOST)/bin/mkhash: $(SCRIPT_DIR)/mkhash.c
	mkdir -p $(dir $@)
	$(CC) -O2 -I$(TOPDIR)/tools/include -o $@ $<

prereq: $(STAGING_DIR_HOST)/bin/mkhash

# Install ldconfig stub
$(eval $(call TestHostCommand,ldconfig-stub,Failed to install stub, \
	touch $(STAGING_DIR_HOST)/bin/ldconfig && \
	chmod +x $(STAGING_DIR_HOST)/bin/ldconfig))
