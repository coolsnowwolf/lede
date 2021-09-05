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

include ../config/.config
include ../config/makefile.conf

ifdef CONFIG_CSHARP_BINDINGS
all: csharp/axInterface.cs
endif

ifdef CONFIG_VBNET_BINDINGS
all: vbnet/axInterface.vb
endif

ifdef CONFIG_JAVA_BINDINGS
all: java/axtlsj.java
endif

ifdef CONFIG_PERL_BINDINGS
all: perl/axTLSp_wrap.c
endif

ifdef CONFIG_LUA_BINDINGS
all: lua/axTLSl_wrap.c
endif

csharp/axInterface.cs: ../ssl/ssl.h
	@perl ./generate_interface.pl -csharp

vbnet/axInterface.vb: ../ssl/ssl.h
	@perl ./generate_interface.pl -vbnet

java/axTLSj.i: ../ssl/ssl.h
	@perl ./generate_SWIG_interface.pl -java

java/axtlsj.java: java/axTLSj.i $(wildcard java/SSL*.java)
	@cd java; swig -java -package axTLSj axTLSj.i; $(MAKE)

perl/axTLSp.i: ../ssl/ssl.h
	@perl ./generate_SWIG_interface.pl -perl

perl/axTLSp_wrap.c: perl/axTLSp.i
	@cd perl; swig -perl5 axTLSp.i; $(MAKE)

lua/axTLSl.i: ../ssl/ssl.h
	@perl ./generate_SWIG_interface.pl -lua

lua/axTLSl_wrap.c: lua/axTLSl.i
	@cd lua; swig -lua axTLSl.i; $(MAKE)

clean::
	$(MAKE) -C csharp clean
	$(MAKE) -C vbnet clean
	$(MAKE) -C java clean
	$(MAKE) -C perl clean
	$(MAKE) -C lua clean

