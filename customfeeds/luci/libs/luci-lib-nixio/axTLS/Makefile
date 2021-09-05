#
# Copyright (c) 2007, Cameron Rich
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# * Neither the name of the axTLS project nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

-include config/.config

ifneq ($(strip $(HAVE_DOT_CONFIG)),y)
all: menuconfig
else
all: target
endif

include config/makefile.conf

target : $(STAGE) $(TARGET)

# VERSION has to come from the command line
RELEASE=axTLS-$(VERSION)

# standard version
target:
	$(MAKE) -C crypto
	$(MAKE) -C ssl
ifdef CONFIG_AXHTTPD
	$(MAKE) -C httpd
endif
ifdef CONFIG_BINDINGS
	$(MAKE) -C bindings
endif
ifdef CONFIG_SAMPLES
	$(MAKE) -C samples
endif

$(STAGE) : ssl/version.h
	@mkdir -p $(STAGE)

# create a version file with something in it.
ssl/version.h:
	@echo "#define AXTLS_VERSION    \"(no version)\"" > ssl/version.h

$(PREFIX) :
	@mkdir -p $(PREFIX)/lib
	@mkdir -p $(PREFIX)/bin

release:
	$(MAKE) -C config/scripts/config clean
	-$(MAKE) clean
	-@rm config/*.msi config/*.back.aip config/config.h config/.config*
	-@rm www/index.20*
	-@rm -fr $(STAGE)
	@echo "#define AXTLS_VERSION    \"$(VERSION)\"" > ssl/version.h
	cd ../; tar cvfz $(RELEASE).tar.gz --wildcards-match-slash --exclude .svn axTLS; cd -;

docs:
	$(MAKE) -C docsrc doco

# build the Win32 demo release version
win32_demo:
	@echo "#define AXTLS_VERSION    \"$(VERSION)\"" > ssl/version.h
	$(MAKE) win32releaseconf

install: $(PREFIX) all
	cp --no-dereference $(STAGE)/libax* $(PREFIX)/lib
	chmod 755 $(PREFIX)/lib/libax* 
ifdef CONFIG_SAMPLES
	install -m 755 $(STAGE)/ax* $(PREFIX)/bin 
endif
ifdef CONFIG_HTTP_HAS_AUTHORIZATION
	install -m 755 $(STAGE)/htpasswd $(PREFIX)/bin 
endif
ifdef CONFIG_PLATFORM_CYGWIN
	install -m 755 $(STAGE)/cygaxtls.dll $(PREFIX)/bin 
endif
ifdef CONFIG_PERL_BINDINGS 
	install -m 755 $(STAGE)/axtlsp.pm `perl -e 'use Config; print $$Config{installarchlib};'`
endif
	@mkdir -p -m 755 $(PREFIX)/include/axTLS
	install -m 644 crypto/*.h $(PREFIX)/include/axTLS
	install -m 644 ssl/*.h $(PREFIX)/include/axTLS
	-rm $(PREFIX)/include/axTLS/cert.h
	-rm $(PREFIX)/include/axTLS/private_key.h
	install -m 644 config/config.h $(PREFIX)/include/axTLS

installclean:
	-@rm $(PREFIX)/lib/libax* > /dev/null 2>&1
	-@rm $(PREFIX)/bin/ax* > /dev/null 2>&1
	-@rm $(PREFIX)/bin/axhttpd* > /dev/null 2>&1
	-@rm `perl -e 'use Config; print $$Config{installarchlib};'`/axtlsp.pm > /dev/null 2>&1

test:
	cd $(STAGE); ssltest; ../ssl/test/test_axssl.sh; cd -;

# tidy up things
clean::
	@cd crypto; $(MAKE) clean
	@cd ssl; $(MAKE) clean
	@cd httpd; $(MAKE) clean
	@cd samples; $(MAKE) clean
	@cd docsrc; $(MAKE) clean
	@cd bindings; $(MAKE) clean

# ---------------------------------------------------------------------------
# mconf stuff
# ---------------------------------------------------------------------------

CONFIG_CONFIG_IN = config/Config.in
CONFIG_DEFCONFIG = config/defconfig

config/scripts/config/conf: config/scripts/config/Makefile
	$(MAKE) -C config/scripts/config conf
	-@if [ ! -f config/.config ] ; then \
		cp $(CONFIG_DEFCONFIG) config/.config; \
	fi

config/scripts/config/mconf: config/scripts/config/Makefile
	$(MAKE) -C config/scripts/config ncurses conf mconf
	-@if [ ! -f config/.config ] ; then \
		cp $(CONFIG_DEFCONFIG) .config; \
	fi

cleanconf:
	$(MAKE) -C config/scripts/config clean
	@rm -f config/.config

menuconfig: config/scripts/config/mconf
	@./config/scripts/config/mconf $(CONFIG_CONFIG_IN)

config: config/scripts/config/conf
	@./config/scripts/config/conf $(CONFIG_CONFIG_IN)

oldconfig: config/scripts/config/conf
	@./config/scripts/config/conf -o $(CONFIG_CONFIG_IN)

default: config/scripts/config/conf
	@./config/scripts/config/conf -d $(CONFIG_CONFIG_IN) > /dev/null
	$(MAKE)

randconfig: config/scripts/config/conf
	@./config/scripts/config/conf -r $(CONFIG_CONFIG_IN)

allnoconfig: config/scripts/config/conf
	@./config/scripts/config/conf -n $(CONFIG_CONFIG_IN)

allyesconfig: config/scripts/config/conf
	@./config/scripts/config/conf -y $(CONFIG_CONFIG_IN)

# The special win32 release configuration
win32releaseconf: config/scripts/config/conf
	@./config/scripts/config/conf -D config/win32config $(CONFIG_CONFIG_IN) > /dev/null
	$(MAKE)

# The special linux release configuration
linuxconf: config/scripts/config/conf
	@./config/scripts/config/conf -D config/linuxconfig $(CONFIG_CONFIG_IN) > /dev/null
	$(MAKE)
