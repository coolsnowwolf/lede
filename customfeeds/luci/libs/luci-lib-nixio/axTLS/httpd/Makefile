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

all : web_server lua

AXTLS_HOME=..

include $(AXTLS_HOME)/config/.config
include $(AXTLS_HOME)/config/makefile.conf

ifndef CONFIG_PLATFORM_WIN32

ifdef CONFIG_PLATFORM_CYGWIN
TARGET=$(AXTLS_HOME)/$(STAGE)/axhttpd.exe
TARGET2=$(AXTLS_HOME)/$(STAGE)/htpasswd.exe
else
TARGET=$(AXTLS_HOME)/$(STAGE)/axhttpd
TARGET2=$(AXTLS_HOME)/$(STAGE)/htpasswd
endif

ifdef CONFIG_HTTP_STATIC_BUILD
LIBS=$(AXTLS_HOME)/$(STAGE)/libaxtls.a
else
LIBS=-L$(AXTLS_HOME)/$(STAGE) -laxtls
endif

ifdef CONFIG_HTTP_BUILD_LUA
lua: kepler-1.1

kepler-1.1:
	@tar xvfz   kepler-1.1-snapshot-20070521-1825.tar.gz
	@cat kepler.patch | patch -p0
	cd kepler-1.1; ./configure --prefix=$(CONFIG_HTTP_LUA_PREFIX) --launcher=cgi --lua-suffix= ; make install
else
lua:
endif

else # win32 build
lua:

TARGET=$(AXTLS_HOME)/$(STAGE)/axhttpd.exe
TARGET2=$(AXTLS_HOME)/$(STAGE)/htpasswd.exe

ifdef CONFIG_HTTP_STATIC_BUILD
LIBS=$(AXTLS_HOME)/$(STAGE)/axtls.static.lib $(AXTLS_HOME)\\config\\axtls.res
else
LIBS=$(AXTLS_HOME)/$(STAGE)/axtls.lib $(AXTLS_HOME)\\config\\axtls.res
endif
endif

ifndef CONFIG_AXHTTPD
web_server:
else

web_server :: $(TARGET)

ifdef CONFIG_HTTP_HAS_AUTHORIZATION
web_server :: $(TARGET2)
endif

OBJ= \
	axhttpd.o \
	proc.o \
	tdate_parse.o

include $(AXTLS_HOME)/config/makefile.post

ifndef CONFIG_PLATFORM_WIN32

$(TARGET): $(OBJ) $(AXTLS_HOME)/$(STAGE)/libaxtls.a
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBS)
ifdef CONFIG_STRIP_UNWANTED_SECTIONS
	$(STRIP) --remove-section=.comment $(TARGET)
endif

$(TARGET2): htpasswd.o $(AXTLS_HOME)/$(STAGE)/libaxtls.a
	$(LD) $(LDFLAGS) -o $@ htpasswd.o $(LIBS)

else    # Win32

OBJ:=$(OBJ:.o=.obj)
%.obj : %.c
	$(CC) $(CFLAGS) $< 

htpasswd.obj : htpasswd.c
	$(CC) $(CFLAGS) $? 
	
$(TARGET): $(OBJ)
	$(LD) $(LDFLAGS) /out:$@ $(LIBS) $?

$(TARGET2): htpasswd.obj
	$(LD) $(LDFLAGS) /out:$@ $(LIBS) $?
endif

endif       # CONFIG_AXHTTPD

clean::
	-@rm -f $(TARGET)*
	-@rm -fr kepler-1.1

