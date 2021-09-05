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
 * Demonstrate the use of the axTLS library in C with a set of 
 * command-line parameters similar to openssl. In fact, openssl clients 
 * should be able to communicate with axTLS servers and visa-versa.
 *
 * This code has various bits enabled depending on the configuration. To enable
 * the most interesting version, compile with the 'full mode' enabled.
 *
 * To see what options you have, run the following:
 * > axssl s_server -?
 * > axssl s_client -?
 *
 * The axtls shared library must be in the same directory or be found 
 * by the OS.
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ssl.h"

/* define standard input */
#ifndef STDIN_FILENO
#define STDIN_FILENO        0
#endif

static void do_server(int argc, char *argv[]);
static void print_options(char *option);
static void print_server_options(char *option);
static void do_client(int argc, char *argv[]);
static void print_client_options(char *option);
static void display_cipher(SSL *ssl);
static void display_session_id(SSL *ssl);

/**
 * Main entry point. Doesn't do much except works out whether we are a client
 * or a server.
 */
int main(int argc, char *argv[])
{
#ifdef WIN32
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSAStartup(wVersionRequested, &wsaData);
#elif !defined(CONFIG_PLATFORM_SOLARIS)
    signal(SIGPIPE, SIG_IGN);           /* ignore pipe errors */
#endif

    if (argc == 2 && strcmp(argv[1], "version") == 0)
    {
        printf("axssl %s %s\n", ssl_version(), __DATE__);
        exit(0);
    }

    if (argc < 2 || (
                strcmp(argv[1], "s_server") && strcmp(argv[1], "s_client")))
        print_options(argc > 1 ? argv[1] : "");

    strcmp(argv[1], "s_server") ? 
        do_client(argc, argv) : do_server(argc, argv);
    return 0;
}

/**
 * Implement the SSL server logic. 
 */
static void do_server(int argc, char *argv[])
{
    int i = 2;
    uint16_t port = 4433;
    uint32_t options = SSL_DISPLAY_CERTS;
    int client_fd;
    SSL_CTX *ssl_ctx;
    int server_fd, res = 0;
    socklen_t client_len;
#ifndef CONFIG_SSL_SKELETON_MODE
    char *private_key_file = NULL;
    const char *password = NULL;
    char **cert;
    int cert_index = 0;
    int cert_size = ssl_get_config(SSL_MAX_CERT_CFG_OFFSET);
#endif
#ifdef WIN32
    char yes = 1;
#else
    int yes = 1;
#endif
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;
    int quiet = 0;
#ifdef CONFIG_SSL_CERT_VERIFICATION
    int ca_cert_index = 0;
    int ca_cert_size = ssl_get_config(SSL_MAX_CA_CERT_CFG_OFFSET);
    char **ca_cert = (char **)calloc(1, sizeof(char *)*ca_cert_size);
#endif
    fd_set read_set;

#ifndef CONFIG_SSL_SKELETON_MODE
    cert = (char **)calloc(1, sizeof(char *)*cert_size);
#endif

    while (i < argc)
    {
        if (strcmp(argv[i], "-accept") == 0)
        {
            if (i >= argc-1)
            {
                print_server_options(argv[i]);
            }

            port = atoi(argv[++i]);
        }
#ifndef CONFIG_SSL_SKELETON_MODE
        else if (strcmp(argv[i], "-cert") == 0)
        {
            if (i >= argc-1 || cert_index >= cert_size)
            {
                print_server_options(argv[i]);
            }

            cert[cert_index++] = argv[++i];
        }
        else if (strcmp(argv[i], "-key") == 0)
        {
            if (i >= argc-1)
            {
                print_server_options(argv[i]);
            }

            private_key_file = argv[++i];
            options |= SSL_NO_DEFAULT_KEY;
        }
        else if (strcmp(argv[i], "-pass") == 0)
        {
            if (i >= argc-1)
            {
                print_server_options(argv[i]);
            }

            password = argv[++i];
        }
#endif
        else if (strcmp(argv[i], "-quiet") == 0)
        {
            quiet = 1;
            options &= ~SSL_DISPLAY_CERTS;
        }
#ifdef CONFIG_SSL_CERT_VERIFICATION
        else if (strcmp(argv[i], "-verify") == 0)
        {
            options |= SSL_CLIENT_AUTHENTICATION;
        }
        else if (strcmp(argv[i], "-CAfile") == 0)
        {
            if (i >= argc-1 || ca_cert_index >= ca_cert_size)
            {
                print_server_options(argv[i]);
            }

            ca_cert[ca_cert_index++] = argv[++i];
        }
#endif
#ifdef CONFIG_SSL_FULL_MODE
        else if (strcmp(argv[i], "-debug") == 0)
        {
            options |= SSL_DISPLAY_BYTES;
        }
        else if (strcmp(argv[i], "-state") == 0)
        {
            options |= SSL_DISPLAY_STATES;
        }
        else if (strcmp(argv[i], "-show-rsa") == 0)
        {
            options |= SSL_DISPLAY_RSA;
        }
#endif
        else    /* don't know what this is */
        {
            print_server_options(argv[i]);
        }

        i++;
    }

    if ((ssl_ctx = ssl_ctx_new(options, SSL_DEFAULT_SVR_SESS)) == NULL)
    {
        fprintf(stderr, "Error: Server context is invalid\n");
        exit(1);
    }

#ifndef CONFIG_SSL_SKELETON_MODE
    if (private_key_file)
    {
        int obj_type = SSL_OBJ_RSA_KEY;
        
        /* auto-detect the key type from the file extension */
        if (strstr(private_key_file, ".p8"))
            obj_type = SSL_OBJ_PKCS8;
        else if (strstr(private_key_file, ".p12"))
            obj_type = SSL_OBJ_PKCS12;

        if (ssl_obj_load(ssl_ctx, obj_type, private_key_file, password))
        {
            fprintf(stderr, "Error: Private key '%s' is undefined.\n", 
                                                        private_key_file);
            exit(1);
        }
    }

    for (i = 0; i < cert_index; i++)
    {
        if (ssl_obj_load(ssl_ctx, SSL_OBJ_X509_CERT, cert[i], NULL))
        {
            printf("Certificate '%s' is undefined.\n", cert[i]);
            exit(1);
        }
    }
#endif

#ifdef CONFIG_SSL_CERT_VERIFICATION
    for (i = 0; i < ca_cert_index; i++)
    {
        if (ssl_obj_load(ssl_ctx, SSL_OBJ_X509_CACERT, ca_cert[i], NULL))
        {
            printf("Certificate '%s' is undefined.\n", ca_cert[i]);
            exit(1);
        }
    }

    free(ca_cert);
#endif
#ifndef CONFIG_SSL_SKELETON_MODE
    free(cert);
#endif

    /* Create socket for incoming connections */
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return;
    }
      
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    /* Construct local address structure */
    memset(&serv_addr, 0, sizeof(serv_addr));      /* Zero out structure */
    serv_addr.sin_family = AF_INET;                /* Internet address family */
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    serv_addr.sin_port = htons(port);              /* Local port */

    /* Bind to the local address */
    if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        exit(1);
    }

    client_len = sizeof(client_addr);

    /*************************************************************************
     * This is where the interesting stuff happens. Up until now we've
     * just been setting up sockets etc. Now we do the SSL handshake.
     *************************************************************************/
    for (;;)
    {
        SSL *ssl;
        int reconnected = 0;

        if (!quiet)
        {
            printf("ACCEPT\n");
            TTY_FLUSH();
        }

        if ((client_fd = accept(server_fd, 
                (struct sockaddr *)&client_addr, &client_len)) < 0)
        {
            res = 1;
            break;
        }

        ssl = ssl_server_new(ssl_ctx, client_fd);

        /* now read (and display) whatever the client sends us */
        for (;;)
        {
            /* allow parallel reading of client and standard input */
            FD_ZERO(&read_set);
            FD_SET(client_fd, &read_set);

#ifndef WIN32
            /* win32 doesn't like mixing up stdin and sockets */
            if (isatty(STDIN_FILENO))/* but only if we are in an active shell */
            {
                FD_SET(STDIN_FILENO, &read_set);
            }

            if ((res = select(client_fd+1, &read_set, NULL, NULL, NULL)) > 0)
            {
               uint8_t buf[1024];

                /* read standard input? */
                if (FD_ISSET(STDIN_FILENO, &read_set))
                {
                    if (fgets((char *)buf, sizeof(buf), stdin) == NULL)
                    {
                        res = SSL_ERROR_CONN_LOST;
                    }
                    else
                    {
                        /* small hack to check renegotiation */
                        if (buf[0] == 'r' && (buf[1] == '\n' || buf[1] == '\r'))
                        {
                            res = ssl_renegotiate(ssl);
                        }
                        else    /* write our ramblings to the client */
                        {
                            res = ssl_write(ssl, buf, strlen((char *)buf)+1);
                        }
                    }
                }
                else    /* a socket read */
#endif
                {
                    /* keep reading until we get something interesting */
                    uint8_t *read_buf;

                    if ((res = ssl_read(ssl, &read_buf)) == SSL_OK)
                    {
                        /* are we in the middle of doing a handshake? */
                        if (ssl_handshake_status(ssl) != SSL_OK)
                        {
                            reconnected = 0;
                        }
                        else if (!reconnected)
                        {
                            /* we are connected/reconnected */
                            if (!quiet)
                            {
                                display_session_id(ssl);
                                display_cipher(ssl);
                            }

                            reconnected = 1;
                        }
                    }

                    if (res > SSL_OK)    /* display our interesting output */
                    {
                        printf("%s", read_buf);
                        TTY_FLUSH();
                    }
                    else if (res < SSL_OK && !quiet)
                    {
                        ssl_display_error(res);
                    }
                }
#ifndef WIN32
            }
#endif

            if (res < SSL_OK)
            {
                if (!quiet)
                {
                    printf("CONNECTION CLOSED\n");
                    TTY_FLUSH();
                }

                break;
            }
        }

        /* client was disconnected or the handshake failed. */
        ssl_free(ssl);
        SOCKET_CLOSE(client_fd);
    }

    ssl_ctx_free(ssl_ctx);
}

/**
 * Implement the SSL client logic.
 */
static void do_client(int argc, char *argv[])
{
#ifdef CONFIG_SSL_ENABLE_CLIENT
    int res, i = 2;
    uint16_t port = 4433;
    uint32_t options = SSL_SERVER_VERIFY_LATER|SSL_DISPLAY_CERTS;
    int client_fd;
    char *private_key_file = NULL;
    struct sockaddr_in client_addr;
    struct hostent *hostent;
    int reconnect = 0;
    uint32_t sin_addr;
    SSL_CTX *ssl_ctx;
    SSL *ssl = NULL;
    int quiet = 0;
    int cert_index = 0, ca_cert_index = 0;
    int cert_size, ca_cert_size;
    char **ca_cert, **cert;
    uint8_t session_id[SSL_SESSION_ID_SIZE];
    fd_set read_set;
    const char *password = NULL;

    FD_ZERO(&read_set);
    sin_addr = inet_addr("127.0.0.1");
    cert_size = ssl_get_config(SSL_MAX_CERT_CFG_OFFSET);
    ca_cert_size = ssl_get_config(SSL_MAX_CA_CERT_CFG_OFFSET);
    ca_cert = (char **)calloc(1, sizeof(char *)*ca_cert_size);
    cert = (char **)calloc(1, sizeof(char *)*cert_size);

    while (i < argc)
    {
        if (strcmp(argv[i], "-connect") == 0)
        {
            char *host, *ptr;

            if (i >= argc-1)
            {
                print_client_options(argv[i]);
            }

            host = argv[++i];
            if ((ptr = strchr(host, ':')) == NULL)
            {
                print_client_options(argv[i]);
            }

            *ptr++ = 0;
            port = atoi(ptr);
            hostent = gethostbyname(host);

            if (hostent == NULL)
            {
                print_client_options(argv[i]);
            }

            sin_addr = *((uint32_t **)hostent->h_addr_list)[0];
        }
        else if (strcmp(argv[i], "-cert") == 0)
        {
            if (i >= argc-1 || cert_index >= cert_size)
            {
                print_client_options(argv[i]);
            }

            cert[cert_index++] = argv[++i];
        }
        else if (strcmp(argv[i], "-key") == 0)
        {
            if (i >= argc-1)
            {
                print_client_options(argv[i]);
            }

            private_key_file = argv[++i];
            options |= SSL_NO_DEFAULT_KEY;
        }
        else if (strcmp(argv[i], "-CAfile") == 0)
        {
            if (i >= argc-1 || ca_cert_index >= ca_cert_size)
            {
                print_client_options(argv[i]);
            }

            ca_cert[ca_cert_index++] = argv[++i];
        }
        else if (strcmp(argv[i], "-verify") == 0)
        {
            options &= ~SSL_SERVER_VERIFY_LATER;
        }
        else if (strcmp(argv[i], "-reconnect") == 0)
        {
            reconnect = 4;
        }
        else if (strcmp(argv[i], "-quiet") == 0)
        {
            quiet = 1;
            options &= ~SSL_DISPLAY_CERTS;
        }
        else if (strcmp(argv[i], "-pass") == 0)
        {
            if (i >= argc-1)
            {
                print_client_options(argv[i]);
            }

            password = argv[++i];
        }
#ifdef CONFIG_SSL_FULL_MODE
        else if (strcmp(argv[i], "-debug") == 0)
        {
            options |= SSL_DISPLAY_BYTES;
        }
        else if (strcmp(argv[i], "-state") == 0)
        {
            options |= SSL_DISPLAY_STATES;
        }
        else if (strcmp(argv[i], "-show-rsa") == 0)
        {
            options |= SSL_DISPLAY_RSA;
        }
#endif
        else    /* don't know what this is */
        {
            print_client_options(argv[i]);
        }

        i++;
    }

    if ((ssl_ctx = ssl_ctx_new(options, SSL_DEFAULT_CLNT_SESS)) == NULL)
    {
        fprintf(stderr, "Error: Client context is invalid\n");
        exit(1);
    }

    if (private_key_file)
    {
        int obj_type = SSL_OBJ_RSA_KEY;
        
        /* auto-detect the key type from the file extension */
        if (strstr(private_key_file, ".p8"))
            obj_type = SSL_OBJ_PKCS8;
        else if (strstr(private_key_file, ".p12"))
            obj_type = SSL_OBJ_PKCS12;

        if (ssl_obj_load(ssl_ctx, obj_type, private_key_file, password))
        {
            fprintf(stderr, "Error: Private key '%s' is undefined.\n", 
                                                        private_key_file);
            exit(1);
        }
    }

    for (i = 0; i < cert_index; i++)
    {
        if (ssl_obj_load(ssl_ctx, SSL_OBJ_X509_CERT, cert[i], NULL))
        {
            printf("Certificate '%s' is undefined.\n", cert[i]);
            exit(1);
        }
    }

    for (i = 0; i < ca_cert_index; i++)
    {
        if (ssl_obj_load(ssl_ctx, SSL_OBJ_X509_CACERT, ca_cert[i], NULL))
        {
            printf("Certificate '%s' is undefined.\n", ca_cert[i]);
            exit(1);
        }
    }

    free(cert);
    free(ca_cert);

    /*************************************************************************
     * This is where the interesting stuff happens. Up until now we've
     * just been setting up sockets etc. Now we do the SSL handshake.
     *************************************************************************/
    client_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(port);
    client_addr.sin_addr.s_addr = sin_addr;

    if (connect(client_fd, (struct sockaddr *)&client_addr, 
                sizeof(client_addr)) < 0)
    {
        perror("connect");
        exit(1);
    }

    if (!quiet)
    {
        printf("CONNECTED\n");
        TTY_FLUSH();
    }

    /* Try session resumption? */
    if (reconnect)
    {
        while (reconnect--)
        {
            ssl = ssl_client_new(ssl_ctx, client_fd, session_id,
                    sizeof(session_id));
            if ((res = ssl_handshake_status(ssl)) != SSL_OK)
            {
                if (!quiet)
                {
                    ssl_display_error(res);
                }

                ssl_free(ssl);
                exit(1);
            }

            display_session_id(ssl);
            memcpy(session_id, ssl_get_session_id(ssl), SSL_SESSION_ID_SIZE);

            if (reconnect)
            {
                ssl_free(ssl);
                SOCKET_CLOSE(client_fd);

                client_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                connect(client_fd, (struct sockaddr *)&client_addr, 
                        sizeof(client_addr));
            }
        }
    }
    else
    {
        ssl = ssl_client_new(ssl_ctx, client_fd, NULL, 0);
    }

    /* check the return status */
    if ((res = ssl_handshake_status(ssl)) != SSL_OK)
    {
        if (!quiet)
        {
            ssl_display_error(res);
        }

        exit(1);
    }

    if (!quiet)
    {
        const char *common_name = ssl_get_cert_dn(ssl,
                SSL_X509_CERT_COMMON_NAME);
        if (common_name)
        {
            printf("Common Name:\t\t\t%s\n", common_name);
        }

        display_session_id(ssl);
        display_cipher(ssl);
    }

    for (;;)
    {
        uint8_t buf[1024];
        res = SSL_OK;

        /* allow parallel reading of server and standard input */
        FD_SET(client_fd, &read_set);
#ifndef WIN32
        /* win32 doesn't like mixing up stdin and sockets */
        FD_SET(STDIN_FILENO, &read_set);

        if ((res = select(client_fd+1, &read_set, NULL, NULL, NULL)) > 0)
        {
            /* read standard input? */
            if (FD_ISSET(STDIN_FILENO, &read_set))
#endif
            {
                if (fgets((char *)buf, sizeof(buf), stdin) == NULL)
                {
                    /* bomb out of here */
                    ssl_free(ssl);
                    break;
                }
                else
                {
                    /* small hack to check renegotiation */
                    if (buf[0] == 'R' && (buf[1] == '\n' || buf[1] == '\r'))
                    {
                        res = ssl_renegotiate(ssl);
                    }
                    else
                    {
                        res = ssl_write(ssl, buf, strlen((char *)buf)+1);
                    }
                }
            }
#ifndef WIN32
            else    /* a socket read */
            {
                uint8_t *read_buf;

                res = ssl_read(ssl, &read_buf);

                if (res > 0)    /* display our interesting output */
                {
                    printf("%s", read_buf);
                    TTY_FLUSH();
                }
            }
        }
#endif

        if (res < 0)
        {
            if (!quiet)
            {
                ssl_display_error(res);
            }

            break;      /* get outta here */
        }
    }

    ssl_ctx_free(ssl_ctx);
    SOCKET_CLOSE(client_fd);
#else
    print_client_options(argv[1]);
#endif
}

/**
 * We've had some sort of command-line error. Print out the basic options.
 */
static void print_options(char *option)
{
    printf("axssl: Error: '%s' is an invalid command.\n", option);
    printf("usage: axssl [s_server|s_client|version] [args ...]\n");
    exit(1);
}

/**
 * We've had some sort of command-line error. Print out the server options.
 */
static void print_server_options(char *option)
{
#ifndef CONFIG_SSL_SKELETON_MODE
    int cert_size = ssl_get_config(SSL_MAX_CERT_CFG_OFFSET);
#endif
#ifdef CONFIG_SSL_CERT_VERIFICATION
    int ca_cert_size = ssl_get_config(SSL_MAX_CA_CERT_CFG_OFFSET);
#endif

    printf("unknown option %s\n", option);
    printf("usage: s_server [args ...]\n");
    printf(" -accept arg\t- port to accept on (default is 4433)\n");
#ifndef CONFIG_SSL_SKELETON_MODE
    printf(" -cert arg\t- certificate file to add (in addition to default)"
                                    " to chain -\n"
          "\t\t  Can repeat up to %d times\n", cert_size);
    printf(" -key arg\t- Private key file to use\n");
    printf(" -pass\t\t- private key file pass phrase source\n");
#endif
    printf(" -quiet\t\t- No server output\n");
#ifdef CONFIG_SSL_CERT_VERIFICATION
    printf(" -verify\t- turn on peer certificate verification\n");
    printf(" -CAfile arg\t- Certificate authority\n");
    printf("\t\t  Can repeat up to %d times\n", ca_cert_size);
#endif
#ifdef CONFIG_SSL_FULL_MODE
    printf(" -debug\t\t- Print more output\n");
    printf(" -state\t\t- Show state messages\n");
    printf(" -show-rsa\t- Show RSA state\n");
#endif
    exit(1);
}

/**
 * We've had some sort of command-line error. Print out the client options.
 */
static void print_client_options(char *option)
{
#ifdef CONFIG_SSL_ENABLE_CLIENT
    int cert_size = ssl_get_config(SSL_MAX_CERT_CFG_OFFSET);
    int ca_cert_size = ssl_get_config(SSL_MAX_CA_CERT_CFG_OFFSET);
#endif

    printf("unknown option %s\n", option);
#ifdef CONFIG_SSL_ENABLE_CLIENT
    printf("usage: s_client [args ...]\n");
    printf(" -connect host:port - who to connect to (default "
            "is localhost:4433)\n");
    printf(" -verify\t- turn on peer certificate verification\n");
    printf(" -cert arg\t- certificate file to use\n");
    printf("\t\t  Can repeat up to %d times\n", cert_size);
    printf(" -key arg\t- Private key file to use\n");
    printf(" -CAfile arg\t- Certificate authority\n");
    printf("\t\t  Can repeat up to %d times\n", ca_cert_size);
    printf(" -quiet\t\t- No client output\n");
    printf(" -reconnect\t- Drop and re-make the connection "
            "with the same Session-ID\n");
    printf(" -pass\t\t- private key file pass phrase source\n");
#ifdef CONFIG_SSL_FULL_MODE
    printf(" -debug\t\t- Print more output\n");
    printf(" -state\t\t- Show state messages\n");
    printf(" -show-rsa\t- Show RSA state\n");
#endif
#else
    printf("Change configuration to allow this feature\n");
#endif
    exit(1);
}

/**
 * Display what cipher we are using 
 */
static void display_cipher(SSL *ssl)
{
    printf("CIPHER is ");
    switch (ssl_get_cipher_id(ssl))
    {
        case SSL_AES128_SHA:
            printf("AES128-SHA");
            break;

        case SSL_AES256_SHA:
            printf("AES256-SHA");
            break;

        case SSL_RC4_128_SHA:
            printf("RC4-SHA");
            break;

        case SSL_RC4_128_MD5:
            printf("RC4-MD5");
            break;

        default:
            printf("Unknown - %d", ssl_get_cipher_id(ssl));
            break;
    }

    printf("\n");
    TTY_FLUSH();
}

/**
 * Display what session id we have.
 */
static void display_session_id(SSL *ssl)
{    
    int i;
    const uint8_t *session_id = ssl_get_session_id(ssl);
    int sess_id_size = ssl_get_session_id_size(ssl);

    if (sess_id_size > 0)
    {
        printf("-----BEGIN SSL SESSION PARAMETERS-----\n");
        for (i = 0; i < sess_id_size; i++)
        {
            printf("%02x", session_id[i]);
        }

        printf("\n-----END SSL SESSION PARAMETERS-----\n");
        TTY_FLUSH();
    }
}
