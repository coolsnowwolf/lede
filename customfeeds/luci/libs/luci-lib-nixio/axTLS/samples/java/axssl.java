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

/*
 * Demonstrate the use of the axTLS library in Java with a set of 
 * command-line parameters similar to openssl. In fact, openssl clients 
 * should be able to communicate with axTLS servers and visa-versa.  *
 * This code has various bits enabled depending on the configuration. To enable
 * the most interesting version, compile with the 'full mode' enabled.
 *
 * To see what options you have, run the following:
 * > java -jar axtls.jar s_server -?
 * > java -jar axtls.jar s_client -?
 *
 * The axtls/axtlsj shared libraries must be in the same directory or be found 
 * by the OS.
 */

import java.io.*;
import java.util.*;
import java.net.*;
import axTLSj.*;

public class axssl
{
   /*
     * Main()
     */
    public static void main(String[] args)
    {
        if (args.length == 1 && args[0].equals("version"))
        {
            System.out.println("axtls.jar " + SSLUtil.version());
            System.exit(0);
        }

        axssl runner = new axssl();

        try
        {
            if (args.length < 1 || 
                    (!args[0].equals("s_server") &&
                     !args[0].equals("s_client")))
            {
                runner.print_options(args.length > 0 ? args[0] : "");
            }

            int build_mode = SSLUtil.buildMode();

            if (args[0].equals("s_server"))
                runner.do_server(build_mode, args);
            else 
                runner.do_client(build_mode, args);
        }
        catch (Exception e) 
        {
            System.out.println(e);
        }
    }

    /*
     * do_server()
     */
    private void do_server(int build_mode, String[] args)
                    throws Exception
    {
        int i = 1;
        int port = 4433;
        int options = axtlsj.SSL_DISPLAY_CERTS;
        boolean quiet = false;
        String password = null;
        String private_key_file = null;

        /* organise the cert/ca_cert lists */
        int cert_size = SSLUtil.maxCerts();
        int ca_cert_size = SSLUtil.maxCACerts();
        String[] cert = new String[cert_size];
        String[] ca_cert = new String[ca_cert_size];
        int cert_index = 0;
        int ca_cert_index = 0;

        while (i < args.length)
        {
            if (args[i].equals("-accept"))
            {
                if (i >= args.length-1)
                {
                    print_server_options(build_mode, args[i]);
                }

                port = Integer.parseInt(args[++i]);
            }
            else if (args[i].equals("-quiet"))
            {
                quiet = true;
                options &= ~(int)axtlsj.SSL_DISPLAY_CERTS;
            }
            else if (build_mode >= axtlsj.SSL_BUILD_SERVER_ONLY)
            {
                if (args[i].equals("-cert"))
                {
                    if (i >= args.length-1 || cert_index >= cert_size)
                    {
                        print_server_options(build_mode, args[i]);
                    }

                    cert[cert_index++] = args[++i];
                }
                else if (args[i].equals("-key"))
                {
                    if (i >= args.length-1)
                    {
                        print_server_options(build_mode, args[i]);
                    }

                    private_key_file = args[++i];
                    options |= axtlsj.SSL_NO_DEFAULT_KEY;
                }
                else if (args[i].equals("-pass"))
                {
                    if (i >= args.length-1)
                    {
                        print_server_options(build_mode, args[i]);
                    }

                    password = args[++i];
                }
                else if (build_mode >= axtlsj.SSL_BUILD_ENABLE_VERIFICATION)
                {
                    if (args[i].equals("-verify"))
                    {
                        options |= axtlsj.SSL_CLIENT_AUTHENTICATION;
                    }
                    else if (args[i].equals("-CAfile"))
                    {
                        if (i >= args.length-1 || ca_cert_index >= ca_cert_size)
                        {
                            print_server_options(build_mode, args[i]);
                        }

                        ca_cert[ca_cert_index++] = args[++i];
                    }
                    else if (build_mode == axtlsj.SSL_BUILD_FULL_MODE)
                    {
                        if (args[i].equals("-debug"))
                        {
                            options |= axtlsj.SSL_DISPLAY_BYTES;
                        }
                        else if (args[i].equals("-state"))
                        {
                            options |= axtlsj.SSL_DISPLAY_STATES;
                        }
                        else if (args[i].equals("-show-rsa"))
                        {
                            options |= axtlsj.SSL_DISPLAY_RSA;
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
        ServerSocket server_sock = new ServerSocket(port);

        /**********************************************************************
         * This is where the interesting stuff happens. Up until now we've
         * just been setting up sockets etc. Now we do the SSL handshake.
         **********************************************************************/
        SSLServer ssl_ctx = new SSLServer(options, 
                                    axtlsj.SSL_DEFAULT_SVR_SESS);

        if (ssl_ctx == null)
            throw new Exception("Error: Server context is invalid");

        if (private_key_file != null)
        {
            int obj_type =  axtlsj.SSL_OBJ_RSA_KEY;

            if (private_key_file.endsWith(".p8"))
                obj_type = axtlsj.SSL_OBJ_PKCS8;
            else if (private_key_file.endsWith(".p12"))
                obj_type = axtlsj.SSL_OBJ_PKCS12;

            if (ssl_ctx.objLoad(obj_type, 
                            private_key_file, password) != axtlsj.SSL_OK)
            {
                throw new Exception("Error: Private key '" + private_key_file + 
                        "' is undefined.");
            }
        }

        for (i = 0; i < cert_index; i++)
        {
            if (ssl_ctx.objLoad(axtlsj.SSL_OBJ_X509_CERT, 
                                        cert[i], null) != axtlsj.SSL_OK)
            {
                throw new Exception("Certificate '" + cert[i] + 
                        "' is undefined.");
            }
        }

        for (i = 0; i < ca_cert_index; i++)
        {
            if (ssl_ctx.objLoad(axtlsj.SSL_OBJ_X509_CACERT, 
                                        ca_cert[i], null) != axtlsj.SSL_OK)
            {
                throw new Exception("Certificate '" + ca_cert[i] + 
                        "' is undefined.");
            }
        }

        int res;
        SSLReadHolder rh = new SSLReadHolder();

        for (;;)
        {
            if (!quiet)
            {
                System.out.println("ACCEPT");
            }

            Socket client_sock = server_sock.accept();

            SSL ssl = ssl_ctx.connect(client_sock);

            while ((res = ssl_ctx.read(ssl, rh)) == axtlsj.SSL_OK)
            {
                /* check when the connection has been established */
                if (ssl.handshakeStatus() == axtlsj.SSL_OK)
                    break;

                /* could do something else here */
            }

            if (res == axtlsj.SSL_OK) /* connection established and ok */
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
                    while ((res = ssl_ctx.read(ssl, rh)) == axtlsj.SSL_OK)
                    {
                        /* could do something else here */
                    }

                    if (res < axtlsj.SSL_OK)
                    {
                        if (!quiet)
                        {
                            System.out.println("CONNECTION CLOSED");
                        }

                        break;
                    }

                    /* convert to String */
                    byte[] buf = rh.getData();
                    char[] str = new char[res];

                    for (i = 0; i < res; i++)
                    {
                        str[i] = (char)buf[i];
                    }

                    System.out.print(str);
                }
            }
            else if (!quiet)
            {
                SSLUtil.displayError(res);
            }

            /* client was disconnected or the handshake failed. */
            ssl.dispose();
            client_sock.close();
        }

        /* ssl_ctx.dispose(); */
    }

    /*
     * do_client()
     */
    private void do_client(int build_mode, String[] args)
                    throws Exception
    {
        if (build_mode < axtlsj.SSL_BUILD_ENABLE_CLIENT)
            print_client_options(build_mode, args[1]);

        int i = 1, res;
        int port = 4433;
        boolean quiet = false;
        String password = null;
        int reconnect = 0;
        String private_key_file = null;
        String hostname = "127.0.0.1";

        /* organise the cert/ca_cert lists */
        int cert_index = 0;
        int ca_cert_index = 0;
        int cert_size = SSLUtil.maxCerts();
        int ca_cert_size = SSLUtil.maxCACerts();
        String[] cert = new String[cert_size];
        String[] ca_cert = new String[ca_cert_size];

        int options = axtlsj.SSL_SERVER_VERIFY_LATER|axtlsj.SSL_DISPLAY_CERTS;
        byte[] session_id = null;

        while (i < args.length)
        {
            if (args[i].equals("-connect"))
            {
                String host_port;

                if (i >= args.length-1)
                {
                    print_client_options(build_mode, args[i]);
                }

                host_port = args[++i];
                int index_colon;

                if ((index_colon = host_port.indexOf(':')) < 0)
                    print_client_options(build_mode, args[i]);

                hostname = new String(host_port.toCharArray(), 
                        0, index_colon);
                port = Integer.parseInt(new String(host_port.toCharArray(), 
                            index_colon+1, host_port.length()-index_colon-1));
            }
            else if (args[i].equals("-cert"))
            {
                if (i >= args.length-1 || cert_index >= cert_size)
                {
                    print_client_options(build_mode, args[i]);
                }

                cert[cert_index++] = args[++i];
            }
            else if (args[i].equals("-CAfile"))
            {
                if (i >= args.length-1 || ca_cert_index >= ca_cert_size)
                {
                    print_client_options(build_mode, args[i]);
                }

                ca_cert[ca_cert_index++] = args[++i];
            }
            else if (args[i].equals("-key"))
            {
                if (i >= args.length-1)
                {
                    print_client_options(build_mode, args[i]);
                }

                private_key_file = args[++i];
                options |= axtlsj.SSL_NO_DEFAULT_KEY;
            }
            else if (args[i].equals("-verify"))
            {
                options &= ~(int)axtlsj.SSL_SERVER_VERIFY_LATER;
            }
            else if (args[i].equals("-reconnect"))
            {
                reconnect = 4;
            }
            else if (args[i].equals("-quiet"))
            {
                quiet = true;
                options &= ~(int)axtlsj.SSL_DISPLAY_CERTS;
            }
            else if (args[i].equals("-pass"))
            {
                if (i >= args.length-1)
                {
                    print_server_options(build_mode, args[i]);
                }

                password = args[++i];
            }
            else if (build_mode == axtlsj.SSL_BUILD_FULL_MODE)
            {
                if (args[i].equals("-debug"))
                {
                    options |= axtlsj.SSL_DISPLAY_BYTES;
                }
                else if (args[i].equals("-state"))
                {
                    options |= axtlsj.SSL_DISPLAY_STATES;
                }
                else if (args[i].equals("-show-rsa"))
                {
                    options |= axtlsj.SSL_DISPLAY_RSA;
                }
                else
                    print_client_options(build_mode, args[i]);
            }
            else    /* don't know what this is */
                print_client_options(build_mode, args[i]);

            i++;
        }

        Socket client_sock = new Socket(hostname, port);

        if (!client_sock.isConnected())
        {
            System.out.println("could not connect");
            throw new Exception();
        }

        if (!quiet)
        {
            System.out.println("CONNECTED");
        }

        /**********************************************************************
         * This is where the interesting stuff happens. Up until now we've
         * just been setting up sockets etc. Now we do the SSL handshake.
         **********************************************************************/
        SSLClient ssl_ctx = new SSLClient(options, 
                                    axtlsj.SSL_DEFAULT_CLNT_SESS);

        if (ssl_ctx == null)
        {
            throw new Exception("Error: Client context is invalid");
        }

        if (private_key_file != null)
        {
            int obj_type =  axtlsj.SSL_OBJ_RSA_KEY;

            if (private_key_file.endsWith(".p8"))
                obj_type = axtlsj.SSL_OBJ_PKCS8;
            else if (private_key_file.endsWith(".p12"))
                obj_type = axtlsj.SSL_OBJ_PKCS12;

            if (ssl_ctx.objLoad(obj_type, 
                            private_key_file, password) != axtlsj.SSL_OK)
            {
                throw new Exception("Error: Private key '" + private_key_file + 
                        "' is undefined.");
            }
        }

        for (i = 0; i < cert_index; i++)
        {
            if (ssl_ctx.objLoad(axtlsj.SSL_OBJ_X509_CERT, 
                                        cert[i], null) != axtlsj.SSL_OK)
            {
                throw new Exception("Certificate '" + cert[i] + 
                        "' is undefined.");
            }
        }

        for (i = 0; i < ca_cert_index; i++)
        {
            if (ssl_ctx.objLoad(axtlsj.SSL_OBJ_X509_CACERT, 
                                        ca_cert[i], null) != axtlsj.SSL_OK)
            {
                throw new Exception("Certificate '" + ca_cert[i] + 
                        "' is undefined.");
            }
        }

        SSL ssl = null;

        /* Try session resumption? */
        if (reconnect > 0)
        {
            while (reconnect-- > 0)
            {
                ssl = ssl_ctx.connect(client_sock, session_id);

                if ((res = ssl.handshakeStatus()) != axtlsj.SSL_OK)
                {
                    if (!quiet)
                    {
                        SSLUtil.displayError(res);
                    }

                    ssl.dispose();
                    throw new Exception();
                }

                display_session_id(ssl);
                session_id = ssl.getSessionId();

                if (reconnect > 0)
                {
                    ssl.dispose();
                    client_sock.close();
                    
                    /* and reconnect */
                    client_sock = new Socket(hostname, port);
                }
            }
        }
        else
        {
            ssl = ssl_ctx.connect(client_sock, null);
        }

        /* check the return status */
        if ((res = ssl.handshakeStatus()) != axtlsj.SSL_OK)
        {
            if (!quiet)
            {
                SSLUtil.displayError(res);
            }

            throw new Exception();
        }

        if (!quiet)
        {
            String common_name =
                ssl.getCertificateDN(axtlsj.SSL_X509_CERT_COMMON_NAME);

            if (common_name != null)
            {
                System.out.println("Common Name:\t\t\t" + common_name);
            }

            display_session_id(ssl);
            display_cipher(ssl);
        }

        BufferedReader in = new BufferedReader(
                new InputStreamReader(System.in));

        for (;;)
        {
            String user_input = in.readLine();

            if (user_input == null)
                break;

            byte[] buf = new byte[user_input.length()+2];
            buf[buf.length-2] = (byte)'\n';     /* add the carriage return */
            buf[buf.length-1] = 0;              /* null terminate */

            for (i = 0; i < buf.length-2; i++)
            {
                buf[i] = (byte)user_input.charAt(i);
            }

            if ((res = ssl_ctx.write(ssl, buf)) < axtlsj.SSL_OK)
            {
                if (!quiet)
                {
                    SSLUtil.displayError(res);
                }

                break;
            }
        }

        ssl_ctx.dispose();
    }

    /**
     * We've had some sort of command-line error. Print out the basic options.
     */
    private void print_options(String option)
    {
        System.out.println("axssl: Error: '" + option + 
                "' is an invalid command.");
        System.out.println("usage: axtlsj.jar [s_server|s_client|version] " + 
                "[args ...]");
        System.exit(1);
    }

    /**
     * We've had some sort of command-line error. Print out the server options.
     */
    private void print_server_options(int build_mode, String option)
    {
        int cert_size = SSLUtil.maxCerts();
        int ca_cert_size = SSLUtil.maxCACerts();

        System.out.println("unknown option " + option);
        System.out.println("usage: s_server [args ...]");
        System.out.println(" -accept arg\t- port to accept on (default " + 
                "is 4433)");
        System.out.println(" -quiet\t\t- No server output");

        if (build_mode >= axtlsj.SSL_BUILD_SERVER_ONLY)
        {
            System.out.println(" -cert arg\t- certificate file to add (in " + 
                    "addition to default) to chain -");
            System.out.println("\t\t  Can repeat up to " + cert_size + " times");
            System.out.println(" -key arg\t- Private key file to use");
            System.out.println(" -pass\t\t- private key file pass phrase source");
        }

        if (build_mode >= axtlsj.SSL_BUILD_ENABLE_VERIFICATION)
        {
            System.out.println(" -verify\t- turn on peer certificate " +
                    "verification");
            System.out.println(" -CAfile arg\t- Certificate authority. ");
            System.out.println("\t\t  Can repeat up to " + 
                    ca_cert_size + " times");
        }

        if (build_mode == axtlsj.SSL_BUILD_FULL_MODE)
        {
            System.out.println(" -debug\t\t- Print more output");
            System.out.println(" -state\t\t- Show state messages");
            System.out.println(" -show-rsa\t- Show RSA state");
        }

        System.exit(1);
    }

    /**
     * We've had some sort of command-line error. Print out the client options.
     */
    private void print_client_options(int build_mode, String option)
    {
        int cert_size = SSLUtil.maxCerts();
        int ca_cert_size = SSLUtil.maxCACerts();

        System.out.println("unknown option " + option);

        if (build_mode >= axtlsj.SSL_BUILD_ENABLE_CLIENT)
        {
            System.out.println("usage: s_client [args ...]");
            System.out.println(" -connect host:port - who to connect to " + 
                    "(default is localhost:4433)");
            System.out.println(" -verify\t- turn on peer certificate " + 
                    "verification");
            System.out.println(" -cert arg\t- certificate file to use");
            System.out.println(" -key arg\t- Private key file to use");
            System.out.println("\t\t  Can repeat up to " + cert_size + 
                    " times");
            System.out.println(" -CAfile arg\t- Certificate authority.");
            System.out.println("\t\t  Can repeat up to " + ca_cert_size + 
                    " times");
            System.out.println(" -quiet\t\t- No client output");
            System.out.println(" -pass\t\t- private key file pass " + 
                        "phrase source");
            System.out.println(" -reconnect\t- Drop and re-make the " +
                    "connection with the same Session-ID");

            if (build_mode == axtlsj.SSL_BUILD_FULL_MODE)
            {
                System.out.println(" -debug\t\t- Print more output");
                System.out.println(" -state\t\t- Show state messages");
                System.out.println(" -show-rsa\t- Show RSA state");
            }
        }
        else 
        {
            System.out.println("Change configuration to allow this feature");
        }

        System.exit(1);
    }

    /**
     * Display what cipher we are using 
     */
    private void display_cipher(SSL ssl)
    {
        System.out.print("CIPHER is ");

        byte ciph_id = ssl.getCipherId();

        if (ciph_id == axtlsj.SSL_AES128_SHA)
            System.out.println("AES128-SHA");
        else if (ciph_id == axtlsj.SSL_AES256_SHA)
            System.out.println("AES256-SHA");
        else if (ciph_id == axtlsj.SSL_RC4_128_SHA)
            System.out.println("RC4-SHA");
        else if (ciph_id == axtlsj.SSL_RC4_128_MD5)
            System.out.println("RC4-MD5");
        else
            System.out.println("Unknown - " + ssl.getCipherId());
    }

    public char toHexChar(int i)
    {
        if ((0 <= i) && (i <= 9 ))
            return (char)('0' + i);
        else
            return (char)('a' + (i-10));
    }

    public void bytesToHex(byte[] data) 
    {
        StringBuffer buf = new StringBuffer();
        for (int i = 0; i < data.length; i++ ) 
        {
            buf.append(toHexChar((data[i]>>>4)&0x0F));
            buf.append(toHexChar(data[i]&0x0F));
        }

        System.out.println(buf);
    }


    /**
     * Display what session id we have.
     */
    private void display_session_id(SSL ssl)
    {    
        byte[] session_id = ssl.getSessionId();

        if (session_id.length > 0)
        {
            System.out.println("-----BEGIN SSL SESSION PARAMETERS-----");
            bytesToHex(session_id);
            System.out.println("-----END SSL SESSION PARAMETERS-----");
        }
    }
}
