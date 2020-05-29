/*
 *  Copyright © 2008-2013 by Eric Bishop <eric@gargoyle-router.com>
 * 
 *  This file is free software: you may copy, redistribute and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation, either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This file is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifndef LIB_EWGET_H
#define LIB_EWGET_H


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#include <sys/errno.h>
#include <sys/select.h>
#include <fcntl.h>
#include <signal.h>





#define EWGET_DEFAULT_TIMEOUT_SECONDS 5
#define EWGET_DEFAULT_READ_BUFFER_SIZE 1024

typedef struct
{
	int protocol;
	char* user;
	char* password;
	char* hostname;
	int port;
	char* path;
	char* user_agent;
} url_request;

typedef struct
{
	char* data;
	char* header;
	int is_text;
	int length;
} http_response;





url_request* parse_url(char* url, char* user_agent);
void parse_redirect(url_request** url, char* redirect_url);
void free_url_request(url_request*);

http_response* get_url(char* url_str, char* user_agent);
http_response* get_url_request(url_request** url);
void free_http_response(http_response* page);


unsigned long get_ewget_read_buffer_size(void);
unsigned long get_ewget_timeout_seconds(void);
void set_ewget_read_buffer_size(unsigned long size);
void set_ewget_timeout_seconds(unsigned long seconds);

/* 
 * at LEAST one of header_stream, body_stream, combined_stream should not be NULL 
 * streams are NOT closed (or opened) by these functions
 *
 * return value is 0 on success, 1 on error
 */
int write_url_to_stream(char* url_str, char* user_agent, FILE* header_stream, FILE* body_stream, FILE* combined_stream);
int write_url_request_to_stream(url_request** url, FILE* header_stream, FILE* body_stream, FILE* combined_stream);




#endif /* end LIB_EWGET_H */
