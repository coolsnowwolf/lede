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
TARGET=$(AXTLS_HOME)/$(STAGE)/axtlsl.dll
else
TARGET=$(CONFIG_LUA_CORE)/lib/lua/5.1/axtlsl.so
endif

ifneq ($(MAKECMDGOALS), clean)

lib: $(TARGET)
OBJ:=axTLSl_wrap.o
include $(AXTLS_HOME)/config/makefile.post

# there are a few static functions that aren't used
CFLAGS += -funit-at-a-time

$(TARGET) : $(OBJ)
	$(LD) $(LDFLAGS) $(LDSHARED) -o $@ $^ -L$(AXTLS_HOME)/$(STAGE) -L$(CONFIG_LUA_CORE)/lib -laxtls -llua

CFLAGS += -I $(CONFIG_LUA_CORE)/include
else
CFLAGS += /I"`cygpath -w $(CONFIG_LUA_CORE)/include`"
LDFLAGS += axtls.lib /libpath:"$(AXTLS_HOME)/$(STAGE)"

$(TARGET) : $(OBJ)
	$(LD) $(LDFLAGS) $(LDSHARED) /out:$@ $(OBJ)
endif # WIN32

clean::
	@rm -f $(TARGET) *.i axTLSl* .depend
