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
include $(AXTLS_HOME)/config/makefile.java.conf

all: lib jar

JAR=$(AXTLS_HOME)/$(STAGE)/axtls.jar

ifdef CONFIG_PLATFORM_WIN32
TARGET=$(AXTLS_HOME)/$(STAGE)/axtlsj.dll
else
TARGET=$(AXTLS_HOME)/$(STAGE)/libaxtlsj.so
endif

lib: $(TARGET)
axTLSj_wrap.o : axTLSj_wrap.c

JAVA_FILES= \
	axtlsjJNI.java \
	axtlsjConstants.java \
	axtlsj.java \
	SSLReadHolder.java \
	SSL.java \
	SSLUtil.java \
	SSLCTX.java \
	SSLServer.java \
	SSLClient.java

OBJ=axTLSj_wrap.o

JAVA_CLASSES:=$(JAVA_FILES:%.java=classes/axTLSj/%.class)

ifdef CONFIG_PLATFORM_WIN32
LDFLAGS += axtls.lib /libpath:"$(AXTLS_HOME)/$(STAGE)"

include $(AXTLS_HOME)/config/makefile.post

$(TARGET) : $(OBJ)
	$(LD) $(LDFLAGS) $(LDSHARED) /out:$@ $(OBJ)
else    # Not Win32

$(TARGET) : $(OBJ)
	$(LD) $(LDFLAGS) -L $(AXTLS_HOME)/$(STAGE) $(LDSHARED) -o $@ $(OBJ) -laxtls 
endif

jar: $(OBJ) $(JAR)

# if we are doing the samples then defer creating the jar until then
$(JAR): $(JAVA_CLASSES)
ifndef CONFIG_JAVA_SAMPLES
	jar cvf $@ -C classes axTLSj
else
	@if [ ! -f $(JAR) ]; then touch $(JAR); fi
endif

classes/axTLSj/%.class : %.java
	javac -d classes -classpath classes $^

clean::
	@rm -f $(JAR) $(TARGET) SWIG* axtls* *.i *.c 
	@rm -fr classes/*

