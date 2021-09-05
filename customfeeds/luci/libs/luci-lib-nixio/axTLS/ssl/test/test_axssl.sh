#!/bin/sh

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

#
# Test the various axssl bindings. To run it, got to the _install directory
# and run this script from there.
#

if grep "CONFIG_PLATFORM_WIN32=y" "../config/.config"  > /dev/null; then
    JAVA_EXE="$JAVA_HOME/bin/java.exe"
    PERL_BIN="/cygdrive/c/Perl/bin/perl"
    KILL_AXSSL="kill %1"
    KILL_CSHARP="kill %1"
    KILL_PERL="kill %1"
    KILL_JAVA="kill %1"
    KILL_LUA="kill %1"
else
    if grep "CONFIG_PLATFORM_CYGWIN=y" "../config/.config"  > /dev/null; then
        # no .net or java on cygwin
        PERL_BIN=/usr/bin/perl
        KILL_AXSSL="killall axssl"
        KILL_PERL="killall /usr/bin/perl"
        KILL_LUA="killall /usr/local/bin/lua"
    else     # Linux
        JAVA_EXE=/usr/java/default/bin/java
        PERL_BIN=/usr/bin/perl
        KILL_AXSSL="killall axssl"
        KILL_CSHARP="killall mono"
        KILL_PERL="killall /usr/bin/perl"
        RUN_CSHARP="mono"
        KILL_JAVA="killall $JAVA_EXE"
        KILL_LUA="killall /usr/local/bin/lua"
    fi
fi

BASE=..
SERVER_ARGS="s_server -accept 15001 -verify -CAfile $BASE/ssl/test/axTLS.ca_x509.cer"
CLIENT_ARGS="s_client -reconnect -connect localhost:15001 -verify -CAfile $BASE/ssl/test/axTLS.ca_x509.cer -key $BASE/ssl/test/axTLS.key_1024 -cert $BASE/ssl/test/axTLS.x509_1024.cer"

# check pem arguments
SERVER_PEM_ARGS="s_server -accept 15001 -pass abcd -key $BASE/ssl/test/axTLS.key_aes128.pem -cert $BASE/ssl/test/axTLS.x509_aes128.pem"
CLIENT_PEM_ARGS="s_client -connect localhost:15001 -CAfile $BASE/ssl/test/axTLS.ca_x509.pem -key $BASE/ssl/test/axTLS.key_1024.pem -cert $BASE/ssl/test/axTLS.x509_1024.pem"

export LD_LIBRARY_PATH=.:`perl -e 'use Config; print $Config{archlib};'`/CORE

if [ -x ./axssl ]; then
echo "############################# C SAMPLE ###########################"
./axssl $SERVER_ARGS &
echo "C Test passed" | ./axssl $CLIENT_ARGS
$KILL_AXSSL
sleep 1

./axssl $SERVER_PEM_ARGS &
echo "C Test passed" | ./axssl $CLIENT_PEM_ARGS
$KILL_AXSSL
sleep 1
echo "### C tests complete"
fi

if [ -f ./axtls.jar ]; then
echo "########################## JAVA SAMPLE ###########################"
"$JAVA_EXE" -jar ./axtls.jar $SERVER_ARGS &
echo "Java Test passed" | "$JAVA_EXE" -jar ./axtls.jar $CLIENT_ARGS
$KILL_JAVA
sleep 1

"$JAVA_EXE" -jar ./axtls.jar $SERVER_PEM_ARGS &
echo "Java Test passed" | "$JAVA_EXE" -jar ./axtls.jar $CLIENT_PEM_ARGS
$KILL_JAVA
sleep 1

echo "### Java tests complete"
fi

if [ -x ./axssl.csharp.exe ]; then
echo "############################ C# SAMPLE ###########################"
$RUN_CSHARP ./axssl.csharp.exe $SERVER_ARGS &
echo "C# Test passed" | $RUN_CSHARP ./axssl.csharp.exe $CLIENT_ARGS
$KILL_CSHARP
sleep 1

$RUN_CSHARP ./axssl.csharp.exe $SERVER_PEM_ARGS &
echo "C# Test passed" | $RUN_CSHARP ./axssl.csharp.exe $CLIENT_PEM_ARGS
$KILL_CSHARP
sleep 1

echo "### C# tests complete"
fi

if [ -x ./axssl.vbnet.exe ]; then
echo "######################## VB.NET SAMPLE ###########################"
echo $SERVER_ARGS
echo $CLIENT_ARGS
./axssl.vbnet $SERVER_ARGS &
echo "VB.NET Test passed" | ./axssl.vbnet.exe $CLIENT_ARGS
kill %1
sleep 1

./axssl.vbnet $SERVER_PEM_ARGS &
echo "VB.NET Test passed" | ./axssl.vbnet.exe $CLIENT_PEM_ARGS
kill %1
sleep 1
echo "### VB.NET tests complete"
fi

if [ -f ./axssl.pl ]; then
echo "########################## PERL SAMPLE ###########################"
"$PERL_BIN" ./axssl.pl $SERVER_ARGS &
echo "Perl Test passed" | "$PERL_BIN" ./axssl.pl $CLIENT_ARGS
$KILL_PERL
sleep 1

"$PERL_BIN" ./axssl.pl $SERVER_PEM_ARGS &
echo "Perl Test passed" | "$PERL_BIN" ./axssl.pl $CLIENT_PEM_ARGS
$KILL_PERL
sleep 1
echo "### Perl tests complete"
fi

if [ -f ./axssl.lua ]; then
echo "########################## LUA SAMPLE ###########################"
./axssl.lua $SERVER_ARGS &
echo "Lua Test passed" | ./axssl.lua $CLIENT_ARGS
$KILL_LUA
sleep 1

./axssl.lua $SERVER_PEM_ARGS &
echo "Lua Test passed" | ./axssl.lua $CLIENT_PEM_ARGS
$KILL_LUA
sleep 1
echo "### Lua tests complete"
fi

echo "########################## ALL TESTS COMPLETE ###########################"
