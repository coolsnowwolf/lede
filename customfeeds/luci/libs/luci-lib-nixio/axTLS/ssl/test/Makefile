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

all:

AXTLS_HOME=../..

include $(AXTLS_HOME)/config/.config
include $(AXTLS_HOME)/config/makefile.conf

ifdef CONFIG_PERFORMANCE_TESTING
all: performance
endif

ifdef CONFIG_SSL_TEST
all: ssltesting
endif

include $(AXTLS_HOME)/config/makefile.post

ifndef CONFIG_PLATFORM_WIN32
performance: $(AXTLS_HOME)/$(STAGE)/perf_bigint
ssltesting: $(AXTLS_HOME)/$(STAGE)/ssltest
LIBS=$(AXTLS_HOME)/$(STAGE)

$(AXTLS_HOME)/$(STAGE)/perf_bigint: perf_bigint.o $(LIBS)/libaxtls.a
	$(CC) $(LDFLAGS) -o $@ $^ -L $(LIBS) -laxtls

$(AXTLS_HOME)/$(STAGE)/ssltest: ssltest.o $(LIBS)/libaxtls.a
	$(CC) $(LDFLAGS) -o $@ $^ -lpthread -L $(LIBS) -laxtls
else
performance: $(AXTLS_HOME)/$(STAGE)/perf_bigint.exe
ssltesting: $(AXTLS_HOME)/$(STAGE)/ssltest.exe

CRYPTO_PATH="$(AXTLS_INCLUDE)crypto\\"
AXTLS_SSL_PATH="$(AXTLS_INCLUDE)ssl\\"

CRYPTO_OBJ=\
	$(CRYPTO_PATH)aes.obj \
	$(CRYPTO_PATH)bigint.obj \
	$(CRYPTO_PATH)crypto_misc.obj \
	$(CRYPTO_PATH)hmac.obj \
	$(CRYPTO_PATH)md2.obj \
	$(CRYPTO_PATH)md5.obj \
	$(CRYPTO_PATH)rc4.obj \
	$(CRYPTO_PATH)rsa.obj \
	$(CRYPTO_PATH)sha1.obj

OBJ=\
	$(AXTLS_SSL_PATH)asn1.obj \
	$(AXTLS_SSL_PATH)gen_cert.obj \
	$(AXTLS_SSL_PATH)loader.obj \
	$(AXTLS_SSL_PATH)openssl.obj \
	$(AXTLS_SSL_PATH)os_port.obj \
	$(AXTLS_SSL_PATH)p12.obj \
	$(AXTLS_SSL_PATH)x509.obj \
	$(AXTLS_SSL_PATH)tls1.obj \
	$(AXTLS_SSL_PATH)tls1_svr.obj \
	$(AXTLS_SSL_PATH)tls1_clnt.obj

$(AXTLS_HOME)/$(STAGE)/perf_bigint.exe: perf_bigint.obj
	$(LD) $(LDFLAGS) /out:$@ $? $(CRYPTO_OBJ) $(OBJ)

$(AXTLS_HOME)/$(STAGE)/ssltest.exe: ssltest.obj
	$(LD) $(LDFLAGS) /out:$@ $? $(CRYPTO_OBJ) $(OBJ)
endif

clean::
	-@rm -f $(AXTLS_HOME)/$(STAGE)/perf_bigint* $(AXTLS_HOME)/$(STAGE)/ssltest*

