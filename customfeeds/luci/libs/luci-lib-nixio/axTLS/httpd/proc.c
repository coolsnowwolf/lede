/*
 * Copyright (c) 2007-2008, Cameron Rich
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include "axhttp.h"

#define HTTP_VERSION        "HTTP/1.1"

static const char * index_file = "index.html";

static int special_read(struct connstruct *cn, void *buf, size_t count);
static int special_write(struct connstruct *cn, 
                                        const char *buf, size_t count);
static void send_error(struct connstruct *cn, int err);
static int hexit(char c);
static void urldecode(char *buf);
static void buildactualfile(struct connstruct *cn);
static int sanitizefile(const char *buf);
static int sanitizehost(char *buf);
static int htaccess_check(struct connstruct *cn);
static const char *getmimetype(const char *name);

#if defined(CONFIG_HTTP_DIRECTORIES)
static void urlencode(const uint8_t *s, char *t);
static void procdirlisting(struct connstruct *cn);
#endif
#if defined(CONFIG_HTTP_HAS_CGI)
static void proccgi(struct connstruct *cn);
static void decode_path_info(struct connstruct *cn, char *path_info);
static int init_read_post_data(char *buf, char *data, struct connstruct *cn, int old_rv);
#endif
#ifdef CONFIG_HTTP_HAS_AUTHORIZATION
static int auth_check(struct connstruct *cn);
#endif

#if AXDEBUG
#define AXDEBUGSTART \
	{ \
		FILE *axdout; \
		axdout = fopen("/var/log/axdebug", "a"); \
	
#define AXDEBUGEND \
		fclose(axdout); \
	}
#else /* AXDEBUG */
#define AXDEBUGSTART
#define AXDEBUGEND
#endif /* AXDEBUG */

/* Returns 1 if elems should continue being read, 0 otherwise */
static int procheadelem(struct connstruct *cn, char *buf) 
{
    char *delim, *value;

    if ((delim = strchr(buf, ' ')) == NULL)
        return 0;

    *delim = 0;
    value = delim+1;

    if (strcmp(buf, "GET") == 0 || strcmp(buf, "HEAD") == 0 ||
                                            strcmp(buf, "POST") == 0) 
    {
        if (buf[0] == 'H') 
            cn->reqtype = TYPE_HEAD;
        else if (buf[0] == 'P') 
            cn->reqtype = TYPE_POST;

        if ((delim = strchr(value, ' ')) == NULL)       /* expect HTTP type */
            return 0;

        *delim = 0;
        urldecode(value);

        if (sanitizefile(value) == 0) 
        {
            send_error(cn, 403);
            return 0;
        }

#if defined(CONFIG_HTTP_HAS_CGI)
        decode_path_info(cn, value);
#else
        my_strncpy(cn->filereq, value, MAXREQUESTLENGTH);
#endif
        cn->if_modified_since = -1;
    } 
    else if (strcmp(buf, "Host:") == 0) 
    {
        if (sanitizehost(value) == 0) 
        {
            removeconnection(cn);
            return 0;
        }

        my_strncpy(cn->server_name, value, MAXREQUESTLENGTH);
    } 
    else if (strcmp(buf, "Connection:") == 0 && strcmp(value, "close") == 0) 
    {
        cn->close_when_done = 1;
    } 
    else if (strcmp(buf, "If-Modified-Since:") == 0) 
    {
        cn->if_modified_since = tdate_parse(value);
    }
    else if (strcmp(buf, "Expect:") == 0)
    {
        send_error(cn, 417); /* expectation failed */
        return 0;
    }
#ifdef CONFIG_HTTP_HAS_AUTHORIZATION
    else if (strcmp(buf, "Authorization:") == 0 &&
                                    strncmp(value, "Basic ", 6) == 0)
    {
        int size;
        if (base64_decode(&value[6], strlen(&value[6]), 
                                        (uint8_t *)cn->authorization, &size))
            cn->authorization[0] = 0;   /* error */
        else
            cn->authorization[size] = 0;
    }
#endif
#if defined(CONFIG_HTTP_HAS_CGI)
    else if (strcmp(buf, "Content-Length:") == 0)
    {
        sscanf(value, "%d", &cn->content_length);
    }
    else if (strcmp(buf, "Cookie:") == 0)
    {
        my_strncpy(cn->cookie, value, MAXREQUESTLENGTH);
    }
#endif

    return 1;
}

#if defined(CONFIG_HTTP_DIRECTORIES)
static void procdirlisting(struct connstruct *cn)
{
    char buf[MAXREQUESTLENGTH];
    char actualfile[1024];

    if (cn->reqtype == TYPE_HEAD) 
    {
        snprintf(buf, sizeof(buf), HTTP_VERSION
                " 200 OK\nContent-Type: text/html\n\n");
        write(cn->networkdesc, buf, strlen(buf));
        removeconnection(cn);
        return;
    }

    strcpy(actualfile, cn->actualfile);

#ifdef WIN32
    strcat(actualfile, "*");
    cn->dirp = FindFirstFile(actualfile, &cn->file_data);

    if (cn->dirp == INVALID_HANDLE_VALUE) 
    {
        send_error(cn, 404);
        return;
    }
#else
    if ((cn->dirp = opendir(actualfile)) == NULL) 
    {
        send_error(cn, 404);
        return;
    }
#endif

    snprintf(buf, sizeof(buf), HTTP_VERSION
            " 200 OK\nContent-Type: text/html\n\n"
            "<html><body>\n<title>Directory Listing</title>\n"
            "<h3>Directory listing of %s://%s%s</h3><br />\n", 
            cn->is_ssl ? "https" : "http", cn->server_name, cn->filereq);
    special_write(cn, buf, strlen(buf));
    cn->state = STATE_DOING_DIR;
}

void procdodir(struct connstruct *cn) 
{
#ifndef WIN32
    struct dirent *dp;
#endif
    char buf[MAXREQUESTLENGTH];
    char encbuf[1024];
    char *file;

    do 
    {
       buf[0] = 0;

#ifdef WIN32
        if (!FindNextFile(cn->dirp, &cn->file_data)) 
#else
        if ((dp = readdir(cn->dirp)) == NULL)  
#endif
        {
            snprintf(buf, sizeof(buf), "</body></html>\n");
            special_write(cn, buf, strlen(buf));
            removeconnection(cn);
#ifndef WIN32
            closedir(cn->dirp);
#endif
            return;
        }

#ifdef WIN32
        file = cn->file_data.cFileName;
#else
        file = dp->d_name;
#endif

        /* if no index file, don't display the ".." directory */
        if (cn->filereq[0] == '/' && cn->filereq[1] == '\0' &&
                strcmp(file, "..") == 0) 
            continue;

        /* don't display files beginning with "." */
        if (file[0] == '.' && file[1] != '.')
            continue;

        /* make sure a '/' is at the end of a directory */
        if (cn->filereq[strlen(cn->filereq)-1] != '/')
            strcat(cn->filereq, "/");

        /* see if the dir + file is another directory */
        snprintf(buf, sizeof(buf), "%s%s", cn->actualfile, file);
        if (isdir(buf))
            strcat(file, "/");

        urlencode((uint8_t *)file, encbuf);
        snprintf(buf, sizeof(buf), "<a href=\"%s%s\">%s</a><br />\n",
                cn->filereq, encbuf, file);
    } while (special_write(cn, buf, strlen(buf)));
}

/* Encode funny chars -> %xx in newly allocated storage */
/* (preserves '/' !) */
static void urlencode(const uint8_t *s, char *t) 
{
    const uint8_t *p = s;
    char *tp = t;

    for (; *p; p++) 
    {
        if ((*p > 0x00 && *p < ',') ||
                (*p > '9' && *p < 'A') ||
                (*p > 'Z' && *p < '_') ||
                (*p > '_' && *p < 'a') ||
                (*p > 'z' && *p < 0xA1)) 
        {
            sprintf((char *)tp, "%%%02X", *p);
            tp += 3; 
        } 
        else 
        {
            *tp = *p;
            tp++;
        }
    }

    *tp='\0';
}

#endif

void procreadhead(struct connstruct *cn) 
{
    char buf[MAXREQUESTLENGTH*4], *tp, *next;
    int rv;

    memset(buf, 0, MAXREQUESTLENGTH*4);
    rv = special_read(cn, buf, sizeof(buf)-1);
    if (rv <= 0) 
    {
        if (rv < 0) /* really dead? */
            removeconnection(cn);
        return;
    }

    buf[rv] = '\0';
    next = tp = buf;

#ifdef CONFIG_HTTP_HAS_AUTHORIZATION
    cn->authorization[0] = 0;
#endif

    /* Split up lines and send to procheadelem() */
    while (*next != '\0') 
    {
        /* If we have a blank line, advance to next stage */
        if (*next == '\r' || *next == '\n') 
        {
#if defined(CONFIG_HTTP_HAS_CGI)
            if (cn->reqtype == TYPE_POST && cn->content_length > 0)
            {
                if (init_read_post_data(buf,next,cn,rv) == 0)
                    return;
            }
#endif

            buildactualfile(cn);
            cn->state = STATE_WANT_TO_SEND_HEAD;
            return;
        }

        while (*next != '\r' && *next != '\n' && *next != '\0') 
            next++;

        if (*next == '\r') 
        {
            *next = '\0';
            next += 2;
        }
        else if (*next == '\n') 
            *next++ = '\0';

        if (procheadelem(cn, tp) == 0) 
            return;

        tp = next;
    }
}

/* In this function we assume that the file has been checked for
 * maliciousness (".."s, etc) and has been decoded
 */
void procsendhead(struct connstruct *cn) 
{
    char buf[MAXREQUESTLENGTH];
    struct stat stbuf;
    time_t now = cn->timeout - CONFIG_HTTP_TIMEOUT;
    char date[32];
    int file_exists;

    /* are we trying to access a file over the HTTP connection instead of a
     * HTTPS connection? Or is this directory disabled? */
    if (htaccess_check(cn))      
    {
        send_error(cn, 403);
        return;
    }

#ifdef CONFIG_HTTP_HAS_AUTHORIZATION
    if (auth_check(cn))     /* see if there is a '.htpasswd' file */
    {
#ifdef CONFIG_HTTP_VERBOSE
        printf("axhttpd: access to %s denied\n", cn->filereq); TTY_FLUSH();
#endif
        removeconnection(cn);
        return;
    }
#endif

    file_exists = stat(cn->actualfile, &stbuf);

#if defined(CONFIG_HTTP_HAS_CGI)

    if (file_exists != -1 && cn->is_cgi)
    {
        if ((stbuf.st_mode & S_IEXEC) == 0 || isdir(cn->actualfile))
        {
            /* A non-executable file, or directory? */
            send_error(cn, 403);
        }
        else
            proccgi(cn);

        return;
    }
#endif

    /* look for "index.html"? */
    if (isdir(cn->actualfile))
    {
        char tbuf[MAXREQUESTLENGTH];
        snprintf(tbuf, MAXREQUESTLENGTH, "%s%s", cn->actualfile, index_file);

        if ((file_exists = stat(tbuf, &stbuf)) != -1) 
            my_strncpy(cn->actualfile, tbuf, MAXREQUESTLENGTH);
        else
        {
#if defined(CONFIG_HTTP_DIRECTORIES)
            /* If not, we do a directory listing of it */
            procdirlisting(cn);
#else
            send_error(cn, 404);
#endif
            return;
        }
    }

    if (file_exists == -1)
    {
        send_error(cn, 404);
        return;
    }

    strcpy(date, ctime(&now));

    /* has the file been read before? */
    if (cn->if_modified_since != -1 && (cn->if_modified_since == 0 || 
                                       cn->if_modified_since >= stbuf.st_mtime))
    {
        snprintf(buf, sizeof(buf), HTTP_VERSION" 304 Not Modified\nServer: "
                "%s\nDate: %s\n", server_version, date);
        special_write(cn, buf, strlen(buf));
        cn->state = STATE_WANT_TO_READ_HEAD;
        return;
    }

    if (cn->reqtype == TYPE_HEAD) 
    {
        removeconnection(cn);
        return;
    } 
    else 
    {
        int flags = O_RDONLY;
#if defined(WIN32) || defined(CONFIG_PLATFORM_CYGWIN)
        flags |= O_BINARY;
#endif
        cn->filedesc = open(cn->actualfile, flags);

        if (cn->filedesc < 0) 
        {
            send_error(cn, 404);
            return;
        }

        snprintf(buf, sizeof(buf), HTTP_VERSION" 200 OK\nServer: %s\n"
            "Content-Type: %s\nContent-Length: %ld\n"
            "Date: %sLast-Modified: %s\n", server_version,
            getmimetype(cn->actualfile), (long) stbuf.st_size,
            date, ctime(&stbuf.st_mtime)); /* ctime() has a \n on the end */

        special_write(cn, buf, strlen(buf));

#ifdef CONFIG_HTTP_VERBOSE
        printf("axhttpd: %s:/%s\n", cn->is_ssl ? "https" : "http", cn->filereq);
        TTY_FLUSH();
#endif

#ifdef WIN32
        for (;;)
        {
            procreadfile(cn);
            if (cn->filedesc == -1)
                break;

            do 
            {
                procsendfile(cn);
            } while (cn->state != STATE_WANT_TO_READ_FILE);
        }
#else
        cn->state = STATE_WANT_TO_READ_FILE;
#endif
    }
}

void procreadfile(struct connstruct *cn) 
{
    int rv = read(cn->filedesc, cn->databuf, BLOCKSIZE);

    if (rv <= 0) 
    {
        close(cn->filedesc);
        cn->filedesc = -1;

        if (cn->close_when_done)        /* close immediately */
            removeconnection(cn);
        else 
        {                               /* keep socket open - HTTP 1.1 */
            cn->state = STATE_WANT_TO_READ_HEAD;
            cn->numbytes = 0;
        }

        return;
    }

    cn->numbytes = rv;
    cn->state = STATE_WANT_TO_SEND_FILE;
}

void procsendfile(struct connstruct *cn) 
{
    int rv = special_write(cn, cn->databuf, cn->numbytes);

    if (rv < 0)
        removeconnection(cn);
    else if (rv == cn->numbytes)
    {
        cn->state = STATE_WANT_TO_READ_FILE;
    }
    else if (rv == 0)
    { 
        /* Do nothing */ 
    }
    else 
    {
        memmove(cn->databuf, cn->databuf + rv, cn->numbytes - rv);
        cn->numbytes -= rv;
    }
}

#if defined(CONFIG_HTTP_HAS_CGI)
/* Should this be a bit more dynamic? It would mean more calls to malloc etc */
#define CGI_ARG_SIZE        17

static void proccgi(struct connstruct *cn) 
{
    int tpipe[2], spipe[2];
    char *myargs[2];
    char cgienv[CGI_ARG_SIZE][MAXREQUESTLENGTH];
    char * cgiptr[CGI_ARG_SIZE+4];
    const char *type = "HEAD";
    int cgi_index = 0, i;
    pid_t pid;
#ifdef WIN32
    int tmp_stdout;
#endif

    snprintf(cgienv[0], MAXREQUESTLENGTH, 
            HTTP_VERSION" 200 OK\nServer: %s\n%s",
            server_version, (cn->reqtype == TYPE_HEAD) ? "\n" : "");
    special_write(cn, cgienv[0], strlen(cgienv[0]));

    if (cn->reqtype == TYPE_HEAD) 
    {
        removeconnection(cn);
        return;
    }

#ifdef CONFIG_HTTP_VERBOSE
        printf("[CGI]: %s:/%s\n", cn->is_ssl ? "https" : "http", cn->filereq);
        TTY_FLUSH();
#endif

    /* win32 cgi is a bit too painful */
#ifndef WIN32
	/* set up pipe that is used for sending POST query data to CGI script*/
    if (cn->reqtype == TYPE_POST) 
    {
        if (pipe(spipe) == -1)
        {
            printf("[CGI]: could not create pipe");
            TTY_FLUSH();
            return;
        }
    }

	if (pipe(tpipe) == -1)
    {
        printf("[CGI]: could not create pipe");
        TTY_FLUSH();
        return;
    }

    /*
     * use vfork() instead of fork() for performance 
     */
    if ((pid = vfork()) > 0)  /* parent */
    {
        /* Send POST query data to CGI script */
        if ((cn->reqtype == TYPE_POST) && (cn->content_length > 0)) 
        {
            write(spipe[1], cn->post_data, cn->content_length);
            close(spipe[0]);	 
            close(spipe[1]);

            /* free the memory that is allocated in read_post_data() */
            free(cn->post_data); 
            cn->post_data = NULL;
        }

        /* Close the write descriptor */
        close(tpipe[1]);
        cn->filedesc = tpipe[0];
        cn->state = STATE_WANT_TO_READ_FILE;
        cn->close_when_done = 1;
        return;
    }

    if (pid < 0) /* vfork failed */
        exit(1);

    /* The problem child... */

    /* Our stdout/stderr goes to the socket */
    dup2(tpipe[1], 1);
    dup2(tpipe[1], 2);

    /* If it was a POST request, send the socket data to our stdin */
    if (cn->reqtype == TYPE_POST) 
        dup2(spipe[0], 0);  
    else    /* Otherwise we can shutdown the read side of the sock */
        shutdown(cn->networkdesc, 0);

    myargs[0] = cn->actualfile;
    myargs[1] = NULL;

    /* 
     * set the cgi args. A url is defined by:
     * http://$SERVER_NAME:$SERVER_PORT$SCRIPT_NAME$PATH_INFO?$QUERY_STRING
     * TODO: other CGI parameters?
     */
    sprintf(cgienv[cgi_index++], "SERVER_SOFTWARE=%s", server_version);
    strcpy(cgienv[cgi_index++], "DOCUMENT_ROOT=" CONFIG_HTTP_WEBROOT);
    snprintf(cgienv[cgi_index++], MAXREQUESTLENGTH,
            "SERVER_NAME=%s", cn->server_name);
    sprintf(cgienv[cgi_index++], "SERVER_PORT=%d", 
            cn->is_ssl ? CONFIG_HTTP_HTTPS_PORT : CONFIG_HTTP_PORT);
    snprintf(cgienv[cgi_index++], MAXREQUESTLENGTH,
            "REQUEST_URI=%s", cn->uri_request);
    snprintf(cgienv[cgi_index++], MAXREQUESTLENGTH,
            "SCRIPT_NAME=%s", cn->filereq);
    snprintf(cgienv[cgi_index++], MAXREQUESTLENGTH,
            "PATH_INFO=%s", cn->uri_path_info);
    snprintf(cgienv[cgi_index++], MAXREQUESTLENGTH,
            "QUERY_STRING=%s", cn->uri_query);
    snprintf(cgienv[cgi_index++], MAXREQUESTLENGTH,
            "REMOTE_ADDR=%s", cn->remote_addr);
    snprintf(cgienv[cgi_index++], MAXREQUESTLENGTH,
            "HTTP_COOKIE=%s", cn->cookie);  /* note: small size */
#if defined(CONFIG_HTTP_HAS_AUTHORIZATION)
    snprintf(cgienv[cgi_index++], MAXREQUESTLENGTH,
            "REMOTE_USER=%s", cn->authorization);
#endif

    switch (cn->reqtype)
    {
        case TYPE_GET: 
            type = "GET";
            break;

        case TYPE_POST:
            type = "POST";
            sprintf(cgienv[cgi_index++], 
                        "CONTENT_LENGTH=%d", cn->content_length);
            strcpy(cgienv[cgi_index++],     /* hard-code? */
                        "CONTENT_TYPE=application/x-www-form-urlencoded");
            break;
    }

    sprintf(cgienv[cgi_index++], "REQUEST_METHOD=%s", type);

    if (cn->is_ssl)
        strcpy(cgienv[cgi_index++], "HTTPS=on");

#ifdef CONFIG_PLATFORM_CYGWIN
    /* TODO: find out why Lua needs this */
    strcpy(cgienv[cgi_index++], "PATH=/usr/bin");
#endif

    if (cgi_index >= CGI_ARG_SIZE)
    {
        printf("Content-type: text/plain\n\nToo many CGI args (%d, %d)\n",
                cgi_index, CGI_ARG_SIZE);
        _exit(1);
    }

    /* copy across the pointer indexes */
    for (i = 0; i < cgi_index; i++)
        cgiptr[i] = cgienv[i];

    cgiptr[i++] = "AUTH_TYPE=Basic";
    cgiptr[i++] = "GATEWAY_INTERFACE=CGI/1.1";
    cgiptr[i++] = "SERVER_PROTOCOL="HTTP_VERSION;
    cgiptr[i] = NULL;

    execve(myargs[0], myargs, cgiptr);
    printf("Content-type: text/plain\n\nshouldn't get here\n");
    _exit(1);
#endif
}

static char * cgi_filetype_match(struct connstruct *cn, const char *fn)
{
    struct cgiextstruct *tp = cgiexts;

    while (tp != NULL) 
    {
        char *t;

        if ((t = strstr(fn, tp->ext)) != NULL)
        {
            t += strlen(tp->ext);

            if (*t == '/' || *t == '\0')
            {
#ifdef CONFIG_HTTP_ENABLE_LUA
                if (strcmp(tp->ext, ".lua") == 0 || strcmp(tp->ext, ".lp") == 0)
                    cn->is_lua = 1;
#endif

                return t;
            }
            else
                return NULL;

        }

        tp = tp->next;
    }

    return NULL;
}

static void decode_path_info(struct connstruct *cn, char *path_info)
{
    char *cgi_delim;

    cn->is_cgi = 0;
#ifdef CONFIG_HTTP_ENABLE_LUA
    cn->is_lua = 0;
#endif
    *cn->uri_request = '\0';
    *cn->uri_path_info = '\0';
    *cn->uri_query = '\0';

    my_strncpy(cn->uri_request, path_info, MAXREQUESTLENGTH);

    /* query info? */
    if ((cgi_delim = strchr(path_info, '?')))
    {
        *cgi_delim = '\0';
        my_strncpy(cn->uri_query, cgi_delim+1, MAXREQUESTLENGTH);
    }

    if ((cgi_delim = cgi_filetype_match(cn, path_info)) != NULL)
    {
        cn->is_cgi = 1;     /* definitely a CGI script */

        /* path info? */
        if (*cgi_delim != '\0')
        {
            my_strncpy(cn->uri_path_info, cgi_delim, MAXREQUESTLENGTH);
            *cgi_delim = '\0';
        }
    }

    /* the bit at the start must be the script name */
    my_strncpy(cn->filereq, path_info, MAXREQUESTLENGTH);
}

static int init_read_post_data(char *buf, char *data, 
                                struct connstruct *cn, int old_rv)
{
   char *next = data;
   int rv = old_rv;
   char *post_data;

    /* Too much Post data to send. MAXPOSTDATASIZE should be 
       configured (now it can be chaged in the header file) */
   if (cn->content_length > MAXPOSTDATASIZE) 
   {
       send_error(cn, 418);
       return 0;
   }
   
   /* remove CRLF */
   while ((*next == '\r' || *next == '\n') && (next < &buf[rv])) 
       next++;
   
   if (cn->post_data == NULL)
   {
       cn->post_data = (char *) calloc(1, (cn->content_length + 1)); 
       /* Allocate buffer for the POST data that will be used by proccgi 
          to send POST data to the CGI script */

       if (cn->post_data == NULL)
       {
           printf("axhttpd: could not allocate memory for POST data\n"); 
           TTY_FLUSH();
           send_error(cn, 599);
           return 0;
       }
   }

   cn->post_state = 0;
   cn->post_read = 0;
   post_data = cn->post_data;

   while (next < &buf[rv])
   { 
       /*copy POST data to buffer*/
       *post_data = *next;
       post_data++;
       next++;
       cn->post_read++;
       if (cn->post_read == cn->content_length)
       { 
           /* No more POST data to be copied */
           *post_data = '\0';
           return 1;
       }
   }

   /* More POST data has to be read. read_post_data will continue with that */
   cn->post_state = 1;
   return 0;
}

void read_post_data(struct connstruct *cn)
{
    char buf[MAXREQUESTLENGTH*4], *next;
    char *post_data;
    int rv;

    bzero(buf,MAXREQUESTLENGTH*4);
    rv = special_read(cn, buf, sizeof(buf)-1);
    if (rv <= 0) 
    {
        if (rv < 0) /* really dead? */
            removeconnection(cn);
        return;
    }

    buf[rv] = '\0';
    next = buf;

    post_data = &cn->post_data[cn->post_read];

    while (next < &buf[rv])
    {
        *post_data = *next;
        post_data++;
        next++;
        cn->post_read++;
        if (cn->post_read == cn->content_length)
        {  
            /* No more POST data to be copied */
            *post_data='\0';
            cn->post_state = 0;
            buildactualfile(cn);
            cn->state = STATE_WANT_TO_SEND_HEAD;
            return;
        }
    }

    /* More POST data to read */
}

#endif  /* CONFIG_HTTP_HAS_CGI */

/* Decode string %xx -> char (in place) */
static void urldecode(char *buf) 
{
    int v;
    char *p, *s, *w;

    w = p = buf;

    while (*p) 
    {
        v = 0;

        if (*p == '%') 
        {
            s = p;
            s++;

            if (isxdigit((int) s[0]) && isxdigit((int) s[1]))
            {
                v = hexit(s[0])*16 + hexit(s[1]);

                if (v) 
                { 
                    /* do not decode %00 to null char */
                    *w = (char)v;
                    p = &s[1];
                }
            }

        }

        if (!v) *w=*p;
        p++; 
        w++;
    }

    *w='\0';
}

static int hexit(char c) 
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    else
        return 0;
}

static void buildactualfile(struct connstruct *cn)
{
    char *cp;
    snprintf(cn->actualfile, MAXREQUESTLENGTH, ".%s", cn->filereq);

#ifndef WIN32
    /* Add directory slash if not there */
    if (isdir(cn->actualfile) && 
            cn->actualfile[strlen(cn->actualfile)-1] != '/')
        strcat(cn->actualfile, "/");

    /* work out the directory name */
    strncpy(cn->dirname, cn->actualfile, MAXREQUESTLENGTH);
    if ((cp = strrchr(cn->dirname, '/')) == NULL)
        cn->dirname[0] = 0;
    else
        *cp = 0;
#else
    {
        char curr_dir[MAXREQUESTLENGTH];
        char path[MAXREQUESTLENGTH];
        char *t = cn->actualfile;

        GetCurrentDirectory(MAXREQUESTLENGTH, curr_dir);

        /* convert all the forward slashes to back slashes */
        while ((t = strchr(t, '/')))
            *t++ = '\\';

        snprintf(path, MAXREQUESTLENGTH, "%s%s", curr_dir, cn->actualfile);
        memcpy(cn->actualfile, path, MAXREQUESTLENGTH);

        /* Add directory slash if not there */
        if (isdir(cn->actualfile) && 
                    cn->actualfile[strlen(cn->actualfile)-1] != '\\')
            strcat(cn->actualfile, "\\");

        /* work out the directory name */
        strncpy(cn->dirname, cn->actualfile, MAXREQUESTLENGTH);
        if ((cp = strrchr(cn->dirname, '\\')) == NULL)
            cn->dirname[0] = 0;
        else
            *cp = 0;
    }
#endif

#if defined(CONFIG_HTTP_ENABLE_LUA)
    /* 
     * Use the lua launcher if this file has a lua extension. Put this at the
     * end as we need the directory name.
     */
    if (cn->is_lua)
        sprintf(cn->actualfile, "%s%s", CONFIG_HTTP_LUA_PREFIX, 
                CONFIG_HTTP_LUA_CGI_LAUNCHER);
#endif
}

static int sanitizefile(const char *buf) 
{
    int len, i;

    /* Don't accept anything not starting with a / */
    if (*buf != '/') 
        return 0;

    len = strlen(buf);
    for (i = 0; i < len; i++) 
    {
        /* Check for "/." i.e. don't send files starting with a . */
        if (buf[i] == '/' && buf[i+1] == '.') 
            return 0;
    }

    return 1;
}

static int sanitizehost(char *buf)
{
    while (*buf != '\0') 
    {
        /* Handle the port */
        if (*buf == ':') 
        {
            *buf = '\0';
            return 1;
        }

        /* Enforce some basic URL rules... */
        if ((isalnum(*buf) == 0 && *buf != '-' && *buf != '.') ||
                (*buf == '.' && *(buf+1) == '.') ||
                (*buf == '.' && *(buf+1) == '-') ||
                (*buf == '-' && *(buf+1) == '.'))
            return 0;

        buf++;
    }

    return 1;
}

static FILE * exist_check(struct connstruct *cn, const char *check_file)
{
    char pathname[MAXREQUESTLENGTH];
    snprintf(pathname, MAXREQUESTLENGTH, "%s/%s", cn->dirname, check_file);
    return fopen(pathname, "r");
}

#ifdef CONFIG_HTTP_HAS_AUTHORIZATION
static void send_authenticate(struct connstruct *cn, const char *realm)
{
    char buf[1024];

    snprintf(buf, sizeof(buf), HTTP_VERSION" 401 Unauthorized\n"
         "WWW-Authenticate: Basic\n"
                 "realm=\"%s\"\n", realm);
    special_write(cn, buf, strlen(buf));
}

static int check_digest(char *salt, const char *msg_passwd)
{
    uint8_t b256_salt[MAXREQUESTLENGTH];
    uint8_t real_passwd[MD5_SIZE];
    int salt_size;
    char *b64_passwd;
    uint8_t md5_result[MD5_SIZE];
    MD5_CTX ctx;

    /* retrieve the salt */
    if ((b64_passwd = strchr(salt, '$')) == NULL)
        return -1;

    *b64_passwd++ = 0;
    if (base64_decode(salt, strlen(salt), b256_salt, &salt_size))
        return -1;

    if (base64_decode(b64_passwd, strlen(b64_passwd), real_passwd, NULL))
        return -1;

    /* very simple MD5 crypt algorithm, but then the salt we use is large */
    MD5_Init(&ctx);
    MD5_Update(&ctx, b256_salt, salt_size);           /* process the salt */
    MD5_Update(&ctx, (uint8_t *)msg_passwd, strlen(msg_passwd)); 
    MD5_Final(md5_result, &ctx);
    return memcmp(md5_result, real_passwd, MD5_SIZE);/* 0 = ok */
}

static int auth_check(struct connstruct *cn)
{
    char line[MAXREQUESTLENGTH];
    FILE *fp;
    char *cp;

    if ((fp = exist_check(cn, ".htpasswd")) == NULL)
        return 0;               /* no .htpasswd file, so let though */

    if (cn->authorization[0] == 0)
        goto error;

    /* cn->authorization is in form "username:password" */
    if ((cp = strchr(cn->authorization, ':')) == NULL)
        goto error;
    else
        *cp++ = 0;  /* cp becomes the password */

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        char *b64_file_passwd;
        int l = strlen(line);

        /* nuke newline */
        if (line[l-1] == '\n')
            line[l-1] = 0;

        /* line is form "username:salt(b64)$password(b64)" */
        if ((b64_file_passwd = strchr(line, ':')) == NULL)
            continue;

        *b64_file_passwd++ = 0;

        if (strcmp(line, cn->authorization)) /* our user? */
            continue;

        if (check_digest(b64_file_passwd, cp) == 0)
        {
            fclose(fp);
            return 0;
        }
    }

error:
    fclose(fp);
    send_authenticate(cn, cn->server_name);
    return -1;
}
#endif

static int htaccess_check(struct connstruct *cn)
{
    char line[MAXREQUESTLENGTH];
    FILE *fp;
    int ret = 0;

    if ((fp = exist_check(cn, ".htaccess")) == NULL)
        return 0;               /* no .htaccess file, so let though */

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        if (strstr(line, "Deny all") || /* access to this dir denied */
                    /* Access will be denied unless SSL is active */
                    (!cn->is_ssl && strstr(line, "SSLRequireSSL")) ||
                    /* Access will be denied if SSL is active */
                    (cn->is_ssl && strstr(line, "SSLDenySSL")))
        {
            ret = -1;
            break;
        }
    }

    fclose(fp);
    return ret;
}

static void send_error(struct connstruct *cn, int err)
{
    char buf[MAXREQUESTLENGTH];
    char *title;
    char *text;

    switch (err)
    {
        case 403:
            title = "Forbidden";
            text = "File is protected";
#ifdef CONFIG_HTTP_VERBOSE
            printf("axhttpd: access to %s denied\n", cn->filereq); TTY_FLUSH();
#endif
            break;

        case 404:
            title = "Not Found";
            text = title;
            break;

        case 418:
            title = "POST data size is to large";
            text = title;
            break;

        default:
            title = "Unknown";
            text = "Unknown";
            break;
    }

    snprintf(buf, MAXREQUESTLENGTH, "HTTP/1.1 %d %s\n"
            "Content-Type: text/html\n"
            "Cache-Control: no-cache,no-store\n"
            "Connection: close\n\n"
            "<html>\n<head>\n<title>%d %s</title></head>\n"
            "<body><h1>%d %s</h1>\n</body></html>\n", 
            err, title, err, title, err, text);
    special_write(cn, buf, strlen(buf));
    removeconnection(cn);
}

static const char *getmimetype(const char *name)
{
    /* only bother with a few mime types - let the browser figure the rest out */
    if (strstr(name, ".htm"))
        return "text/html";
    else if (strstr(name, ".css"))
        return "text/css"; 
    else
        return "application/octet-stream";
}

static int special_write(struct connstruct *cn, 
                                        const char *buf, size_t count)
{
    if (cn->is_ssl)
    {
        SSL *ssl = cn->ssl;
        return ssl ? ssl_write(ssl, (uint8_t *)buf, count) : -1;
    }
    else
        return SOCKET_WRITE(cn->networkdesc, buf, count);
}

static int special_read(struct connstruct *cn, void *buf, size_t count)
{
    int res;

    if (cn->is_ssl)
    {
        uint8_t *read_buf;
        if ((res = ssl_read(cn->ssl, &read_buf)) > SSL_OK)
        {
            memcpy(buf, read_buf, res > (int)count ? count : res);
        }
    }
    else
        res = SOCKET_READ(cn->networkdesc, buf, count);

    return res;
}

