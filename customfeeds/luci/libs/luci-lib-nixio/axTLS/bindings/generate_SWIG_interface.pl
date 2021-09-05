#!/usr/bin/perl

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

#===============================================================
# Transforms function signature into SWIG format
sub transformSignature
{
    foreach $item (@_)
    { 
        $line =~ s/STDCALL //g;
        $line =~ s/EXP_FUNC/extern/g;

        # make API Java more 'byte' friendly
        $line =~ s/uint32_t/int/g;
        $line =~ s/const uint8_t \* /const unsigned char \* /g;
        $line =~ s/\(void\)/()/g;
        if ($ARGV[0] eq "-java")
        {
            $line =~ s/.*ssl_read.*//g;
            $line =~ s/const uint8_t \*(\w+)/const signed char $1\[\]/g;
            $line =~ s/uint8_t/signed char/g;
        }
        elsif ($ARGV[0] eq "-perl")
        {
            $line =~ s/const uint8_t \*(\w+)/const unsigned char $1\[\]/g;
            $line =~ s/uint8_t/unsigned char/g;
        }
        else # lua
        {
            $line =~ s/const uint8_t \*session_id/const unsigned char session_id\[\]/g;
            $line =~ s/const uint8_t \*\w+/unsigned char *INPUT/g;
            $line =~ s/uint8_t/unsigned char/g;
        }
    }

    return $line;
}

# Parse input file
sub parseFile
{
    foreach $line (@_)
    {
        next if $line =~ /ssl_x509_create/; # ignore for now

        # test for a #define
        if (!$skip && $line =~ m/^#define/)
        {
            $splitDefine = 1 if $line =~ m/\\$/;
            print DATA_OUT $line;

            # check line is not split
            next if $splitDefine == 1;
        }

        # pick up second line of #define statement
        if ($splitDefine) 
        {
            print DATA_OUT $line;

            # check line is not split
            $splitDefine = ($line =~ m/\\$/);
            next;
        } 

        # test for function declaration
        if (!$skip && $line =~ /EXP_FUNC/ && $line !~/\/\*/)
        {
            $line = transformSignature($line);
            $splitFunctionDeclaration = $line !~ /;/;
            print DATA_OUT $line;
            next;
        }

        if ($splitFunctionDeclaration) 
        {
            $line = transformSignature($line);
            $splitFunctionDeclaration = $line !~ /;/;
            print DATA_OUT $line;
            next;
        }
    }
}

#===============================================================

# Determine which module to build from cammand-line options
use strict;
use Getopt::Std;

my $module;
my $interfaceFile;
my $data_file;
my $skip;
my $splitLine;
my @raw_data;

if (not defined  $ARGV[0])
{
    goto ouch;
}

if ($ARGV[0] eq "-java")
{
    print "Generating Java interface file\n";
    $module = "axtlsj";
    $interfaceFile = "java/axTLSj.i";
}
elsif ($ARGV[0] eq "-perl")
{
    print "Generating Perl interface file\n";
    $module = "axtlsp";
    $interfaceFile = "perl/axTLSp.i";
}
elsif ($ARGV[0] eq "-lua")
{
    print "Generating lua interface file\n";
    $module = "axtlsl";
    $interfaceFile = "lua/axTLSl.i";
}
else
{
ouch:
    die "Usage: $0 [-java | -perl | -lua]\n";
}

# Input file required to generate SWIG interface file.
$data_file = "../ssl/ssl.h";

# Open input files
open(DATA_IN, $data_file) || die("Could not open file ($data_file)!");
@raw_data = <DATA_IN>;

# Open output file
open(DATA_OUT, ">$interfaceFile") || die("Cannot Open File");

#
# I wish I could say it was easy to generate the Perl/Java/Lua bindings, 
# but each had their own set of challenges... :-(.
#
print DATA_OUT << "END";
%module $module\n

/* include our own header */
%inline %{
#include "ssl.h"
%}

%include "typemaps.i"
/* Some SWIG magic to make the API a bit more Java friendly */
#ifdef SWIGJAVA

%apply long { SSL * };
%apply long { SSL_CTX * };
%apply long { SSLObjLoader * };

/* allow "unsigned char []" to become "byte[]" */
%include "arrays_java.i"

/* convert these pointers to use long */
%apply signed char[] {unsigned char *};
%apply signed char[] {signed char *};

/* allow ssl_get_session_id() to return "byte[]" */
%typemap(out) unsigned char * ssl_get_session_id \"if (result) jresult = SWIG_JavaArrayOutSchar(jenv, result, ssl_get_session_id_size((SSL const *)arg1));\"

/* allow ssl_client_new() to have a null session_id input */
%typemap(in) const signed char session_id[] (jbyte *jarr) {
    if (jarg3 == NULL)
    {
        jresult = (jint)ssl_client_new(arg1,arg2,NULL,0);
        return jresult;
    }
    
    if (!SWIG_JavaArrayInSchar(jenv, &jarr, &arg3, jarg3)) return 0;
}   

/* Lot's of work required for an ssl_read() due to its various custom
 * requirements.
 */
%native (ssl_read) int ssl_read(SSL *ssl, jobject in_data);
%{
JNIEXPORT jint JNICALL Java_axTLSj_axtlsjJNI_ssl_1read(JNIEnv *jenv, jclass jcls, jint jarg1, jobject jarg2) {
    jint jresult = 0 ;
    SSL *arg1;
    unsigned char *arg2;
    jbyte *jarr;
    int result;
    JNIEnv e = *jenv;
    jclass holder_class;
    jfieldID fid;

    arg1 = (SSL *)jarg1;
    result = (int)ssl_read(arg1, &arg2);

    /* find the "m_buf" entry in the SSLReadHolder class */
    if (!(holder_class = e->GetObjectClass(jenv,jarg2)) ||
            !(fid = e->GetFieldID(jenv,holder_class, "m_buf", "[B")))
        return SSL_NOT_OK;

    if (result > SSL_OK)
    {
        int i;

        /* create a new byte array to hold the read data */
        jbyteArray jarray = e->NewByteArray(jenv, result);

        /* copy the bytes across to the java byte array */
        jarr = e->GetByteArrayElements(jenv, jarray, 0);
        for (i = 0; i < result; i++)
            jarr[i] = (jbyte)arg2[i];

        /* clean up and set the new m_buf object */
        e->ReleaseByteArrayElements(jenv, jarray, jarr, 0);
        e->SetObjectField(jenv, jarg2, fid, jarray);
    }
    else    /* set to null */
        e->SetObjectField(jenv, jarg2, fid, NULL);

    jresult = (jint)result;
    return jresult;
}
%}

/* Big hack to get hold of a socket's file descriptor */
%typemap (jtype) long "Object"
%typemap (jstype) long "Object"
%native (getFd) int getFd(long sock);
%{
JNIEXPORT jint JNICALL Java_axTLSj_axtlsjJNI_getFd(JNIEnv *env, jclass jcls, jobject sock)
{
    JNIEnv e = *env;
    jfieldID fid;
    jobject impl;
    jobject fdesc;

    /* get the SocketImpl from the Socket */
    if (!(jcls = e->GetObjectClass(env,sock)) ||
            !(fid = e->GetFieldID(env,jcls,"impl","Ljava/net/SocketImpl;")) ||
            !(impl = e->GetObjectField(env,sock,fid))) return -1;

    /* get the FileDescriptor from the SocketImpl */
    if (!(jcls = e->GetObjectClass(env,impl)) ||
            !(fid = e->GetFieldID(env,jcls,"fd","Ljava/io/FileDescriptor;")) ||
            !(fdesc = e->GetObjectField(env,impl,fid))) return -1;

    /* get the fd from the FileDescriptor */
    if (!(jcls = e->GetObjectClass(env,fdesc)) ||
            !(fid = e->GetFieldID(env,jcls,"fd","I"))) return -1;

    /* return the descriptor */
    return e->GetIntField(env,fdesc,fid);
} 
%}

#endif

/* Some SWIG magic to make the API a bit more Perl friendly */
#ifdef SWIGPERL

/* for ssl_session_id() */
%typemap(out) const unsigned char * {
    SV *svs = newSVpv((unsigned char *)\$1, ssl_get_session_id_size((SSL const *)arg1));
    \$result = newRV(svs);
    sv_2mortal(\$result);
    argvi++;
}

/* for ssl_write() */
%typemap(in) const unsigned char out_data[] {
    SV* tempsv;
    if (!SvROK(\$input))
        croak("Argument \$argnum is not a reference.");
    tempsv = SvRV(\$input);
    if (SvTYPE(tempsv) != SVt_PV)
        croak("Argument \$argnum is not an string.");
    \$1 = (unsigned char *)SvPV(tempsv, PL_na);
}

/* for ssl_read() */
%typemap(in) unsigned char **in_data (unsigned char *buf) {
    \$1 = &buf;
}

%typemap(argout) unsigned char **in_data { 
    if (result > SSL_OK) {
        SV *svs = newSVpv(*\$1, result);
        \$result = newRV(svs);
        sv_2mortal(\$result);
        argvi++;
    }
}

/* for ssl_client_new() */
%typemap(in) const unsigned char session_id[] {
    /* check for a reference */
    if (SvOK(\$input) && SvROK(\$input)) {
        SV* tempsv = SvRV(\$input);
        if (SvTYPE(tempsv) != SVt_PV)
            croak("Argument \$argnum is not an string.");
        \$1 = (unsigned char *)SvPV(tempsv, PL_na); 
    } 
    else
        \$1 = NULL;
}

#endif

/* Some SWIG magic to make the API a bit more Lua friendly */
#ifdef SWIGLUA
SWIG_NUMBER_TYPEMAP(unsigned char);
SWIG_TYPEMAP_NUM_ARR(uchar,unsigned char);

/* for ssl_session_id() */
%typemap(out) const unsigned char * {
    int i;
    lua_newtable(L);
    for (i = 0; i < ssl_get_session_id_size((SSL const *)arg1); i++){
        lua_pushnumber(L,(lua_Number)result[i]);
        lua_rawseti(L,-2,i+1); /* -1 is the number, -2 is the table */
    }
    SWIG_arg++;
}

/* for ssl_read() */
%typemap(in) unsigned char **in_data (unsigned char *buf) {
    \$1 = &buf;
}

%typemap(argout) unsigned char **in_data { 
    if (result > SSL_OK) {
		int i;
		lua_newtable(L);
		for (i = 0; i < result; i++){
			lua_pushnumber(L,(lua_Number)buf2[i]);
			lua_rawseti(L,-2,i+1); /* -1 is the number, -2 is the table */
		}
        SWIG_arg++;
    }
}

/* for ssl_client_new() */
%typemap(in) const unsigned char session_id[] {
    if (lua_isnil(L,\$input))
        \$1 = NULL;
    else
        \$1 = SWIG_get_uchar_num_array_fixed(L,\$input, ssl_get_session_id((SSL const *)\$1));
}

#endif

END

# Initialise loop variables
$skip = 1;
$splitLine = 0;

parseFile(@raw_data);

close(DATA_IN);
close(DATA_OUT);

#===============================================================

