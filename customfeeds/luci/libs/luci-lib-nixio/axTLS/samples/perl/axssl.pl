#!/usr/bin/perl -w
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
# Demonstrate the use of the axTLS library in Perl with a set of 
# command-line parameters similar to openssl. In fact, openssl clients 
# should be able to communicate with axTLS servers and visa-versa.
#
# This code has various bits enabled depending on the configuration. To enable
# the most interesting version, compile with the 'full mode' enabled.
#
# To see what options you have, run the following:
# > [perl] axssl s_server -?
# > [perl] axssl s_client -?
#
# The axtls/axtlsp shared libraries must be in the same directory or be found 
# by the OS. axtlsp.pm must be in this directory or be in @INC.
#
# Under Win32, ActivePerl was used (see
# http://www.activestate.com/Products/ActivePerl/?mp=1)
#
use axtlsp;
use IO::Socket;

# To get access to Win32 file descriptor stuff
my $is_win32 = 0;

if ($^O eq "MSWin32")
{
    eval("use Win32API::File 0.08 qw( :ALL )");
    $is_win32 = 1;
}

use strict;

#
# Win32 has some problems with socket handles
#
sub get_native_sock
{
    my ($sock) = @_;
    return $is_win32 ? FdGetOsFHandle($sock) : $sock;
}

# print version?
if ($#ARGV == 0 && $ARGV[0] eq "version")
{
    printf("axssl.pl ".axtlsp::ssl_version()."\n");
    exit 0;
}

#
# Main entry point. Doesn't do much except works out whether we are a client
# or a server.
#
print_options($#ARGV > -1 ? $ARGV[0] : "")
        if ($#ARGV < 0 || ($ARGV[0] ne "s_server" && $ARGV[0] ne "s_client"));


# Cygwin/Win32 issue - flush our output continuously
select STDOUT;
local $|=1;

my $build_mode = axtlsp::ssl_get_config($axtlsp::SSL_BUILD_MODE);
$ARGV[0] eq "s_server" ? do_server($build_mode) : do_client($build_mode);

#
# Implement the SSL server logic. 
#
sub do_server
{
    my ($build_mode) = @_;
    my $i = 1;
    my $port = 4433;
    my $options = $axtlsp::SSL_DISPLAY_CERTS;
    my $quiet = 0;
    my $password = undef;
    my $private_key_file = undef;
    my $cert_size = axtlsp::ssl_get_config($axtlsp::SSL_MAX_CERT_CFG_OFFSET);
    my $ca_cert_size = axtlsp::ssl_get_config(
                        $axtlsp::SSL_MAX_CA_CERT_CFG_OFFSET);
    my @cert;
    my @ca_cert;

    while ($i <= $#ARGV)
    {
        if ($ARGV[$i] eq  "-accept")
        {
            print_server_options($build_mode, $ARGV[$i]) if $i >= $#ARGV;
            $port = $ARGV[++$i];
        }
        elsif ($ARGV[$i] eq "-quiet")
        {
            $quiet = 1;
            $options &= ~$axtlsp::SSL_DISPLAY_CERTS;
        }
        elsif ($build_mode >= $axtlsp::SSL_BUILD_SERVER_ONLY)
        {
            if ($ARGV[$i] eq "-cert")
            {
                print_server_options($build_mode, $ARGV[$i]) 
                                    if $i >= $#ARGV || $#cert >= $cert_size-1;

                push @cert,  $ARGV[++$i];
            }
            elsif ($ARGV[$i] eq "-key")
            {
                print_server_options($build_mode, $ARGV[$i]) if $i >= $#ARGV;
                $private_key_file = $ARGV[++$i];
                $options |= $axtlsp::SSL_NO_DEFAULT_KEY;
            }
            elsif ($ARGV[$i] eq "-pass")
            {
                print_server_options($build_mode, $ARGV[$i]) if $i >= $#ARGV;
                $password = $ARGV[++$i];
            }
            elsif ($build_mode >= $axtlsp::SSL_BUILD_ENABLE_VERIFICATION)
            {
                if ($ARGV[$i] eq "-verify")
                {
                    $options |= $axtlsp::SSL_CLIENT_AUTHENTICATION;
                }
                elsif ($ARGV[$i] eq "-CAfile")
                {
                    print_server_options($build_mode, $ARGV[$i])  
                                if $i >= $#ARGV || $#ca_cert >= $ca_cert_size-1;
                    push @ca_cert, $ARGV[++$i];
                }
                elsif ($build_mode == $axtlsp::SSL_BUILD_FULL_MODE)
                {
                    if ($ARGV[$i] eq "-debug")
                    {
                        $options |= $axtlsp::SSL_DISPLAY_BYTES;
                    }
                    elsif ($ARGV[$i] eq "-state")
                    {
                        $options |= $axtlsp::SSL_DISPLAY_STATES;
                    }
                    elsif ($ARGV[$i] eq "-show-rsa")
                    {
                        $options |= $axtlsp::SSL_DISPLAY_RSA;
                    }
                    else
                    {
                        print_server_options($build_mode, $ARGV[$i]);
                    }
                }
                else
                {
                    print_server_options($build_mode, $ARGV[$i]);
                }
            }
            else 
            {
                print_server_options($build_mode, $ARGV[$i]);
            }
        }
        else 
        {
            print_server_options($build_mode, $ARGV[$i]);
        }

        $i++;
    }

    # Create socket for incoming connections
    my $server_sock = IO::Socket::INET->new(Proto => 'tcp',
                              LocalPort => $port,
                              Listen => 1,
                              Reuse => 1) or die $!;

    ###########################################################################
    # This is where the interesting stuff happens. Up until now we've
    # just been setting up sockets etc. Now we do the SSL handshake.
    ###########################################################################
    my $ssl_ctx = axtlsp::ssl_ctx_new($options, $axtlsp::SSL_DEFAULT_SVR_SESS);
    die "Error: Server context is invalid" if not defined $ssl_ctx;

    if (defined $private_key_file)
    {
        my $obj_type = $axtlsp::SSL_OBJ_RSA_KEY;

        $obj_type = $axtlsp::SSL_OBJ_PKCS8 if $private_key_file =~ /.p8$/;
        $obj_type = $axtlsp::SSL_OBJ_PKCS12 if $private_key_file =~ /.p12$/;

        die "Private key '$private_key_file' is undefined." if 
                axtlsp::ssl_obj_load($ssl_ctx, $obj_type,
                        $private_key_file, $password);
    }

    foreach (@cert)
    {
        die "Certificate '$_' is undefined." 
            if axtlsp::ssl_obj_load($ssl_ctx, $axtlsp::SSL_OBJ_X509_CERT, 
                    $_, undef) != $axtlsp::SSL_OK;
    }

    foreach (@ca_cert)
    {
        die "Certificate '$_' is undefined." 
            if axtlsp::ssl_obj_load($ssl_ctx, $axtlsp::SSL_OBJ_X509_CACERT, 
                    $_, undef) != $axtlsp::SSL_OK;
    }

    for (;;)
    {
        printf("ACCEPT\n") if not $quiet;
        my $client_sock = $server_sock->accept;
        my $native_sock = get_native_sock($client_sock->fileno);

        # This doesn't work in Win32 - need to get file descriptor from socket.
        my $ssl = axtlsp::ssl_server_new($ssl_ctx, $native_sock);

        # do the actual SSL handshake
        my $res;
        my $buf;
        my $connected = 0;

        while (1)
        {
            ($res, $buf) = axtlsp::ssl_read($ssl, undef);
            last if $res < $axtlsp::SSL_OK;

            if ($res == $axtlsp::SSL_OK) # connection established and ok
            {
                if (axtlsp::ssl_handshake_status($ssl) == $axtlsp::SSL_OK)
                {
                    if (!$quiet && !$connected)
                    {
                        display_session_id($ssl);
                        display_cipher($ssl);
                    }

                    $connected = 1;
                }
            }

            if ($res > $axtlsp::SSL_OK)
            {
                printf($$buf);
            }
            elsif ($res < $axtlsp::SSL_OK)
            {
                axtlsp::ssl_display_error($res) if not $quiet;
                last;
            } 
        }

        # client was disconnected or the handshake failed.
        printf("CONNECTION CLOSED\n") if not $quiet;
        axtlsp::ssl_free($ssl);
        $client_sock->close;
    }

    axtlsp::ssl_ctx_free($ssl_ctx);
}

#
# Implement the SSL client logic.
#
sub do_client
{
    my ($build_mode) = @_;
    my $i = 1;
    my $port = 4433;
    my $options = $axtlsp::SSL_SERVER_VERIFY_LATER|$axtlsp::SSL_DISPLAY_CERTS;
    my $private_key_file = undef;
    my $reconnect = 0;
    my $quiet = 0;
    my $password = undef;
    my @session_id;
    my $host = "127.0.0.1";
    my @cert;
    my @ca_cert;
    my $cert_size = axtlsp::ssl_get_config(
            $axtlsp::SSL_MAX_CERT_CFG_OFFSET);
    my $ca_cert_size = axtlsp::ssl_get_config(
            $axtlsp::SSL_MAX_CA_CERT_CFG_OFFSET);

    while ($i <= $#ARGV)
    {
        if ($ARGV[$i] eq "-connect")
        {
            print_client_options($build_mode, $ARGV[$i]) if $i >= $#ARGV;
            ($host, $port) = split(':', $ARGV[++$i]);
        }
        elsif ($ARGV[$i] eq "-cert")
        {
            print_client_options($build_mode, $ARGV[$i]) 
                if $i >= $#ARGV || $#cert >= $cert_size-1;

            push @cert, $ARGV[++$i];
        }
        elsif ($ARGV[$i] eq "-key")
        {
            print_client_options($build_mode, $ARGV[$i]) if $i >= $#ARGV;
            $private_key_file = $ARGV[++$i];
            $options |= $axtlsp::SSL_NO_DEFAULT_KEY;
        }
        elsif ($ARGV[$i] eq "-CAfile")
        {
            print_client_options($build_mode, $ARGV[$i]) 
                if $i >= $#ARGV || $#ca_cert >= $ca_cert_size-1;

            push @ca_cert, $ARGV[++$i];
        }
        elsif ($ARGV[$i] eq "-verify")
        {
            $options &= ~$axtlsp::SSL_SERVER_VERIFY_LATER;
        }
        elsif ($ARGV[$i] eq "-reconnect")
        {
            $reconnect = 4;
        }
        elsif ($ARGV[$i] eq "-quiet")
        {
            $quiet = 1;
            $options &= ~$axtlsp::SSL_DISPLAY_CERTS;
        }
        elsif ($ARGV[$i] eq "-pass")
        {
            print_server_options($build_mode, $ARGV[$i]) if $i >= $#ARGV;
            $password = $ARGV[++$i];
        }
        elsif ($build_mode == $axtlsp::SSL_BUILD_FULL_MODE)
        {
            if ($ARGV[$i] eq "-debug")
            {
                $options |= $axtlsp::SSL_DISPLAY_BYTES;
            }
            elsif ($ARGV[$i] eq "-state")
            {
                $options |= $axtlsp::SSL_DISPLAY_STATES;
            }
            elsif ($ARGV[$i] eq "-show-rsa")
            {
                $options |= $axtlsp::SSL_DISPLAY_RSA;
            }
            else    # don't know what this is
            {
                print_client_options($build_mode, $ARGV[$i]);
            }
        }
        else    # don't know what this is
        {
            print_client_options($build_mode, $ARGV[$i]);
        }

        $i++;
    }

    my $client_sock = new IO::Socket::INET (
                        PeerAddr => $host, PeerPort => $port, Proto => 'tcp')
                    || die ("no socket: $!");
    my $ssl;
    my $res;
    my $native_sock = get_native_sock($client_sock->fileno);

    printf("CONNECTED\n") if not $quiet;

    ###########################################################################
    # This is where the interesting stuff happens. Up until now we've
    # just been setting up sockets etc. Now we do the SSL handshake.
    ###########################################################################
    my $ssl_ctx = axtlsp::ssl_ctx_new($options, $axtlsp::SSL_DEFAULT_CLNT_SESS);
    die "Error: Client context is invalid" if not defined $ssl_ctx;

    if (defined $private_key_file)
    {
        my $obj_type = $axtlsp::SSL_OBJ_RSA_KEY;

        $obj_type = $axtlsp::SSL_OBJ_PKCS8 if $private_key_file =~ /.p8$/;
        $obj_type = $axtlsp::SSL_OBJ_PKCS12 if $private_key_file =~ /.p12$/;

        die "Private key '$private_key_file' is undefined." if 
                axtlsp::ssl_obj_load($ssl_ctx, $obj_type,
                        $private_key_file, $password);
    }

    foreach (@cert)
    {
        die "Certificate '$_' is undefined." 
            if axtlsp::ssl_obj_load($ssl_ctx, $axtlsp::SSL_OBJ_X509_CERT, 
                    $_, undef) != $axtlsp::SSL_OK;
    }

    foreach (@ca_cert)
    {
        die "Certificate '$_' is undefined." 
            if axtlsp::ssl_obj_load($ssl_ctx, $axtlsp::SSL_OBJ_X509_CACERT, 
                    $_, undef) != $axtlsp::SSL_OK;
    }

    # Try session resumption?
    if ($reconnect)
    {
        my $session_id = undef;
        my $sess_id_size = 0;

        while ($reconnect--)
        {
            $ssl = axtlsp::ssl_client_new($ssl_ctx, $native_sock, 
                            $session_id, $sess_id_size);

            $res = axtlsp::ssl_handshake_status($ssl);
            if ($res != $axtlsp::SSL_OK)
            {
                axtlsp::ssl_display_error($res) if !$quiet;
                axtlsp::ssl_free($ssl);
                exit 1;
            }

            display_session_id($ssl);
            $session_id = axtlsp::ssl_get_session_id($ssl);

            if ($reconnect)
            {
                axtlsp::ssl_free($ssl);
                $client_sock->close;
                $client_sock = new IO::Socket::INET (
                        PeerAddr => $host, PeerPort => $port, Proto => 'tcp')
                    || die ("no socket: $!");

            }
        }
    }
    else
    {
        $ssl = axtlsp::ssl_client_new($ssl_ctx, $native_sock, undef, 0);
    }

    # check the return status
    $res = axtlsp::ssl_handshake_status($ssl);
    if ($res != $axtlsp::SSL_OK)
    {
        axtlsp::ssl_display_error($res) if not $quiet;
        exit 1;
    }

    if (!$quiet)
    {
        my $common_name = axtlsp::ssl_get_cert_dn($ssl, 
                    $axtlsp::SSL_X509_CERT_COMMON_NAME);

        printf("Common Name:\t\t\t%s\n", $common_name) if defined $common_name;
        display_session_id($ssl);
        display_cipher($ssl);
    }

    while (<STDIN>)
    {
        my $cstring = pack("a*x", $_);   # add null terminator
        $res = axtlsp::ssl_write($ssl, \$cstring, length($cstring));
        if ($res < $axtlsp::SSL_OK)
        {
            axtlsp::ssl_display_error($res) if not $quiet;
            last;
        }
    }

    axtlsp::ssl_ctx_free($ssl_ctx);
    $client_sock->close;
}

#
# We've had some sort of command-line error. Print out the basic options.
#
sub print_options
{
    my ($option) = @_;
    printf("axssl: Error: '%s' is an invalid command.\n", $option);
    printf("usage: axssl [s_server|s_client|version] [args ...]\n");
    exit 1;
}

#
# We've had some sort of command-line error. Print out the server options.
#
sub print_server_options
{
    my ($build_mode, $option) = @_;
    my $cert_size = axtlsp::ssl_get_config($axtlsp::SSL_MAX_CERT_CFG_OFFSET);
    my $ca_cert_size = axtlsp::ssl_get_config(
            $axtlsp::SSL_MAX_CA_CERT_CFG_OFFSET);

    printf("unknown option %s\n", $option);
    printf("usage: s_server [args ...]\n");
    printf(" -accept arg\t- port to accept on (default is 4433)\n");
    printf(" -quiet\t\t- No server output\n");

    if ($build_mode >= $axtlsp::SSL_BUILD_SERVER_ONLY)
    {
        printf(" -cert arg\t- certificate file to add (in addition to default)".
                                        " to chain -\n".
          "\t\t  Can repeat up to %d times\n", $cert_size);
        printf(" -key arg\t- Private key file to use - default DER format\n");
        printf(" -pass\t\t- private key file pass phrase source\n");
    }

    if ($build_mode >= $axtlsp::SSL_BUILD_ENABLE_VERIFICATION)
    {
        printf(" -verify\t- turn on peer certificate verification\n");
        printf(" -CAfile arg\t- Certificate authority - default DER format\n");
        printf("\t\t  Can repeat up to %d times\n", $ca_cert_size);
    }

    if ($build_mode == $axtlsp::SSL_BUILD_FULL_MODE)
    {
        printf(" -debug\t\t- Print more output\n");
        printf(" -state\t\t- Show state messages\n");
        printf(" -show-rsa\t- Show RSA state\n");
    }

    exit 1;
}

#
# We've had some sort of command-line error. Print out the client options.
#
sub print_client_options
{
    my ($build_mode, $option) = @_;
    my $cert_size = axtlsp::ssl_get_config($axtlsp::SSL_MAX_CERT_CFG_OFFSET);
    my $ca_cert_size = axtlsp::ssl_get_config(
            $axtlsp::SSL_MAX_CA_CERT_CFG_OFFSET);

    printf("unknown option %s\n", $option);

    if ($build_mode >= $axtlsp::SSL_BUILD_ENABLE_CLIENT)
    {
        printf("usage: s_client [args ...]\n");
        printf(" -connect host:port - who to connect to (default ".
                "is localhost:4433)\n");
        printf(" -verify\t- turn on peer certificate verification\n");
        printf(" -cert arg\t- certificate file to use - default DER format\n");
        printf(" -key arg\t- Private key file to use - default DER format\n");
        printf("\t\t  Can repeat up to %d times\n", $cert_size);
        printf(" -CAfile arg\t- Certificate authority - default DER format\n");
        printf("\t\t  Can repeat up to %d times\n", $ca_cert_size);
        printf(" -quiet\t\t- No client output\n");
        printf(" -pass\t\t- private key file pass phrase source\n");
        printf(" -reconnect\t- Drop and re-make the connection ".
                "with the same Session-ID\n");

        if ($build_mode == $axtlsp::SSL_BUILD_FULL_MODE)
        {
            printf(" -debug\t\t- Print more output\n");
            printf(" -state\t\t- Show state messages\n");
            printf(" -show-rsa\t- Show RSA state\n");
        }
    }
    else
    {
        printf("Change configuration to allow this feature\n");
    }

    exit 1;
}

#
# Display what cipher we are using 
#
sub display_cipher
{
    my ($ssl) = @_;
    printf("CIPHER is ");
    my $cipher_id = axtlsp::ssl_get_cipher_id($ssl);

    if ($cipher_id == $axtlsp::SSL_AES128_SHA)
    {
        printf("AES128-SHA");
    }
    elsif ($cipher_id == $axtlsp::SSL_AES256_SHA)
    {
        printf("AES256-SHA");
    }
    elsif ($axtlsp::SSL_RC4_128_SHA)
    {
        printf("RC4-SHA");
    }
    elsif ($axtlsp::SSL_RC4_128_MD5)
    {
        printf("RC4-MD5");
    }
    else 
    {
        printf("Unknown - %d", $cipher_id);
    }

    printf("\n");
}

#
# Display what session id we have.
#
sub display_session_id
{    
    my ($ssl) = @_;
    my $session_id = axtlsp::ssl_get_session_id($ssl);
    if (length($$session_id) > 0)
    {
        printf("-----BEGIN SSL SESSION PARAMETERS-----\n");
        printf(unpack("H*", $$session_id));
        printf("\n-----END SSL SESSION PARAMETERS-----\n");
    }
}
