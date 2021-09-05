#!/usr/local/bin/lua

--
-- Copyright (c) 2007, Cameron Rich
--
-- All rights reserved.
--
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions are met:
--
-- * Redistributions of source code must retain the above copyright notice,
--   this list of conditions and the following disclaimer.
-- * Redistributions in binary form must reproduce the above copyright
--   notice, this list of conditions and the following disclaimer in the
--   documentation and/or other materials provided with the distribution.
-- * Neither the name of the axTLS project nor the names of its
--   contributors may be used to endorse or promote products derived
--   from this software without specific prior written permission.
--
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
-- "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
-- LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
-- A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
-- CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
-- SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
-- TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
-- DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
-- OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
-- NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
-- THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--

--
-- Demonstrate the use of the axTLS library in Lua with a set of 
-- command-line parameters similar to openssl. In fact, openssl clients 
-- should be able to communicate with axTLS servers and visa-versa.
--
-- This code has various bits enabled depending on the configuration. To enable
-- the most interesting version, compile with the 'full mode' enabled.
--
-- To see what options you have, run the following:
-- > [lua] axssl s_server -?
-- > [lua] axssl s_client -?
--
-- The axtls/axtlsl shared libraries must be in the same directory or be found 
-- by the OS.
--
--
require "bit"
require("axtlsl")
local socket = require("socket")

-- print version?
if #arg == 1 and arg[1] == "version" then
    print("axssl.lua "..axtlsl.ssl_version())
    os.exit(1)
end

--
-- We've had some sort of command-line error. Print out the basic options.
--
function print_options(option)
    print("axssl: Error: '"..option.."' is an invalid command.")
    print("usage: axssl [s_server|s_client|version] [args ...]")
    os.exit(1)
end

--
-- We've had some sort of command-line error. Print out the server options.
--
function print_server_options(build_mode, option)
    local cert_size = axtlsl.ssl_get_config(axtlsl.SSL_MAX_CERT_CFG_OFFSET)
    local ca_cert_size = axtlsl.ssl_get_config(
            axtlsl.SSL_MAX_CA_CERT_CFG_OFFSET)

    print("unknown option "..option)
    print("usage: s_server [args ...]")
    print(" -accept\t- port to accept on (default is 4433)")
    print(" -quiet\t\t- No server output")

    if build_mode >= axtlsl.SSL_BUILD_SERVER_ONLY then
        print(" -cert arg\t- certificate file to add (in addition to "..
                "default) to chain -")
        print("\t\t  Can repeat up to "..cert_size.." times")
        print(" -key arg\t- Private key file to use - default DER format")
        print(" -pass\t\t- private key file pass phrase source")
    end

    if build_mode >= axtlsl.SSL_BUILD_ENABLE_VERIFICATION then
        print(" -verify\t- turn on peer certificate verification")
        print(" -CAfile arg\t- Certificate authority - default DER format")
        print("\t\t  Can repeat up to "..ca_cert_size.." times")
    end

    if build_mode == axtlsl.SSL_BUILD_FULL_MODE then
        print(" -debug\t\t- Print more output")
        print(" -state\t\t- Show state messages")
        print(" -show-rsa\t- Show RSA state")
    end

    os.exit(1)
end

--
-- We've had some sort of command-line error. Print out the client options.
--
function print_client_options(build_mode, option)
    local cert_size = axtlsl.ssl_get_config(axtlsl.SSL_MAX_CERT_CFG_OFFSET)
    local ca_cert_size = axtlsl.ssl_get_config(
            axtlsl.SSL_MAX_CA_CERT_CFG_OFFSET)

    print("unknown option "..option)

    if build_mode >= axtlsl.SSL_BUILD_ENABLE_CLIENT then
        print("usage: s_client [args ...]")
        print(" -connect host:port - who to connect to (default "..
                "is localhost:4433)")
        print(" -verify\t- turn on peer certificate verification")
        print(" -cert arg\t- certificate file to use - default DER format")
        print(" -key arg\t- Private key file to use - default DER format")
        print("\t\t  Can repeat up to "..cert_size.." times")
        print(" -CAfile arg\t- Certificate authority - default DER format")
        print("\t\t  Can repeat up to "..ca_cert_size.."times")
        print(" -quiet\t\t- No client output")
        print(" -pass\t\t- private key file pass phrase source")
        print(" -reconnect\t- Drop and re-make the connection "..
                "with the same Session-ID")

        if build_mode == axtlsl.SSL_BUILD_FULL_MODE then
            print(" -debug\t\t- Print more output")
            print(" -state\t\t- Show state messages")
            print(" -show-rsa\t- Show RSA state")
        end
    else
        print("Change configuration to allow this feature")
    end

    os.exit(1)
end

-- Implement the SSL server logic. 
function do_server(build_mode)
    local i = 2
    local v
    local port = 4433
    local options = axtlsl.SSL_DISPLAY_CERTS
    local quiet = false
    local password = ""
    local private_key_file = nil
    local cert_size = axtlsl.ssl_get_config(axtlsl.SSL_MAX_CERT_CFG_OFFSET)
    local ca_cert_size = axtlsl.
                            ssl_get_config(axtlsl.SSL_MAX_CA_CERT_CFG_OFFSET)
    local cert = {}
    local ca_cert = {}

    while i <= #arg do
        if arg[i] ==  "-accept" then
            if i >= #arg then
                print_server_options(build_mode, arg[i])
            end

            i = i + 1
            port = arg[i]
        elseif arg[i] == "-quiet" then
            quiet = true
            options = bit.band(options, bit.bnot(axtlsl.SSL_DISPLAY_CERTS))
        elseif build_mode >= axtlsl.SSL_BUILD_SERVER_ONLY then
            if arg[i] == "-cert" then
                if i >= #arg or #cert >= cert_size then
                    print_server_options(build_mode, arg[i]) 
                end

                i = i + 1
                table.insert(cert, arg[i])
            elseif arg[i] == "-key" then
                if i >= #arg then
                    print_server_options(build_mode, arg[i]) 
                end

                i = i + 1
                private_key_file = arg[i]
                options = bit.bor(options, axtlsl.SSL_NO_DEFAULT_KEY)
            elseif arg[i] == "-pass" then
                if i >= #arg then
                    print_server_options(build_mode, arg[i]) 
                end

                i = i + 1
                password = arg[i]
            elseif build_mode >= axtlsl.SSL_BUILD_ENABLE_VERIFICATION then
                if arg[i] == "-verify" then
                    options = bit.bor(options, axtlsl.SSL_CLIENT_AUTHENTICATION)
                elseif arg[i] == "-CAfile" then
                    if i >= #arg or #ca_cert >= ca_cert_size then
                        print_server_options(build_mode, arg[i])  
                    end

                    i = i + 1
                    table.insert(ca_cert, arg[i])
                elseif build_mode == axtlsl.SSL_BUILD_FULL_MODE then
                    if arg[i] == "-debug" then
                        options = bit.bor(options, axtlsl.SSL_DISPLAY_BYTES)
                    elseif arg[i] == "-state" then
                        options = bit.bor(options, axtlsl.SSL_DISPLAY_STATES)
                    elseif arg[i] == "-show-rsa" then
                        options = bit.bor(options, axtlsl.SSL_DISPLAY_RSA)
                    else
                        print_server_options(build_mode, arg[i])
                    end
                else
                    print_server_options(build_mode, arg[i])
                end
            else 
                print_server_options(build_mode, arg[i])
            end
        else 
            print_server_options(build_mode, arg[i])
        end

        i = i + 1
    end

    -- Create socket for incoming connections
    local server_sock = socket.try(socket.bind("*", port))

    ---------------------------------------------------------------------------
    -- This is where the interesting stuff happens. Up until now we've
    -- just been setting up sockets etc. Now we do the SSL handshake.
    ---------------------------------------------------------------------------
    local ssl_ctx = axtlsl.ssl_ctx_new(options, axtlsl.SSL_DEFAULT_SVR_SESS)
    if ssl_ctx == nil then error("Error: Server context is invalid") end

    if private_key_file ~= nil then
        local obj_type = axtlsl.SSL_OBJ_RSA_KEY

        if string.find(private_key_file, ".p8") then 
            obj_type = axtlsl.SSL_OBJ_PKCS8 
        end

        if string.find(private_key_file, ".p12") then 
            obj_type = axtlsl.SSL_OBJ_PKCS12 
        end

        if axtlsl.ssl_obj_load(ssl_ctx, obj_type, private_key_file, 
                                        password) ~= axtlsl.SSL_OK then
            error("Private key '" .. private_key_file .. "' is undefined.")
        end
    end

    for _, v in ipairs(cert) do
        if axtlsl.ssl_obj_load(ssl_ctx, axtlsl.SSL_OBJ_X509_CERT, v, "") ~= 
                                        axtlsl.SSL_OK then
            error("Certificate '"..v .. "' is undefined.")
        end
    end

    for _, v in ipairs(ca_cert) do
        if axtlsl.ssl_obj_load(ssl_ctx, axtlsl.SSL_OBJ_X509_CACERT, v, "") ~= 
                                        axtlsl.SSL_OK then
            error("Certificate '"..v .."' is undefined.")
        end
    end

    while true do
        if not quiet then print("ACCEPT") end
        local client_sock = server_sock:accept();
        local ssl = axtlsl.ssl_server_new(ssl_ctx, client_sock:getfd())

        -- do the actual SSL handshake
        local connected = false
        local res
        local buf

        while true do
            socket.select({client_sock}, nil)
            res, buf = axtlsl.ssl_read(ssl)

            if res == axtlsl.SSL_OK then -- connection established and ok
                if axtlsl.ssl_handshake_status(ssl) == axtlsl.SSL_OK then
                    if not quiet and not connected then
                        display_session_id(ssl)
                        display_cipher(ssl)
                    end
                    connected = true
                end
            end

            if res > axtlsl.SSL_OK then
                for _, v in ipairs(buf) do
                    io.write(string.format("%c", v))
                end
            elseif res < axtlsl.SSL_OK then 
                if not quiet then
                    axtlsl.ssl_display_error(res)
                end
                break
            end
        end

        -- client was disconnected or the handshake failed.
        print("CONNECTION CLOSED")
        axtlsl.ssl_free(ssl)
        client_sock:close()
    end

    axtlsl.ssl_ctx_free(ssl_ctx)
end

--
-- Implement the SSL client logic.
--
function do_client(build_mode)
    local i = 2
    local v
    local port = 4433
    local options = 
            bit.bor(axtlsl.SSL_SERVER_VERIFY_LATER, axtlsl.SSL_DISPLAY_CERTS)
    local private_key_file = nil
    local reconnect = 0
    local quiet = false
    local password = ""
    local session_id = {}
    local host = "127.0.0.1"
    local cert_size = axtlsl.ssl_get_config(axtlsl.SSL_MAX_CERT_CFG_OFFSET)
    local ca_cert_size = axtlsl.
                            ssl_get_config(axtlsl.SSL_MAX_CA_CERT_CFG_OFFSET)
    local cert = {}
    local ca_cert = {}

    while i <= #arg do
        if arg[i] == "-connect" then
            if i >= #arg then
                print_client_options(build_mode, arg[i]) 
            end

            i = i + 1
            local t = string.find(arg[i], ":")
            host = string.sub(arg[i], 1, t-1)
            port = string.sub(arg[i], t+1)
        elseif arg[i] == "-cert" then
            if i >= #arg or #cert >= cert_size then
                print_client_options(build_mode, arg[i]) 
            end

            i = i + 1
            table.insert(cert, arg[i])
        elseif arg[i] == "-key" then
            if i >= #arg then
                print_client_options(build_mode, arg[i])
            end

            i = i + 1
            private_key_file = arg[i]
            options = bit.bor(options, axtlsl.SSL_NO_DEFAULT_KEY)
        elseif arg[i] == "-CAfile" then
            if i >= #arg or #ca_cert >= ca_cert_size then
                print_client_options(build_mode, arg[i]) 
            end

            i = i + 1
            table.insert(ca_cert, arg[i])
        elseif arg[i] == "-verify" then
            options = bit.band(options, 
                                bit.bnot(axtlsl.SSL_SERVER_VERIFY_LATER))
        elseif arg[i] == "-reconnect" then
            reconnect = 4
        elseif arg[i] == "-quiet" then
            quiet = true
            options = bit.band(options, bnot(axtlsl.SSL_DISPLAY_CERTS))
        elseif arg[i] == "-pass" then
            if i >= #arg then
                print_server_options(build_mode, arg[i])
            end

            i = i + 1
            password = arg[i]
        elseif build_mode == axtlsl.SSL_BUILD_FULL_MODE then
            if arg[i] == "-debug" then
                options = bit.bor(options, axtlsl.SSL_DISPLAY_BYTES)
            elseif arg[i] == "-state" then
                options = bit.bor(axtlsl.SSL_DISPLAY_STATES)
            elseif arg[i] == "-show-rsa" then
                options = bit.bor(axtlsl.SSL_DISPLAY_RSA)
            else    -- don't know what this is
                print_client_options(build_mode, arg[i])
            end
        else    -- don't know what this is
            print_client_options(build_mode, arg[i])
        end

        i = i + 1
    end

    local client_sock = socket.try(socket.connect(host, port))
    local ssl
    local res

    if not quiet then print("CONNECTED") end

    ---------------------------------------------------------------------------
    -- This is where the interesting stuff happens. Up until now we've
    -- just been setting up sockets etc. Now we do the SSL handshake.
    ---------------------------------------------------------------------------
    local ssl_ctx = axtlsl.ssl_ctx_new(options, axtlsl.SSL_DEFAULT_CLNT_SESS)

    if ssl_ctx == nil then 
        error("Error: Client context is invalid")
    end

    if private_key_file ~= nil then
        local obj_type = axtlsl.SSL_OBJ_RSA_KEY

        if string.find(private_key_file, ".p8") then 
            obj_type = axtlsl.SSL_OBJ_PKCS8 
        end

        if string.find(private_key_file, ".p12") then 
            obj_type = axtlsl.SSL_OBJ_PKCS12 
        end

        if axtlsl.ssl_obj_load(ssl_ctx, obj_type, private_key_file, 
                                        password) ~= axtlsl.SSL_OK then
            error("Private key '"..private_key_file.."' is undefined.")
        end
    end

    for _, v in ipairs(cert) do
        if axtlsl.ssl_obj_load(ssl_ctx, axtlsl.SSL_OBJ_X509_CERT, v, "") ~= 
                                        axtlsl.SSL_OK then
            error("Certificate '"..v .. "' is undefined.")
        end
    end

    for _, v in ipairs(ca_cert) do
        if axtlsl.ssl_obj_load(ssl_ctx, axtlsl.SSL_OBJ_X509_CACERT, v, "") ~= 
                                        axtlsl.SSL_OK then
            error("Certificate '"..v .."' is undefined.")
        end
    end

    -- Try session resumption?
    if reconnect ~= 0 then
        local session_id = nil
        local sess_id_size = 0

        while reconnect > 0 do
            reconnect = reconnect - 1
            ssl = axtlsl.ssl_client_new(ssl_ctx, 
                    client_sock:getfd(), session_id, sess_id_size)

            res = axtlsl.ssl_handshake_status(ssl)
            if res ~= axtlsl.SSL_OK then
                if not quiet then axtlsl.ssl_display_error(res) end
                axtlsl.ssl_free(ssl)
                os.exit(1)
            end

            display_session_id(ssl)
            session_id = axtlsl.ssl_get_session_id(ssl)
            sess_id_size = axtlsl.ssl_get_session_id_size(ssl)

            if reconnect > 0 then
                axtlsl.ssl_free(ssl)
                client_sock:close()
                client_sock = socket.try(socket.connect(host, port))
            end

        end
    else
        ssl = axtlsl.ssl_client_new(ssl_ctx, client_sock:getfd(), nil, 0)
    end

    -- check the return status
    res = axtlsl.ssl_handshake_status(ssl)
    if res ~= axtlsl.SSL_OK then
        if not quiet then axtlsl.ssl_display_error(res) end
        os.exit(1)
    end

    if not quiet then
        local common_name = axtlsl.ssl_get_cert_dn(ssl, 
                            axtlsl.SSL_X509_CERT_COMMON_NAME)

        if common_name ~= nil then 
            print("Common Name:\t\t\t"..common_name)
        end

        display_session_id(ssl)
        display_cipher(ssl)
    end

    while true do
        local line = io.read()
    if line == nil then break end
        local bytes = {}

        for i = 1, #line do
            bytes[i] = line.byte(line, i)
        end

        bytes[#line+1] = 10      -- add carriage return, null
        bytes[#line+2] = 0

        res = axtlsl.ssl_write(ssl, bytes, #bytes)
        if res < axtlsl.SSL_OK then
            if not quiet then axtlsl.ssl_display_error(res) end
            break
        end
    end

    axtlsl.ssl_ctx_free(ssl_ctx)
    client_sock:close()
end

--
-- Display what cipher we are using 
--
function display_cipher(ssl)
    io.write("CIPHER is ")
    local cipher_id = axtlsl.ssl_get_cipher_id(ssl)

    if cipher_id == axtlsl.SSL_AES128_SHA then
        print("AES128-SHA")
    elseif cipher_id == axtlsl.SSL_AES256_SHA then
        print("AES256-SHA")
    elseif axtlsl.SSL_RC4_128_SHA then
        print("RC4-SHA")
    elseif axtlsl.SSL_RC4_128_MD5 then
        print("RC4-MD5")
    else 
        print("Unknown - "..cipher_id)
    end
end

--
-- Display what session id we have.
--
function display_session_id(ssl)
    local session_id = axtlsl.ssl_get_session_id(ssl)
    local v

    if #session_id > 0 then
        print("-----BEGIN SSL SESSION PARAMETERS-----")
        for _, v in ipairs(session_id) do
            io.write(string.format("%02x", v))
        end
        print("\n-----END SSL SESSION PARAMETERS-----")
    end
end

--
-- Main entry point. Doesn't do much except works out whether we are a client
-- or a server.
--
if #arg == 0 or (arg[1] ~= "s_server" and arg[1] ~= "s_client") then
    print_options(#arg > 0 and arg[1] or "")
end

local build_mode = axtlsl.ssl_get_config(axtlsl.SSL_BUILD_MODE)
_ = arg[1] == "s_server" and do_server(build_mode) or do_client(build_mode)
os.exit(0)

