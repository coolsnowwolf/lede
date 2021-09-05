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

#include "ssl.h"

#define BACKLOG 15
#define VERSION "1.0.0"
#ifdef CONFIG_HTTP_HAS_IPV6
#define HAVE_IPV6
#endif

#define MAXPOSTDATASIZE                     30000
#define MAXREQUESTLENGTH                    256
#define BLOCKSIZE                           4096

#define INITIAL_CONNECTION_SLOTS            10
#define CONFIG_HTTP_DEFAULT_SSL_OPTIONS     SSL_DISPLAY_CERTS

#define STATE_WANT_TO_READ_HEAD             1
#define STATE_WANT_TO_SEND_HEAD             2
#define STATE_WANT_TO_READ_FILE             3
#define STATE_WANT_TO_SEND_FILE             4
#define STATE_DOING_DIR                     5

enum
{
    TYPE_GET,
    TYPE_HEAD,
    TYPE_POST
};

struct connstruct 
{
    struct connstruct *next;
    int state;
    int reqtype;
    int networkdesc;
    int filedesc;
    SSL *ssl;

#if defined(CONFIG_HTTP_DIRECTORIES)
#ifdef WIN32
    HANDLE dirp;
    WIN32_FIND_DATA file_data;
#else
    DIR *dirp;
#endif
#endif

    time_t timeout;
    char actualfile[MAXREQUESTLENGTH];
    char filereq[MAXREQUESTLENGTH];
    char dirname[MAXREQUESTLENGTH];
    char server_name[MAXREQUESTLENGTH];
    int numbytes;
    char databuf[BLOCKSIZE];
    uint8_t is_ssl;
    uint8_t close_when_done;
    time_t if_modified_since;

#if defined(CONFIG_HTTP_HAS_CGI)
    uint8_t is_cgi;
#ifdef CONFIG_HTTP_ENABLE_LUA
    uint8_t is_lua;
#endif
    int content_length;
    char remote_addr[MAXREQUESTLENGTH];
    char uri_request[MAXREQUESTLENGTH];
    char uri_path_info[MAXREQUESTLENGTH];
    char uri_query[MAXREQUESTLENGTH];
    char cookie[MAXREQUESTLENGTH];
#endif
#if defined(CONFIG_HTTP_HAS_AUTHORIZATION)
    char authorization[MAXREQUESTLENGTH];
#endif
  int post_read;
  int post_state;
  char *post_data;
};

struct serverstruct 
{
    struct serverstruct *next;
    int sd;
    int is_ssl;
    SSL_CTX *ssl_ctx;
};

#if defined(CONFIG_HTTP_HAS_CGI)
struct cgiextstruct 
{
    struct cgiextstruct *next;
    char *ext;
};
#endif

/* global prototypes */
extern struct serverstruct *servers;
extern struct connstruct *usedconns;
extern struct connstruct *freeconns;
extern const char * const server_version;

#if defined(CONFIG_HTTP_HAS_CGI)
extern struct cgiextstruct *cgiexts;
#endif

/* conn.c prototypes */
void removeconnection(struct connstruct *cn);

/* proc.c prototypes */
void procdodir(struct connstruct *cn);
void procreadhead(struct connstruct *cn);
void procsendhead(struct connstruct *cn);
void procreadfile(struct connstruct *cn);
void procsendfile(struct connstruct *cn);
#if defined(CONFIG_HTTP_HAS_CGI)
void read_post_data(struct connstruct *cn);
#endif

/* misc.c prototypes */
char *my_strncpy(char *dest, const char *src, size_t n);
int isdir(const char *name);

/* tdate prototypes */
void tdate_init(void);
time_t tdate_parse(const char* str);

