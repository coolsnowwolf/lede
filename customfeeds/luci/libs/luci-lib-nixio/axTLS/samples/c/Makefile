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

all : sample

AXTLS_HOME=../..

include $(AXTLS_HOME)/config/.config
include $(AXTLS_HOME)/config/makefile.conf

ifndef CONFIG_PLATFORM_WIN32

ifdef CONFIG_PLATFORM_CYGWIN
TARGET=$(AXTLS_HOME)/$(STAGE)/axssl.exe
else
TARGET=$(AXTLS_HOME)/$(STAGE)/axssl
endif   # cygwin

LIBS=$(AXTLS_HOME)/$(STAGE)
else
TARGET=$(AXTLS_HOME)/$(STAGE)/axssl.exe
endif

ifndef CONFIG_C_SAMPLES
sample:

else
sample : $(TARGET)
OBJ= axssl.o
include $(AXTLS_HOME)/config/makefile.post

ifndef CONFIG_PLATFORM_WIN32

$(TARGET): $(OBJ) $(LIBS)/libaxtls.a
	$(LD) $(LDFLAGS) -o $@ $(OBJ) -L$(LIBS) -laxtls 
ifdef CONFIG_STRIP_UNWANTED_SECTIONS
	$(STRIP) --remove-section=.comment $(TARGET)
endif   # use strip
else    # Win32

$(TARGET): $(OBJ)
	$(LD) $(LDFLAGS) $(AXTLS_HOME)/config/axtls.res /out:$@ $^ /libpath:"$(AXTLS_HOME)/$(STAGE)" axtls.lib
endif

endif    # CONFIG_C_SAMPLES

clean::
	-@rm -f $(AXTLS_HOME)/$(STAGE)/axssl*

