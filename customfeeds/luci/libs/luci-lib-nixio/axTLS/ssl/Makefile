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

AXTLS_HOME=..

include $(AXTLS_HOME)/config/.config
include $(AXTLS_HOME)/config/makefile.conf

all: libs
ifdef CONFIG_PERFORMANCE_TESTING
	$(MAKE) -C test
else
ifdef CONFIG_SSL_TEST
	$(MAKE) -C test
endif
endif

ifndef CONFIG_PLATFORM_WIN32
TARGET1=$(AXTLS_HOME)/$(STAGE)/libaxtls.a
BASETARGET=libaxtls.so
CRYPTO_PATH=$(AXTLS_HOME)/crypto/
ifdef CONFIG_PLATFORM_CYGWIN
TARGET2=$(AXTLS_HOME)/$(STAGE)/libaxtls.dll.a
else
TARGET2=$(AXTLS_HOME)/$(STAGE)/$(LIBMINOR)
endif

# shared library major/minor numbers
LIBMAJOR=$(BASETARGET).1
LIBMINOR=$(BASETARGET).1.2
else
TARGET1=$(AXTLS_HOME)/$(STAGE)/axtls.lib
TARGET2=$(AXTLS_HOME)/$(STAGE)/axtls.dll
STATIC_LIB=$(AXTLS_HOME)/$(STAGE)/axtls.static.lib
CRYPTO_PATH=$(AXTLS_HOME)\\crypto\\
endif

libs: $(TARGET1) $(TARGET2)

CRYPTO_OBJ=\
	$(CRYPTO_PATH)aes.o \
	$(CRYPTO_PATH)bigint.o \
	$(CRYPTO_PATH)crypto_misc.o \
	$(CRYPTO_PATH)hmac.o \
	$(CRYPTO_PATH)md2.o \
	$(CRYPTO_PATH)md5.o \
	$(CRYPTO_PATH)rc4.o \
	$(CRYPTO_PATH)rsa.o \
	$(CRYPTO_PATH)sha1.o

OBJ=\
	asn1.o \
	gen_cert.o \
	loader.o \
	openssl.o \
	os_port.o \
	p12.o \
	tls1.o \
	tls1_svr.o \
	tls1_clnt.o \
	x509.o

include $(AXTLS_HOME)/config/makefile.post

ifndef CONFIG_PLATFORM_WIN32     # Linux/Unix/Cygwin

$(TARGET1) : $(OBJ)
	$(AR) -r $@ $(CRYPTO_OBJ) $(OBJ)

$(TARGET2) : $(OBJ)
ifndef CONFIG_PLATFORM_CYGWIN
	$(LD) $(LDFLAGS) $(LDSHARED) -o $(AXTLS_HOME)/$(STAGE)/$(LIBMINOR) $(CRYPTO_OBJ) $(OBJ)
	cd $(AXTLS_HOME)/$(STAGE); ln -sf $(LIBMINOR) $(LIBMAJOR); ln -sf $(LIBMAJOR) $(BASETARGET); cd -
else
	$(LD) $(LDFLAGS) $(LDSHARED) -o $(AXTLS_HOME)/$(STAGE)/cygaxtls.dll \
    -Wl,--out-implib=$(AXTLS_HOME)/$(STAGE)/libaxtls.dll.a \
    -Wl,--export-all-symbols \
    -Wl,--enable-auto-import $(CRYPTO_OBJ) $(OBJ)
endif

else  # Win32
CRYPTO_OBJ:=$(CRYPTO_OBJ:.o=.obj)

$(TARGET1) : $(OBJ)
	$(AR) /out:$@ $(CRYPTO_OBJ) $(OBJ)

$(TARGET2) : $(OBJ)
	cp $(TARGET1) $(STATIC_LIB)
	$(LD) $(LDFLAGS) $(LDSHARED) /out:$@ $(CRYPTO_OBJ) $(OBJ)

endif    

clean::
	$(MAKE) -C test clean
	-@rm -f $(AXTLS_HOME)/$(STAGE)/* *.a $(TARGET1) $(TARGET2)

