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

AXTLS_HOME=../..

include $(AXTLS_HOME)/config/.config
include $(AXTLS_HOME)/config/makefile.conf

all: lib

ifdef CONFIG_PLATFORM_WIN32
TARGET=$(AXTLS_HOME)/$(STAGE)/axtlsp.dll
else
TARGET=$(AXTLS_HOME)/$(STAGE)/libaxtlsp.so
endif

ifneq ($(MAKECMDGOALS), clean)

ifdef CONFIG_PLATFORM_WIN32
PERL5_CORE:=$(shell cygpath -w "$(CONFIG_PERL_CORE)")
else
PERL5_CORE= $(shell perl -e 'use Config; print $$Config{archlib};')/CORE
endif

all: test_perl

test_perl:
	@if ! [ -d "$(PERL5_CORE)" ]; then \
		echo "*** Error: Perl not installed at $(CONFIG_PERL_CORE) - go to " \
        "http://www.cpan.org/authors/id/G/GR/GRAHAMC/SiePerl-5.8.0-bin-1.0-Win32.INSTALL.exe" && exit 1; \
	fi

endif

lib: $(TARGET)
OBJ:=axTLSp_wrap.o
include $(AXTLS_HOME)/config/makefile.post

ifndef CONFIG_PLATFORM_WIN32        # Linux/Unix/Cygwin

#
# Could have used libperl.a, but it increases the library to over 1MB, so just
# use libperl.so. But this needs to be in the shared library path for things to
# work.
#
$(TARGET) : $(OBJ)
	$(LD) $(LDFLAGS) -L$(AXTLS_HOME)/$(STAGE) -L$(PERL5_CORE) $(LDSHARED) -o $@ $(OBJ) -laxtls -lperl
ifdef CONFIG_PLATFORM_CYGWIN
	cd $(AXTLS_HOME)/$(STAGE); ln -sf $(notdir $@) axtlsp.dll
endif
	@install axtlsp.pm $(AXTLS_HOME)/$(STAGE)

CFLAGS += -D_GNU_SOURCE -I$(PERL5_CORE)
else
CFLAGS += /I"$(PERL5_CORE)"
LDFLAGS += $(CONFIG_PERL_LIB) /libpath:"$(PERL5_CORE)" axtls.lib /libpath:"$(AXTLS_HOME)/$(STAGE)"

$(TARGET) : $(OBJ)
	$(LD) $(LDFLAGS) $(LDSHARED) /out:$@ $(OBJ)
	install axtlsp.pm $(AXTLS_HOME)/$(STAGE)
endif # WIN32

clean::
	@rm -f $(TARGET) axtls* *.i axTLSp* *.c .depend $(AXTLS_HOME)/$(STAGE)/axtlsp.pm
