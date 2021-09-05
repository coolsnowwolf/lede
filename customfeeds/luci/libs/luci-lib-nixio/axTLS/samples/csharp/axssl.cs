/*
 * Copyright (c) 2007, Cameron Rich
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 * * Neither the name of the axTLS project nor the names of its contributors 
 *   may be used to endorse or promote products derived from this software 
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Demonstrate the use of the axTLS library in C# with a set of 
 * command-line parameters similar to openssl. In fact, openssl clients 
 * should be able to communicate with axTLS servers and visa-versa.
 *
 * This code has various bits enabled depending on the configuration. To enable
 * the most interesting version, compile with the 'full mode' enabled.
 *
 * To see what options you have, run the following:
 * > axssl.csharp.exe s_server -?
 * > axssl.csharp.exe s_client -?
 *
 * The axtls shared library must be in the same directory or be found 
 * by the OS.
 */

using System;
using System.Net;
using System.Net.Sockets;
using axTLS;

public class axssl
{
    /*
     * Main()
     */
    public static void Main(string[] args)
    {
        if (args.Length == 1 && args[0] == "version")
        {
            Console.WriteLine("axssl.csharp " + SSLUtil.Version());
            Environment.Exit(0); 
        }

        axssl runner = new axssl();

        if (args.Length < 1 || (args[0] != "s_server" && args[0] != "s_client"))
            runner.print_options(args.Length > 0 ? args[0] : "");

        int build_mode = SSLUtil.BuildMode();

        if (args[0] == "s_server")
            runner.do_server(build_mode, args);
        else 
            runner.do_client(build_mode, args);
    }

    /*
     * do_server()
     */
    private void do_server(int build_mode, string[] args)
    {
        int i = 1;
        int port = 4433;
        uint options = axtls.SSL_DISPLAY_CERTS;
        bool quiet = false;
        string password = null;
        string private_key_file = null;

        /* organise the cert/ca_cert lists */
        int cert_size = SSLUtil.MaxCerts();
        int ca_cert_size = SSLUtil.MaxCACerts();
        string[] cert = new string[cert_size];
        string[] ca_cert = new string[ca_cert_size];
        int cert_index = 0;
        int ca_cert_index = 0;

        while (i < args.Length)
        {
            if (args[i] == "-accept")
            {
                if (i >= args.Length-1)
                {
                    print_server_options(build_mode, args[i]);
                }

                port = Int32.Parse(args[++i]);
            }
            else if (args[i] == "-quiet")
            {
                quiet = true;
                options &= ~(uint)axtls.SSL_DISPLAY_CERTS;
            }
            else if (build_mode >= axtls.SSL_BUILD_SERVER_ONLY)
            {
                if (args[i] == "-cert")
                {
                    if (i >= args.Length-1 || cert_index >= cert_size)
                    {
                        print_server_options(build_mode, args[i]);
                    }

                    cert[cert_index++] = args[++i];
                }
                else if (args[i] == "-key")
                {
                    if (i >= args.Length-1)
                    {
                        print_server_options(build_mode, args[i]);
                    }

                    private_key_file = args[++i];
                    options |= axtls.SSL_NO_DEFAULT_KEY;
                }
                else if (args[i] == "-pass")
                {
                    if (i >= args.Length-1)
                    {
                        print_server_options(build_mode, args[i]);
                    }

                    password = args[++i];
                }
                else if (build_mode >= axtls.SSL_BUILD_ENABLE_VERIFICATION)
                {
                    if (args[i] == "-verify")
                    {
                        options |= axtls.SSL_CLIENT_AUTHENTICATION;
                    }
                    else if (args[i] == "-CAfile")
                    {
                        if (i >= args.Length-1 || ca_cert_index >= ca_cert_size)
                        {
                            print_server_options(build_mode, args[i]);
                        }

                        ca_cert[ca_cert_index++] = args[++i];
                    }
                    else if (build_mode == axtls.SSL_BUILD_FULL_MODE)
                    {
                        if (args[i] == "-debug")
                        {
                            options |= axtls.SSL_DISPLAY_BYTES;
                        }
                        else if (args[i] == "-state")
                        {
                            options |= axtls.SSL_DISPLAY_STATES;
                        }
                        else if (args[i] == "-show-rsa")
                        {
                            options |= axtls.SSL_DISPLAY_RSA;
                        }
                        else
                            print_server_options(build_mode, args[i]);
                    }
                    else
                        print_server_options(build_mode, args[i]);
                }
                else 
                    print_server_options(build_mode, args[i]);
            }
            else
                print_server_options(build_mode, args[i]);

            i++;
        }

        /* Create socket for incoming connections */
        IPEndPoint ep = new IPEndPoint(IPAddress.Any, port);
        TcpListener server_sock = new TcpListener(ep);
        server_sock.Start();      

        /**********************************************************************
         * This is where the interesting stuff happens. Up until now we've
         * just been setting up sockets etc. Now we do the SSL handshake.
         **********************************************************************/
        SSLServer ssl_ctx = new SSLServer(
                                options, axtls.SSL_DEFAULT_SVR_SESS);

        if (ssl_ctx == null)
        {
            Console.Error.WriteLine("Error: Server context is invalid");
            Environment.Exit(1);
        }

        if (private_key_file != null)
        {
            int obj_type = axtls.SSL_OBJ_RSA_KEY;

            if (private_key_file.EndsWith(".p8"))
                obj_type = axtls.SSL_OBJ_PKCS8;
            else if (private_key_file.EndsWith(".p12"))
                obj_type = axtls.SSL_OBJ_PKCS12;

            if (ssl_ctx.ObjLoad(obj_type,
                             private_key_file, password) != axtls.SSL_OK)
            {
                Console.Error.WriteLine("Private key '" + private_key_file +
                                                            "' is undefined.");
                Environment.Exit(1);
            }
        }

        for (i = 0; i < cert_index; i++)
        {
            if (ssl_ctx.ObjLoad(axtls.SSL_OBJ_X509_CERT, 
                                        cert[i], null) != axtls.SSL_OK)
            {
                Console.WriteLine("Certificate '" + cert[i] + 
                        "' is undefined.");
                Environment.Exit(1);
            }
        }

        for (i = 0; i < ca_cert_index; i++)
        {
            if (ssl_ctx.ObjLoad(axtls.SSL_OBJ_X509_CACERT, 
                                        ca_cert[i], null) != axtls.SSL_OK)
            {
                Console.WriteLine("Certificate '" + cert[i] + 
                                                        "' is undefined.");
                Environment.Exit(1);
            }
        }

        byte[] buf = null;
        int res;

        for (;;)
        {
            if (!quiet)
            {
                Console.WriteLine("ACCEPT");
            }

            Socket client_sock = server_sock.AcceptSocket();

            SSL ssl = ssl_ctx.Connect(client_sock);

            /* do the actual SSL handshake */
            while ((res = ssl_ctx.Read(ssl, out buf)) == axtls.SSL_OK)
            {
                /* check when the connection has been established */
                if (ssl.HandshakeStatus() == axtls.SSL_OK)
                    break;

                /* could do something else here */
            }

            if (res == axtls.SSL_OK) /* connection established and ok */
            {
                if (!quiet)
                {
                    display_session_id(ssl);
                    display_cipher(ssl);
                }

                /* now read (and display) whatever the client sends us */
                for (;;)
                {
                    /* keep reading until we get something interesting */
                    while ((res = ssl_ctx.Read(ssl, out buf)) == axtls.SSL_OK)
                    {
                        /* could do something else here */
                    }

                    if (res < axtls.SSL_OK)
                    {
                        if (!quiet)
                        {
                            Console.WriteLine("CONNECTION CLOSED");
                        }

                        break;
                    }

                    /* convert to string */
                    char[] str = new char[res];
                    for (i = 0; i < res; i++)
                    {
                        str[i] = (char)buf[i];
                    }

                    Console.Write(str);
                }
            }
            else if (!quiet)
            {
                SSLUtil.DisplayError(res);
            }

            /* client was disconnected or the handshake failed. */
            ssl.Dispose();
            client_sock.Close();
        }

        /* ssl_ctx.Dispose(); */
    }

    /*
     * do_client()
     */
    private void do_client(int build_mode, string[] args)
    {
        if (build_mode < axtls.SSL_BUILD_ENABLE_CLIENT)
        {
            print_client_options(build_mode, args[1]);
        }

        int i = 1, res;
        int port = 4433;
        bool quiet = false;
        string password = null;
        int reconnect = 0;
        string private_key_file = null;
        string hostname = "127.0.0.1";

        /* organise the cert/ca_cert lists */
        int cert_index = 0;
        int ca_cert_index = 0;
        int cert_size = SSLUtil.MaxCerts();
        int ca_cert_size = SSLUtil.MaxCACerts();
        string[] cert = new string[cert_size];
        string[] ca_cert = new string[ca_cert_size];

        uint options = axtls.SSL_SERVER_VERIFY_LATER|axtls.SSL_DISPLAY_CERTS;
        byte[] session_id = null;

        while (i < args.Length)
        {
            if (args[i] == "-connect")
            {
                string host_port;

                if (i >= args.Length-1)
                {
                    print_client_options(build_mode, args[i]);
                }

                host_port = args[++i];
                int index_colon;

                if ((index_colon = host_port.IndexOf(':')) < 0)
                    print_client_options(build_mode, args[i]);

                hostname = new string(host_port.ToCharArray(), 
                        0, index_colon);
                port = Int32.Parse(new String(host_port.ToCharArray(), 
                            index_colon+1, host_port.Length-index_colon-1));
            }
            else if (args[i] == "-cert")
            {
                if (i >= args.Length-1 || cert_index >= cert_size)
                {
                    print_client_options(build_mode, args[i]);
                }

                cert[cert_index++] = args[++i];
            }
            else if (args[i] == "-key")
            {
                if (i >= args.Length-1)
                {
                    print_client_options(build_mode, args[i]);
                }

                private_key_file = args[++i];
                options |= axtls.SSL_NO_DEFAULT_KEY;
            }
            else if (args[i] == "-CAfile")
            {
                if (i >= args.Length-1 || ca_cert_index >= ca_cert_size)
                {
                    print_client_options(build_mode, args[i]);
                }

                ca_cert[ca_cert_index++] = args[++i];
            }
            else if (args[i] == "-verify")
            {
                options &= ~(uint)axtls.SSL_SERVER_VERIFY_LATER;
            }
            else if (args[i] == "-reconnect")
            {
                reconnect = 4;
            }
            else if (args[i] == "-quiet")
            {
                quiet = true;
                options &= ~(uint)axtls.SSL_DISPLAY_CERTS;
            }
            else if (args[i] == "-pass")
            {
                if (i >= args.Length-1)
                {
                    print_client_options(build_mode, args[i]);
                }

                password = args[++i];
            }
            else if (build_mode == axtls.SSL_BUILD_FULL_MODE)
            {
                if (args[i] == "-debug")
                {
                    options |= axtls.SSL_DISPLAY_BYTES;
                }
                else if (args[i] == "-state")
                {
                    options |= axtls.SSL_DISPLAY_STATES;
                }
                else if (args[i] == "-show-rsa")
                {
                    options |= axtls.SSL_DISPLAY_RSA;
                }
                else
                    print_client_options(build_mode, args[i]);
            }
            else    /* don't know what this is */
                print_client_options(build_mode, args[i]);

            i++;
        }

        // IPHostEntry hostInfo = Dns.Resolve(hostname); 
        IPHostEntry hostInfo = Dns.GetHostEntry(hostname);
        IPAddress[] addresses = hostInfo.AddressList;
        IPEndPoint ep = new IPEndPoint(addresses[0], port); 
        Socket client_sock = new Socket(AddressFamily.InterNetwork, 
                SocketType.Stream, ProtocolType.Tcp);
        client_sock.Connect(ep);

        if (!client_sock.Connected)
        {
            Console.WriteLine("could not connect");
            Environment.Exit(1);
        }

        if (!quiet)
        {
            Console.WriteLine("CONNECTED");
        }

        /**********************************************************************
         * This is where the interesting stuff happens. Up until now we've
         * just been setting up sockets etc. Now we do the SSL handshake.
         **********************************************************************/
        SSLClient ssl_ctx = new SSLClient(options,
                                    axtls.SSL_DEFAULT_CLNT_SESS);

        if (ssl_ctx == null)
        {
            Console.Error.WriteLine("Error: Client context is invalid");
            Environment.Exit(1);
        }

        if (private_key_file != null)
        {
            int obj_type = axtls.SSL_OBJ_RSA_KEY;

            if (private_key_file.EndsWith(".p8"))
                obj_type = axtls.SSL_OBJ_PKCS8;
            else if (private_key_file.EndsWith(".p12"))
                obj_type = axtls.SSL_OBJ_PKCS12;

            if (ssl_ctx.ObjLoad(obj_type,
                             private_key_file, password) != axtls.SSL_OK)
            {
                Console.Error.WriteLine("Private key '" + private_key_file +
                                                            "' is undefined.");
                Environment.Exit(1);
            }
        }

        for (i = 0; i < cert_index; i++)
        {
            if (ssl_ctx.ObjLoad(axtls.SSL_OBJ_X509_CERT, 
                                        cert[i], null) != axtls.SSL_OK)
            {
                Console.WriteLine("Certificate '" + cert[i] + 
                        "' is undefined.");
                Environment.Exit(1);
            }
        }

        for (i = 0; i < ca_cert_index; i++)
        {
            if (ssl_ctx.ObjLoad(axtls.SSL_OBJ_X509_CACERT, 
                                        ca_cert[i], null) != axtls.SSL_OK)
            {
                Console.WriteLine("Certificate '" + cert[i] + 
                                                        "' is undefined.");
                Environment.Exit(1);
            }
        }

        SSL ssl = new SSL(new IntPtr(0));   /* keep compiler happy */

        /* Try session resumption? */
        if (reconnect > 0)
        {
            while (reconnect-- > 0)
            {
                ssl = ssl_ctx.Connect(client_sock, session_id);

                if ((res = ssl.HandshakeStatus()) != axtls.SSL_OK)
                {
                    if (!quiet)
                    {
                        SSLUtil.DisplayError(res);
                    }

                    ssl.Dispose();
                    Environment.Exit(1);
                }

                display_session_id(ssl);
                session_id = ssl.GetSessionId();

                if (reconnect > 0)
                {
                    ssl.Dispose();
                    client_sock.Close();
                    
                    /* and reconnect */
                    client_sock = new Socket(AddressFamily.InterNetwork, 
                        SocketType.Stream, ProtocolType.Tcp);
                    client_sock.Connect(ep);
                }
            }
        }
        else
        {
            ssl = ssl_ctx.Connect(client_sock, null);
        }

        /* check the return status */
        if ((res = ssl.HandshakeStatus()) != axtls.SSL_OK)
        {
            if (!quiet)
            {
                SSLUtil.DisplayError(res);
            }

            Environment.Exit(1);
        }

        if (!quiet)
        {
            string common_name =
                ssl.GetCertificateDN(axtls.SSL_X509_CERT_COMMON_NAME);

            if (common_name != null)
            {
                Console.WriteLine("Common Name:\t\t\t" + common_name);
            }

            display_session_id(ssl);
            display_cipher(ssl);
        }

        for (;;)
        {
            string user_input = Console.ReadLine();

            if (user_input == null)
                break;

            byte[] buf = new byte[user_input.Length+2];
            buf[buf.Length-2] = (byte)'\n';     /* add the carriage return */
            buf[buf.Length-1] = 0;              /* null terminate */

            for (i = 0; i < buf.Length-2; i++)
            {
                buf[i] = (byte)user_input[i];
            }

            if ((res = ssl_ctx.Write(ssl, buf, buf.Length)) < axtls.SSL_OK)
            {
                if (!quiet)
                {
                    SSLUtil.DisplayError(res);
                }

                break;
            }
        }

        ssl_ctx.Dispose();
    }

    /**
     * We've had some sort of command-line error. Print out the basic options.
     */
    private void print_options(string option)
    {
        Console.WriteLine("axssl: Error: '" + option + 
                "' is an invalid command.");
        Console.WriteLine("usage: axssl.csharp [s_server|" +
                            "s_client|version] [args ...]");
        Environment.Exit(1);
    }

    /**
     * We've had some sort of command-line error. Print out the server options.
     */
    private void print_server_options(int build_mode, string option)
    {
        int cert_size = SSLUtil.MaxCerts();
        int ca_cert_size = SSLUtil.MaxCACerts();

        Console.WriteLine("unknown option " + option);
        Console.WriteLine("usage: s_server [args ...]");
        Console.WriteLine(" -accept arg\t- port to accept on (default " + 
                "is 4433)");
        Console.WriteLine(" -quiet\t\t- No server output");

        if (build_mode >= axtls.SSL_BUILD_SERVER_ONLY)
        {
          Console.WriteLine(" -cert arg\t- certificate file to add (in " + 
                  "addition to default) to chain -");
          Console.WriteLine("\t\t  Can repeat up to " + cert_size + " times");
          Console.WriteLine(" -key arg\t- Private key file to use");
          Console.WriteLine(" -pass\t\t- private key file pass phrase source");
        }

        if (build_mode >= axtls.SSL_BUILD_ENABLE_VERIFICATION)
        {
            Console.WriteLine(" -verify\t- turn on peer certificate " +
                    "verification");
            Console.WriteLine(" -CAfile arg\t- Certificate authority.");
            Console.WriteLine("\t\t  Can repeat up to " + 
                    ca_cert_size + "times");
        }

        if (build_mode == axtls.SSL_BUILD_FULL_MODE)
        {
            Console.WriteLine(" -debug\t\t- Print more output");
            Console.WriteLine(" -state\t\t- Show state messages");
            Console.WriteLine(" -show-rsa\t- Show RSA state");
        }

        Environment.Exit(1);
    }

    /**
     * We've had some sort of command-line error. Print out the client options.
     */
    private void print_client_options(int build_mode, string option)
    {
        int cert_size = SSLUtil.MaxCerts();
        int ca_cert_size = SSLUtil.MaxCACerts();

        Console.WriteLine("unknown option " + option);

        if (build_mode >= axtls.SSL_BUILD_ENABLE_CLIENT)
        {
            Console.WriteLine("usage: s_client [args ...]");
            Console.WriteLine(" -connect host:port - who to connect to " + 
                    "(default is localhost:4433)");
            Console.WriteLine(" -verify\t- turn on peer certificate " + 
                    "verification");
            Console.WriteLine(" -cert arg\t- certificate file to use");
            Console.WriteLine("\t\t  Can repeat up to %d times", cert_size);
            Console.WriteLine(" -key arg\t- Private key file to use");
            Console.WriteLine(" -CAfile arg\t- Certificate authority.");
            Console.WriteLine("\t\t  Can repeat up to " + ca_cert_size + 
                    " times");
            Console.WriteLine(" -quiet\t\t- No client output");
            Console.WriteLine(" -pass\t\t- private key file pass " + 
                    "phrase source");
            Console.WriteLine(" -reconnect\t- Drop and re-make the " +
                    "connection with the same Session-ID");

            if (build_mode == axtls.SSL_BUILD_FULL_MODE)
            {
                Console.WriteLine(" -debug\t\t- Print more output");
                Console.WriteLine(" -state\t\t- Show state messages");
                Console.WriteLine(" -show-rsa\t- Show RSA state");
            }
        }
        else 
        {
            Console.WriteLine("Change configuration to allow this feature");
        }

        Environment.Exit(1);
    }

    /**
     * Display what cipher we are using 
     */
    private void display_cipher(SSL ssl)
    {
        Console.Write("CIPHER is ");

        switch (ssl.GetCipherId())
        {
            case axtls.SSL_AES128_SHA:
                Console.WriteLine("AES128-SHA");
                break;

            case axtls.SSL_AES256_SHA:
                Console.WriteLine("AES256-SHA");
                break;

            case axtls.SSL_RC4_128_SHA:
                Console.WriteLine("RC4-SHA");
                break;

            case axtls.SSL_RC4_128_MD5:
                Console.WriteLine("RC4-MD5");
                break;

            default:
                Console.WriteLine("Unknown - " + ssl.GetCipherId());
                break;
        }
    }

    /**
     * Display what session id we have.
     */
    private void display_session_id(SSL ssl)
    {    
        byte[] session_id = ssl.GetSessionId();

        if (session_id.Length > 0)
        {
            Console.WriteLine("-----BEGIN SSL SESSION PARAMETERS-----");
            foreach (byte b in session_id)
            {
                Console.Write("{0:x02}", b);
            }

            Console.WriteLine("\n-----END SSL SESSION PARAMETERS-----");
        }
    }
}
