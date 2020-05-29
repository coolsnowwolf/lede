/*
 *  Copyright © 2008-2011 by Eric Bishop <eric@gargoyle-router.com>
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



#include "ewget.h"

#ifdef USE_ERICS_TOOLS
	#include "erics_tools.h"
#else
static void *safe_malloc(size_t size)
{
	void* val = malloc(size);
	if(val == NULL)
	{
		fprintf(stderr, "ERROR: MALLOC FAILURE!\n");
		exit(1);
	}
	return val;
}

static char* safe_strdup(const char* str)
{
	char* new_str = strdup(str);
	if(new_str == NULL)
	{
		fprintf(stderr, "ERROR: MALLOC FAILURE!\n");
		exit(1);
	}
	return new_str;
}
#endif



#define malloc safe_malloc
#define strdup safe_strdup



#define UNKNOWN_PROTO 1
#define HTTP_PROTO 2
#define HTTPS_PROTO 3



static unsigned long __ewget_timeout_seconds  = EWGET_DEFAULT_TIMEOUT_SECONDS;
static unsigned long __ewget_read_buffer_size = EWGET_DEFAULT_READ_BUFFER_SIZE;



static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


/* general utility functions */
#ifndef USE_ERICS_TOOLS
static char* dynamic_strcat(int num_strs, ...);  /* multi string concatenation function (uses dynamic memory allocation)*/
static void to_lowercase(char* str);
static char* dynamic_replace(char* template, char* old, char* new);
#endif
static char* http_unescape(const char* escaped);



static int char_index(char* str, int chr);
static char* encode_base_64_str( char* original, int linesize );
static void encode_block_base64( unsigned char in[3], unsigned char out[4], int len );
static char* escape_chars_to_hex(char* str, char* chars_to_escape);
static int tcp_connect(char* hostname, int port);


/* http read/write functions */
static void* initialize_connection_http(char* host, int port);
static int read_http(void* connection_data, char* read_buffer, int read_length);
static int write_http(void* connection_data, char* data, int data_length);
static void destroy_connection_http(void* connection_data);

/* functions for actually performing http request */
static char* create_http_request(url_request* url);
static void get_http_response(	void* connection_data, 
				int (*read_connection)(void*, char*, int), 
				http_response *reply, 
				FILE* header_stream, 
				FILE* body_stream, 
				FILE* combined_stream,
				int follow_redirects,
				char** redirect_url);
static http_response* retrieve_http(	url_request *url, 
					void* (*initialize_connection)(char*, int), 
					int (*read_connection)(void*, char*, int),
					int (*write_connection)(void*, char*, int),
					void (*destroy_connection)(void*),
					FILE* header_stream,
					FILE* body_stream,
					FILE* combined_stream,
					int alloc_response,
					int* ret_val,
					int follow_redirects,
					char** redirect_url
					);




/* alarm variable and signal handler for timeout */
static int alarm_socket = -1;
static int alarm_socket_closed=0;
static void alarm_triggered(int sig); 


/* SSL definitions */
#ifdef HAVE_SSL

	#ifdef USE_OPENSSL
		#include <openssl/ssl.h>
	#endif
	
	#ifdef USE_CYASSL
		#ifdef USE_CYASSL_INCLUDE_DIR
			#include <cyassl/openssl/ssl.h>
		#else
			#include <openssl/ssl.h>
		#endif
	#endif
	
	#ifdef USE_MBEDTLS	
	
		#define mbedtls_time_t     time_t
		#include <mbedtls/net.h>
		#include <mbedtls/ssl.h>
		#include <mbedtls/certs.h>
		#include <mbedtls/x509.h>
		#include <mbedtls/rsa.h>
		#include <mbedtls/error.h>
		#include <mbedtls/version.h>
		#include <mbedtls/entropy.h>

		static const int default_ciphersuites[] =
		{
			#if defined(MBEDTLS_AES_C)
			#if defined(MBEDTLS_SHA2_C)
			    MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA256,
			#endif /* MBEDTLS_SHA2_C */
			#if defined(MBEDTLS_GCM_C) && defined(MBEDTLS_SHA4_C)
			    MBEDTLS_TLS_RSA_WITH_AES_256_GCM_SHA384,
			#endif /* MBEDTLS_SHA2_C */
			    MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA,
			#endif
			#if defined(MBEDTLS_CAMELLIA_C)
			#if defined(MBEDTLS_SHA2_C)
			    MBEDTLS_TLS_RSA_WITH_CAMELLIA_256_CBC_SHA256,
			#endif /* MBEDTLS_SHA2_C */
			    MBEDTLS_TLS_RSA_WITH_CAMELLIA_256_CBC_SHA,
			#endif
			#if defined(MBEDTLS_AES_C)
			#if defined(MBEDTLS_SHA2_C)
			    MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA256,
			#endif /* MBEDTLS_SHA2_C */
			#if defined(MBEDTLS_GCM_C) && defined(MBEDTLS_SHA2_C)
			    MBEDTLS_TLS_RSA_WITH_AES_128_GCM_SHA256,
			#endif /* MBEDTLS_SHA2_C */
			    MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA,
			#endif
			#if defined(MBEDTLS_CAMELLIA_C)
			#if defined(MBEDTLS_SHA2_C)
			    MBEDTLS_TLS_RSA_WITH_CAMELLIA_128_CBC_SHA256,
			#endif /* MBEDTLS_SHA2_C */
			    MBEDTLS_TLS_RSA_WITH_CAMELLIA_128_CBC_SHA,
			#endif
			#if defined(MBEDTLS_DES_C)
			    MBEDTLS_TLS_RSA_WITH_3DES_EDE_CBC_SHA,
			#endif
			#if defined(MBEDTLS_ARC4_C)
			    MBEDTLS_TLS_RSA_WITH_RC4_128_SHA,
			    MBEDTLS_TLS_RSA_WITH_RC4_128_MD5,
			#endif
			    0
		};


		typedef mbedtls_ssl_context SSL;
		typedef struct pctx
		{
			int socket;
			mbedtls_ssl_config conf;
			mbedtls_x509_crt ssl_client_cert;
			mbedtls_pk_context key;
			mbedtls_ssl_session ssl_client_session;

		} SSL_CTX;
		
		static void SSL_free( SSL* ssl ) { mbedtls_ssl_free(ssl); free(ssl); }
		static void SSL_CTX_free(SSL_CTX* ctx) { free(ctx) ; }

		static int SSL_read( SSL* ssl, char *buf, size_t len ) { return mbedtls_ssl_read(ssl, (unsigned char*)buf, len); }
		static int SSL_write( SSL* ssl, char *buf, size_t len ) { return mbedtls_ssl_write(ssl, (unsigned char*)buf, len); }

	#endif
	

	#ifdef USE_MATRIXSSL
		#include "matrixssl_helper.h"
		typedef sslKeys_t SSL_CTX;
		static void SSL_CTX_free(SSL_CTX* ctx){ free(ctx); }
	#endif

	typedef struct
	{
		int socket;
		SSL* ssl;
		SSL_CTX* ctx;
	} ssl_connection_data;

	static void* initialize_connection_https(char* host, int port);
	static int read_https(void* connection_data, char* read_buffer, int read_length);
	static int write_https(void* connection_data, char* data, int data_length);
	static void destroy_connection_https(void* connection_data);

#endif
/* end SSL definitions */



/**********************************************
 * Externally available function definitions
 **********************************************/

http_response* get_url(char* url_str, char* user_agent)
{
	url_request* url = parse_url(url_str, user_agent);
	http_response *reply = get_url_request(&url);
	free_url_request(url);
	return reply;
}

http_response* get_url_request(url_request** url)
{
	http_response *reply = NULL;
	int dummy_ret;
	// TODO: Make this configurable
	int follow_redirects = 1;
	int redirect_count = 0;
	int redirect_max = 3;
	char* redirect_url = NULL;
	while(redirect_count < redirect_max)
	{
		if(redirect_url != NULL)
		{
			parse_redirect(url, redirect_url);
			redirect_url = NULL;
		}
		if((*url)->protocol == HTTP_PROTO)
		{
			reply = retrieve_http(*url, initialize_connection_http, read_http, write_http, destroy_connection_http, NULL, NULL, NULL, 1, &dummy_ret, follow_redirects, &redirect_url);
		}
		#ifdef HAVE_SSL
			if((*url)->protocol == HTTPS_PROTO)
			{
				reply = retrieve_http(*url, initialize_connection_https, read_https, write_https, destroy_connection_https, NULL, NULL, NULL, 1, &dummy_ret, follow_redirects, &redirect_url);
			}
		#endif
		if(redirect_url != NULL)
		{
			//process redirect
			redirect_count = redirect_count + 1;
			if(redirect_count == redirect_max)
			{
				//printf("ERROR: Too many redirects\n"); //This might interfere with some regex processing in other processes. Best to not print it.
			}
		}
		else
		{
			redirect_count = redirect_max;
		}
	}

	return reply;
}


int write_url_to_stream(char* url_str, char* user_agent, FILE* header_stream, FILE* body_stream, FILE* combined_stream)
{
	int ret;
	url_request* url = parse_url(url_str, user_agent);
	ret = write_url_request_to_stream(&url, header_stream, body_stream, combined_stream);
	free_url_request(url);
	return ret;
}

int write_url_request_to_stream(url_request** url, FILE* header_stream, FILE* body_stream, FILE* combined_stream)
{
	int ret = 1;
	// TODO: Make this configurable
	int follow_redirects = 1;
	int redirect_count = 0;
	int redirect_max = 3;
	char* redirect_url = NULL;
	while(redirect_count < redirect_max)
	{
		if(redirect_url != NULL)
		{
			parse_redirect(url, redirect_url);
			redirect_url = NULL;
		}
		if((*url)->protocol == HTTP_PROTO)
		{
			retrieve_http(*url, initialize_connection_http, read_http, write_http, destroy_connection_http, header_stream, body_stream, combined_stream, 0, &ret, follow_redirects, &redirect_url);
		}
		#ifdef HAVE_SSL
			if((*url)->protocol == HTTPS_PROTO)
			{
				retrieve_http(*url, initialize_connection_https, read_https, write_https, destroy_connection_https, header_stream, body_stream, combined_stream, 0, &ret, follow_redirects, &redirect_url);
			}
		#endif
		if(redirect_url != NULL)
		{
			//process redirect
			redirect_count = redirect_count + 1;
			if(redirect_count == redirect_max)
			{
				ret = 1;
				//printf("ERROR: Too many redirects\n"); //This might interfere with some regex processing in other processes. Best to not print it.
			}
		}
		else
		{
			redirect_count = redirect_max;
		}
	}

	return ret;

}




void free_http_response(http_response* page)
{
	if(page != NULL)
	{
		if(page->data != NULL)
		{
			free(page->data);
		}
		if(page->header != NULL)
		{
			free(page->header);
		}
		free(page);
	}
}



url_request* parse_url(char* url, char* user_agent)
{
	char* lower_url;
	char* remainder;
	url_request *new_url = (url_request*)malloc(sizeof(url_request));
	new_url->protocol = UNKNOWN_PROTO;
	new_url->user = NULL;
	new_url->password = NULL;
	new_url->hostname = NULL;
	new_url->port = -1;
	new_url->path = NULL;
	new_url->user_agent = user_agent == NULL ? strdup("ewget 1.0") : strdup(user_agent);


	if(url == NULL)
	{
		return new_url;
	}
	

	

	
	/* step 1, parse out protocol */
	lower_url = strdup(url);
	to_lowercase(lower_url);

	remainder = NULL;
	if(strstr(lower_url, "http://") == lower_url)
	{
		new_url->protocol = HTTP_PROTO;
		new_url->port = 80;
		remainder = url+7;
	}
	else if(strstr(lower_url, "https://") == lower_url)
	{
		new_url->protocol = HTTPS_PROTO;
		new_url->port = 443;
		remainder = url+8;
	}
	else if(strstr(lower_url, "://") == NULL) /*if no prefix provided assume HTTP */
	{
		new_url->protocol = HTTP_PROTO;
		new_url->port = 80;
		remainder = url;
	}
	free(lower_url);


	if(remainder != NULL) /* if protocol is defined as something we don't support (e.g. ftp) do not parse url and return NULL */
	{
		char escape_chars[] = "\n\r\t\"\\ []<>{}|^~`:,";
		int port_begin;
		
		
		/* step 2, parse out user/password if present */
		int path_begin = char_index(remainder, '/');
		int user_pass_end = char_index(remainder, '@');
		path_begin = path_begin >= 0 ? path_begin : strlen(remainder);

		if(user_pass_end >= 0 && user_pass_end < path_begin)
		{
			/* found user/password */
			char* unescaped_user;
			int user_end = char_index(remainder, ':');	
			user_end = user_end < user_pass_end && user_end >= 0 ? user_end : user_pass_end;
			new_url->user = (char*)malloc((user_end+1)*sizeof(char));
			memcpy(new_url->user, remainder, user_end);
			(new_url->user)[user_end] = '\0';
			unescaped_user = http_unescape(new_url->user);
			free(new_url->user);
			new_url->user = unescaped_user;
			
			if(user_end != user_pass_end)
			{
				char* unescaped_pass;
				int pass_length = user_pass_end-user_end-1;
				new_url->password = (char*)malloc((pass_length+1)*sizeof(char));
				memcpy(new_url->password, remainder+user_end+1, pass_length);
				(new_url->password)[pass_length] = '\0';
				unescaped_pass = http_unescape(new_url->password);
				free(new_url->password);
				new_url->password = unescaped_pass;
			}
			

			remainder = remainder + user_pass_end + 1;
			path_begin = char_index(remainder, '/');
			path_begin = path_begin >= 0 ? path_begin : strlen(remainder);
		}
		
		/*
		 * step 3, parse out hostname & port, we escape characters after this point since
		 * this will be included directly in GET request
		 * this dynamicaly allocates memory , remember to free url at end
		 */
		port_begin = char_index(remainder, ':');
		
		
		
		if(port_begin >= 0 && port_begin < path_begin )
		{
			char* raw_host = (char*)malloc((port_begin+1)*sizeof(char));
			memcpy(raw_host, remainder, port_begin);
			raw_host[port_begin] = '\0';
			new_url->hostname = escape_chars_to_hex(raw_host, escape_chars);
			free(raw_host);

			if(path_begin-port_begin-1 <= 5)
			{
				int read;
				char port_str[6];
				memcpy(port_str, remainder+port_begin+1, path_begin-port_begin-1);
				port_str[ path_begin-port_begin-1] = '\0';
				
				if(sscanf(port_str, "%d", &read) > 0)
				{
					if(read <= 65535)
					{
						new_url->port = read;
					}
				}
			}
		}
		else
		{
			char* raw_host = (char*)malloc((path_begin+1)*sizeof(char));
			memcpy(raw_host, remainder, path_begin);
			raw_host[path_begin] = '\0';
			new_url->hostname = escape_chars_to_hex(raw_host, escape_chars);
			free(raw_host);
		}
		
		remainder = remainder + path_begin;
		url = escape_chars_to_hex(remainder, escape_chars);
		remainder = url;
		
		
		if(remainder[0] != '/')
		{
			new_url->path = (char*)malloc(2*sizeof(char));
			(new_url->path)[0] = '/';
			(new_url->path)[1] = '\0';
		}
		else
		{
			/* be a little more agressive in escaping path string */
			new_url->path = strdup(remainder);
		}
		free(url); /* free memory allocated from escaping characters */
	
	}
	return new_url;
}


void free_url_request(url_request* url)
{
	if(url != NULL)
	{
		if(url->user != NULL)
		{
			free(url->user);
		}
		if(url->password != NULL)
		{
			free(url->password);
		}
		if(url->hostname != NULL)
		{
			free(url->hostname);
		}
		if(url->path != NULL)
		{
			free(url->path);
		}
		if(url->user_agent != NULL)
		{
			free(url->user_agent);
		}
		free(url);
	}
}



void set_ewget_read_buffer_size(unsigned long size)
{
	__ewget_read_buffer_size = size;
}
void set_ewget_timeout_seconds(unsigned long seconds)
{
	__ewget_timeout_seconds  = seconds;
}
unsigned long get_ewget_read_buffer_size(void)
{
	return __ewget_read_buffer_size;
}
unsigned long get_ewget_timeout_seconds(void)
{
	return __ewget_timeout_seconds;
}



/**********************************************
 * Internal function definitions
 **********************************************/


/* substitute [%00 - %ff] with related Glyph char */
static char* http_unescape(const char* escaped)
{
	char* unescaped = strdup(escaped);
	char* s = (char*)escaped;

	char old[4];
	char new[2];
	char* new_unescaped;

	while( *s != '\0')
	{  
		/* hex-encoded char found*/
		if(*s == '%')
		{
		       	unsigned int new_char_num;
			strncpy(old, s, 3);
			old[3] = '\0';
			sscanf(old+1, "%2X", &new_char_num);
			if('%' != (char)new_char_num)
			{
				sprintf(new, "%c", (char)new_char_num);
				new_unescaped = dynamic_replace(unescaped, old, new);
				free(unescaped);
				unescaped = new_unescaped;
			}
		}
		s = *s == '%' ? s+3 : s+1;
	}

	/* be sure to do '%' LAST, otherwise everthing gets FUBAR */
	sprintf(old, "%%%.2X", (int)'%');
	sprintf(new, "%c", '%');
	new_unescaped = dynamic_replace(unescaped, old, new);
	free(unescaped);
	unescaped = new_unescaped;

	return unescaped;
}





static void alarm_triggered(int sig)
{
	if(alarm_socket >= 0)
	{
		close(alarm_socket);
		alarm_socket = -1;
		alarm_socket_closed = 1;
	}
}

void parse_redirect(url_request** url, char* redirect_url)
{
	url_request* new_url = parse_url(redirect_url, (*url)->user_agent);
	//transfer username and pass
	if((*url)->user != NULL)
	{
		if(new_url->user != NULL) free(new_url->user);
		new_url->user = (*url)->user;
		(*url)->user = NULL;
	}
	if((*url)->password != NULL)
	{
		if(new_url->password != NULL) free(new_url->password);
		new_url->password = (*url)->password;
		(*url)->password = NULL;
	}
	free_url_request(*url);
	
	*url = new_url;
}


static http_response* retrieve_http(	url_request *url, 
					void* (*initialize_connection)(char*, int), 
					int (*read_connection)(void*, char*, int),
					int (*write_connection)(void*, char*, int),
					void (*destroy_connection)(void*),
					FILE* header_stream,
					FILE* body_stream,
					FILE* combined_stream,
					int alloc_response,
					int* ret_val,
					int follow_redirects,
					char** redirect_url
					)
{
	http_response *reply = NULL;
	*ret_val  = 1;

	if(url->hostname != NULL && url->port >= 0 && url->path != NULL)
	{
		void* connection_data = initialize_connection(url->hostname, url->port);
		if(connection_data != NULL)
		{
			char* request = create_http_request(url);
			
			/* send request */
			int test = write_connection(connection_data, request, strlen(request));
			if(test == strlen(request))
			{
				if(alloc_response)
				{
					reply = (http_response*)malloc(sizeof(http_response));
					reply->data = NULL;
					reply->header = NULL;
				}
				*ret_val = 0;
				get_http_response(connection_data, read_connection, reply, header_stream, body_stream, combined_stream, follow_redirects, redirect_url);
				if(*redirect_url != NULL)
				{
					//process redirect
					free_http_response(reply);
					reply = NULL;
					*ret_val = 1;
				}
			}
			free(request);
			destroy_connection(connection_data);
		}
	}
	return reply;
}





static char* create_http_request(url_request* url)
{
	char* req_str1;
	char* req_str2;
	char port_str[8];


	url->user_agent = url->user_agent == NULL ? strdup("ewget 1.0") : url->user_agent;

	req_str1 = dynamic_strcat(	
					10,
					"GET ",	url->path, " HTTP/1.0\r\n", 
					"User-Agent: ", url->user_agent, "\r\n", 
					"Accept: */*\r\n", 
					"Connection: close\r\n", 
					"Host: ", url->hostname
					);

	if( (url->protocol == HTTP_PROTO && url->port != 80) || (url->protocol == HTTPS_PROTO && url->port != 443) )
	{
		sprintf(port_str, ":%d\r\n", url->port);
	}
	else
	{
		sprintf(port_str, "\r\n");
	}
	req_str2 = dynamic_strcat(2, req_str1, port_str);
	free(req_str1);

		
	if(url->user != NULL)
	{
		char* encoded_auth;
		char* plain_auth = NULL;
		if(url->password == NULL)
		{
			plain_auth = strdup(url->user);
		}
		else
		{
			plain_auth = dynamic_strcat(3, url->user, ":", url->password);
		}
		encoded_auth = encode_base_64_str(plain_auth, 999999);
		

		req_str1 = dynamic_strcat(4, req_str2, "Authorization: Basic ", encoded_auth, "\r\n");
		free(req_str2);
		req_str2 = req_str1;

		free(plain_auth);
		free(encoded_auth);
	}
	req_str1 = dynamic_strcat(2, req_str2, "\r\n");
	free(req_str2);

	return req_str1;
}


static void get_http_response(void* connection_data, int (*read_connection)(void*, char*, int), http_response *reply, FILE* header_stream, FILE* body_stream, FILE* combined_stream, int follow_redirects, char** redirect_url)
{
	unsigned long  read_buffer_size = __ewget_read_buffer_size ;
	int bytes_read;
	int header_bytes_read = 0;
	int body_bytes_read = 0;
	int reading_header=1;
	char* read_buffer = (char*)malloc((read_buffer_size+1)*sizeof(char));
	char last_two_bytes_of_old_read_buffer[3] = { '\0', '\0', '\0' }; 
	

	if(reply != NULL)
	{
		if(reply->header != NULL) { free(reply->header); }
		if(reply->data   != NULL) { free(reply->data);   }
		reply->header  = NULL;
		reply->data    = NULL;
		reply->is_text = 0;
		reply->length  = 0;
	}


	bytes_read = read_connection(connection_data, read_buffer, read_buffer_size);
	bytes_read = bytes_read < 0 ? 0 : bytes_read;
	read_buffer[bytes_read] = '\0'; /* facilitates header string processing */
	while(bytes_read > 0)
	{
		int header_length = 0;
		char* body_start = read_buffer;
		if(reading_header)
		{
			char* cr_end = NULL;
			char* lf_end = NULL;

			/* set cr_end and lf_end */
			if(cr_end == NULL && lf_end == NULL && last_two_bytes_of_old_read_buffer[0] == '\n' && last_two_bytes_of_old_read_buffer[1] == '\r')
			{
				cr_end = read_buffer[0] == '\n' ? read_buffer+1 : cr_end;
			}
			if(cr_end == NULL && lf_end == NULL && last_two_bytes_of_old_read_buffer[1] == '\n')
			{
				cr_end = read_buffer[0] == '\r' && read_buffer[1] == '\n' ? read_buffer+2 : cr_end;
				lf_end = read_buffer[0] == '\n' ? read_buffer+1 : lf_end;
			}
			if(cr_end == NULL && lf_end == NULL)
			{
				cr_end = strstr(read_buffer, "\n\r\n");
				lf_end = strstr(read_buffer, "\n\n");
				cr_end = cr_end != NULL ? cr_end+3 : cr_end;
				lf_end = lf_end != NULL ? lf_end+2 : lf_end;
			}

			
			/* determine header_length given cr_end and lf_end */	
			if(cr_end != NULL && lf_end != NULL)
			{
				body_start = cr_end < lf_end ? cr_end : lf_end;
			}
			else
			{
				body_start = cr_end != NULL ? cr_end : lf_end;
			}
			header_length = body_start != NULL ? (int)(body_start - read_buffer) : bytes_read;


			/* check for redirects before continuing */
			if(follow_redirects)
			{
				char* tmpheader = malloc((header_length+1)*sizeof(char));
				if(tmpheader != NULL)
				{
					memcpy(tmpheader, read_buffer, header_length);
					tmpheader[header_length] = '\0';
	
					char* statusline_start = strstr(tmpheader, "HTTP/1.");
					if(statusline_start != NULL)
					{
						//Format should be: HTTP/1.x xxx Reason Phrase. We need to retrieve xxx which should always be at a standard offset...
						int status_code = 0;
						sscanf(statusline_start+9, "%3d", &status_code);
						if(status_code == 301 || status_code == 302 || status_code == 307)
						{
							//There is also the Content-Location: field, which should always be after this, but we won't factor that in here.
							char* location_start = strstr(tmpheader, "Location:");
							if(location_start != NULL)
							{
								int location_end = char_index(location_start, '\r');
								location_end = location_end - 10;
								char* location = malloc((location_end+1)*sizeof(char));
								memcpy(location, location_start+10, location_end);
								location[location_end] = '\0';
								*redirect_url = location;
								free(tmpheader);
								free(read_buffer);
								return;
							}
						}
					}
					free(tmpheader);
				}
			}

			/* write header data to streams and/or http response*/
			if(header_stream != NULL)
			{
				fwrite(read_buffer, 1, header_length, header_stream);
			}
			if(combined_stream != NULL)
			{
				fwrite(read_buffer, 1, header_length, combined_stream);
			}
			if(reply != NULL)
			{
				char* header = (char*)malloc((header_bytes_read+header_length+1)*sizeof(char));
				char* content_start;
				if(header_bytes_read > 0)
				{
					memcpy(header, reply->header, header_bytes_read);
					free(reply->header);
				}
				memcpy(header+header_bytes_read, read_buffer, header_length);
				header[header_bytes_read+header_length] = '\0';
				reply->header = strdup(header);
				

				header_bytes_read = header_bytes_read + header_length;


				to_lowercase(header);
				content_start = strstr(header, "content-type:");
				if(content_start != NULL)
				{
					int content_end = char_index(content_start, '\n');
					char *content = (char*)malloc((content_end+1)*sizeof(char));
					memcpy(content, content_start, content_end);
					content[content_end] = '\0';
					reply->is_text = strstr(content, "text") == NULL ? 0 : 1;
					free(content);
				}
				free(header);

			}

			/* if end of header detected, update for that */
			if(body_start != NULL)
			{
				reading_header = 0;
			}
		}

		/* write body data to streams(s) and/or http response */
		if(!reading_header)
		{
			int body_length = bytes_read - header_length;
			if(body_stream != NULL)
			{
				fwrite(body_start, 1, body_length, body_stream);
			}
			if(combined_stream != NULL)
			{
				fwrite(body_start, 1, body_length, combined_stream);
			}
			if(reply != NULL)
			{
				char* body = (char*)malloc((body_bytes_read+body_length)*sizeof(char));
				if(body_bytes_read > 0)
				{
					memcpy(body, reply->data, body_bytes_read);
					free(reply->data);
				}
				memcpy(body+body_bytes_read, body_start, body_length);
				body_bytes_read = body_bytes_read + body_length;
				reply->data = body;
				reply->length = body_bytes_read;
			}
		}
		
		last_two_bytes_of_old_read_buffer[0] = bytes_read == 1 ? '\0' : read_buffer[bytes_read-2];
		last_two_bytes_of_old_read_buffer[1] = read_buffer[bytes_read-1];

		bytes_read = read_connection(connection_data, read_buffer, read_buffer_size);
		bytes_read = bytes_read < 0 ? 0 : bytes_read;
		read_buffer[bytes_read] = '\0'; /* facilitates header string processing */
	}
	free(read_buffer);

}

/*

static http_response* get_http_response(void* connection_data, int (*read_connection)(void*, char*, int))
{


	char* http_data = NULL;
	int read_buffer_size = DEFAULT_READ_BUFFER_SIZE;
	char* read_buffer = (char*)malloc((read_buffer_size+1)*sizeof(char));
	int total_bytes_read = 0;
	int bytes_read;
	
	http_response *reply = (http_response*)malloc(sizeof(http_response));
	reply->header = NULL;
	reply->data = NULL;
	reply->is_text = 0;
	reply->length = 0;


	bytes_read = read_connection(connection_data, read_buffer, read_buffer_size);
	bytes_read = bytes_read < 0 ? 0 : bytes_read;
	read_buffer[bytes_read] = '\0'; // facilitates string processing 
	while(bytes_read > 0)
	{
		int updated_header = 0;
		if(reply->header == NULL)
		{
			int header_end = -1;
			char* cr_end = strstr(read_buffer, "\n\r\n");
			char* lf_end = strstr(read_buffer, "\n\n");
			if(cr_end != NULL && lf_end != NULL)
			{
				char *first_end = cr_end < lf_end ? cr_end : lf_end;
				int modifier = cr_end < lf_end ? 2 : 1;
				header_end = modifier+ (int)(first_end - read_buffer);
			}
			else if(cr_end != NULL)
			{
				header_end = 2+ (int)(cr_end - read_buffer);
			}
			else if(lf_end != NULL)
			{
				header_end = 1 + (int)(lf_end - read_buffer);
			}
			if(header_end < 0 && bytes_read < read_buffer_size)
			{
				header_end = bytes_read-1;
			}
			if(header_end > 0)
			{
				char* header = (char*)malloc((total_bytes_read+header_end+1)*sizeof(char));
				char* content_start;
				
				updated_header = 1;
			
				if(total_bytes_read > 0)
				{
					memcpy(header, http_data, total_bytes_read);
				}
				memcpy(header, read_buffer, header_end);
				header[total_bytes_read+header_end] = '\0';
				reply->header= strdup(header);

				free(http_data);
				total_bytes_read = (bytes_read-(header_end+1)) >= 0 ? (bytes_read-(header_end+1)) : 0;
				http_data = (char*)malloc( (total_bytes_read+1)*sizeof(char));
				memcpy(http_data, read_buffer+header_end+1, total_bytes_read );
				
				to_lowercase(header);
				content_start = strstr(header, "content-type:");
				if(content_start != NULL)
				{
					int content_end = char_index(content_start, '\n');
					char *content = (char*)malloc((content_end+1)*sizeof(char));
					memcpy(content, content_start, content_end);
					content[content_end] = '\0';
					reply->is_text = strstr(content, "text") == NULL ? 0 : 1;
					free(content);
				}
				free(header);
			}
		}
		if(updated_header ==0)
		{
			char* old_http_data = http_data;
			http_data = (char*)malloc((total_bytes_read + bytes_read+1)*sizeof(char));
			memcpy(http_data, old_http_data, total_bytes_read);
			memcpy(http_data+total_bytes_read, read_buffer, bytes_read);
			if(old_http_data != NULL)
			{
				free(old_http_data);
			}
			total_bytes_read = total_bytes_read + bytes_read;
		}
		bytes_read=read_connection(connection_data, read_buffer, read_buffer_size);
		bytes_read = bytes_read < 0 ? 0 : bytes_read;
		read_buffer[bytes_read] = '\0'; // facilitates string processing 
	}
	if(http_data == NULL)
	{
		http_data = (char*)malloc(sizeof(char));
	}
	http_data[total_bytes_read] = '\0';
	reply->length = total_bytes_read;
	reply->data = http_data;

	free(read_buffer);
	return reply;
}

*/


static int char_index(char* str, int ch)
{
	char* result = strchr(str, ch);
	int return_value = result == NULL ? -1 : (int)(result - str);
	return return_value;
}


static int tcp_connect(char* hostname, int port)
{
	struct hostent* host;
	int sockfd;
	struct sockaddr_in address;
	long arg = 0;
	int connection;



	if(hostname == NULL)
	{
		return -1;
	}
       	host = gethostbyname(hostname);
	if(host == NULL)
	{
		return -1;
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		return -1;
	}
	

	address.sin_family= AF_INET;
	address.sin_addr.s_addr = ((struct in_addr *)host->h_addr)->s_addr;
	address.sin_port = htons(port); /* htons is necessary -- it makes sure byte ordering is correct */

	/* Set non-blocking */
	arg = 0;
	if( (arg = fcntl(sockfd, F_GETFL, NULL)) < 0)
	{
	       	close(sockfd);
		return -1;
	}
	arg |= O_NONBLOCK; 
  	if( fcntl(sockfd, F_SETFL, arg) < 0)
	{
	       	close(sockfd);
		return -1;
	}	
	

	connection = connect(sockfd, (struct sockaddr *)&address, sizeof(address));
	if(connection < 0)
	{
		if (errno == EINPROGRESS)
		{ 
			fd_set myset; 
			struct timeval tv;
			int valopt;
			
      			tv.tv_sec = __ewget_timeout_seconds; 
			tv.tv_usec = 0; 
			FD_ZERO(&myset); 
			FD_SET(sockfd, &myset); 
			connection = select(sockfd+1, NULL, &myset, NULL, &tv); 
			while(connection < 0 && errno != EINTR)
			{
				connection = select(sockfd+1, NULL, &myset, NULL, &tv); 
			}
			
           		if (connection > 0)
			{
				/* Socket selected for write */
				socklen_t sock_len = sizeof(int); 
				if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &sock_len) < 0)
				{
					close(sockfd);
					return -1;
				}
				/* Check the value returned... */
				if (valopt) 
				{
					close(sockfd);
                 			return -1;
				}
			}
			else
			{
				close(sockfd);
				return -1;
			}
		}
		else
		{
			close(sockfd);
			return -1;
		}
	
	} 
	

	/* Set to blocking mode again... */
	if( (arg = fcntl(sockfd, F_GETFL, NULL)) < 0)
	{
		close(sockfd);
		return -1;	
	} 
	arg &= (~O_NONBLOCK); 
	if( fcntl(sockfd, F_SETFL, arg) < 0)
	{
		close(sockfd);
		return -1;
	}


	return sockfd;
}


static char* escape_chars_to_hex(char* str, char* chars_to_escape)
{
	
	char* new_str = NULL;
	if(str != NULL)
	{
		if(chars_to_escape == NULL)
		{
			new_str = strdup(str);
		}
		else
		{
			int last_piece_start = 0;
			int str_index;
			new_str = strdup("");
			for(str_index = 0; str[str_index] != '\0'; str_index++)
			{
				int found = 0;
				int escape_index;
				for(escape_index = 0; chars_to_escape[escape_index] != '\0' && found == 0; escape_index++)
				{
					found = chars_to_escape[escape_index] == str[str_index] ? 1 : 0;
				}
				if(found == 1)
				{
					int last_piece_length = str_index - last_piece_start;
					char* last_piece = (char*)malloc((1+last_piece_length)*sizeof(char));
					char buf[5];
					char* old_str;

					memcpy(last_piece, str+last_piece_start, last_piece_length);
					last_piece[last_piece_length] = '\0';
						
					sprintf(buf, "%%%X", str[str_index]);

					old_str = new_str;
					new_str = dynamic_strcat(3, old_str, last_piece, buf);
					free(old_str);
					last_piece_start = str_index+1;
				}	
			}
			if(last_piece_start < str_index)
			{
				char* old_str = new_str;
				new_str = dynamic_strcat(2, old_str, str+last_piece_start);
				free(old_str);
			}
		}	
	}	
	return new_str;
}


static char* encode_base_64_str( char* original, int linesize )
{
	unsigned char in[3];
	unsigned char out[4];
	int i, len, blocksout = 0;

	
	char* encoded;
	int original_index = 0; 
	int encoded_index = 0;

	if(original == NULL)
	{
		encoded = (char*)malloc(sizeof(char));
		encoded[0] = '\0';
	}
	else
	{
		encoded = (char*)malloc( ((4*strlen(original)/3)+4)*sizeof(char) );
		if(original[original_index] == '\0')
		{
			encoded[0] = '\0';
		}
		while( original[original_index] != '\0')
		{
			len = 0;
			for(i=0; i < 3; i++)
			{
				in[i] = original[original_index];
				len = original[original_index] == '\0' ? len : len+1;
				original_index = original[original_index] == '\0' ? original_index : original_index+1;
			}
			if(len)
			{
				encode_block_base64(in, out, len);
				for(i=0; i < 4; i++)
				{
					encoded[encoded_index] = out[i];
					encoded_index++;
				}
				blocksout++;
			}
			if(blocksout >= (linesize/4))
			{
				encoded[encoded_index] = '\n';
				encoded_index++;
			}
		}
		encoded[encoded_index] = '\0';
	}
	return encoded;
}

/* encode 3 8-bit binary bytes as 4 '6-bit' characters */
static void encode_block_base64( unsigned char in[3], unsigned char out[4], int len )
{
    out[0] = cb64[ in[0] >> 2 ];
    out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

#ifndef USE_ERICS_TOOLS


char* dynamic_replace(char* template_str, char* old, char* new)
{
	char *ret;
	int i, count = 0;
	int newlen = strlen(new);
	int oldlen = strlen(old);

	char* dyn_template = strdup(template_str);
	char* s = dyn_template;
	for (i = 0; s[i] != '\0'; i++)
	{
		if (strstr(&s[i], old) == &s[i])
		{
			count++;
			i += oldlen - 1;
		}
	}
	ret = malloc(i + 1 + count * (newlen - oldlen));

	i = 0;
	while (*s)
	{
		if (strstr(s, old) == s)
		{
			strcpy(&ret[i], new);
			i += newlen;
			s += oldlen;
		}
		else
		{
			ret[i++] = *s++;
		}
	}
	ret[i] = '\0';
	free(dyn_template);

	return ret;
}


static void to_lowercase(char* str)
{
	int i;
	for(i = 0; str[i] != '\0'; i++)
	{
		str[i] = tolower(str[i]);
	}
}


char* dynamic_strcat(int num_strs, ...)
{
	
	va_list strs;
	int new_length = 0;
	int i;
	int next_start;
	char* new_str;
		
	va_start(strs, num_strs);
	for(i=0; i < num_strs; i++)
	{
		char* next_arg = va_arg(strs, char*);
		if(next_arg != NULL)
		{
			new_length = new_length + strlen(next_arg);
		}
	}
	va_end(strs);
	
	new_str = malloc((1+new_length)*sizeof(char));
	va_start(strs, num_strs);
	next_start = 0;
	for(i=0; i < num_strs; i++)
	{
		char* next_arg = va_arg(strs, char*);
		if(next_arg != NULL)
		{
			int next_length = strlen(next_arg);
			memcpy(new_str+next_start,next_arg, next_length);
			next_start = next_start+next_length;
		}
	}
	new_str[next_start] = '\0';
	
	return new_str;
}


#endif


/* returns data upon success, NULL on failure */
static void* initialize_connection_http(char* host, int port)
{
	int *socket = (int*)malloc(sizeof(int));
	
	/*
	* as soon as we open, or attempt to open a connection
	* indicate that the latest connection has not had
	* a timeout/close triggered on it
	*/
	alarm_socket_closed = 0;
	*socket	= tcp_connect(host, port);

	if(*socket >= 0)
	{	
		return socket;
	}
	else
	{
		free(socket);
		return NULL;
	}
}

static int read_http(void* connection_data, char* read_buffer, int read_length)
{
	int* socket = (int*)connection_data;
	int bytes_read = -1;

	alarm_socket = *socket;	
	signal(SIGALRM, alarm_triggered );
	alarm(__ewget_timeout_seconds);
	bytes_read = read(*socket, read_buffer, read_length);
	alarm(0);

	return bytes_read;

}
static int write_http(void* connection_data, char* data, int data_length)
{
	int* socket = (int*)connection_data;
	return write(*socket, data, data_length);
}

static void destroy_connection_http(void* connection_data)
{
	if(connection_data != NULL)
	{
		int* socket = (int*)connection_data;
		if(alarm_socket_closed == 0)
		{
			close(*socket);
		}
		free(socket);
		alarm_socket_closed = 0;
	}
}

#ifdef HAVE_SSL

#ifdef USE_MBEDTLS

static int urandom_fd = -1;
static int ewget_urandom_init(void)
{
	if (urandom_fd > -1)
	{
		return 1;
	}

	urandom_fd = open("/dev/urandom", O_RDONLY);
	if (urandom_fd < 0)
	{
		return -1;
	}

	return 1;
}
static int ewget_urandom(void *ctx, unsigned char *out, size_t len)
{
	if (read(urandom_fd, out, len) < 0)
		return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;

	return 0;
}
#endif



static void* initialize_connection_https(char* host, int port)
{
	ssl_connection_data* connection_data = NULL;
	int socket;

	/*
	* as soon as we open, or attempt to open a connection
	* indicate that the latest connection has not had
	* a timeout/close triggered on it
	*/
	alarm_socket_closed = 0;
	socket = tcp_connect(host, port);

	if(socket >= 0)
	{
		int initialized = -1;
		SSL* ssl = NULL;
		SSL_CTX* ctx = NULL;
		#ifdef USE_OPENSSL
			const SSL_METHOD* meth;
			SSL_library_init();
			SSL_load_error_strings();
			
			meth = SSLv23_method();
			ctx = SSL_CTX_new(meth);
    			ssl = SSL_new(ctx);
			SSL_set_fd(ssl, socket);
			initialized = SSL_connect(ssl);
			/* would check cert here if we were doing it */
		#endif

		#ifdef USE_MBEDTLS
			if(ewget_urandom_init() > 0)
			{
				ssl = (SSL*)malloc(sizeof(SSL));
				memset(ssl, 0, sizeof(SSL));
				
				ctx = (SSL_CTX*)malloc(sizeof(SSL_CTX));	
				memset(ctx, 0, sizeof(SSL_CTX));
				mbedtls_pk_init(&ctx->key);


				/* if(mbedtls_ssl_init(ssl) == 0) */
				mbedtls_ssl_init(ssl);
				mbedtls_ssl_config_init(&(ctx->conf));
				mbedtls_ssl_config_defaults( &(ctx->conf),
					MBEDTLS_SSL_IS_CLIENT,
					MBEDTLS_SSL_TRANSPORT_STREAM,
					MBEDTLS_SSL_PRESET_DEFAULT );
				
				mbedtls_ssl_conf_endpoint(&(ctx->conf), MBEDTLS_SSL_IS_CLIENT);
				mbedtls_ssl_conf_authmode(&(ctx->conf), MBEDTLS_SSL_VERIFY_NONE);
				mbedtls_ssl_conf_rng(&(ctx->conf), ewget_urandom, NULL);
				mbedtls_ssl_conf_own_cert(&(ctx->conf), &(ctx->ssl_client_cert), &(ctx->key)); 
				mbedtls_ssl_conf_ciphersuites(&(ctx->conf), default_ciphersuites);
				mbedtls_ssl_setup( ssl, &(ctx->conf) );
				
				ctx->socket = socket;
				mbedtls_ssl_set_bio(ssl, &(ctx->socket), mbedtls_net_send, mbedtls_net_recv, NULL);
			
				mbedtls_ssl_session_reset(ssl);
				initialized = mbedtls_ssl_handshake(ssl);
				
			}
		#endif



		#ifdef USE_CYASSL
			SSL_METHOD*  method  = 0;
			#if defined(CYASSL_DTLS)
				method  = DTLSv1_client_method();
			#elif  !defined(NO_TLS)
				method  = TLSv1_client_method();
			#else
				method  = SSLv3_client_method();
			#endif
			ctx = SSL_CTX_new(method);
			SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);
    			ssl = SSL_new(ctx);
			SSL_set_fd(ssl, socket);
			initialized = SSL_connect(ssl);
		#endif

		#ifdef USE_MATRIXSSL
			matrixSslOpen();
			initialized = -1;
		       	if(matrixSslOpen() >= 0)
			{
				int key_test = matrixSslReadKeys(&ctx, NULL, NULL, NULL, NULL);
				ssl = SSL_new(ctx, 0);
				if(ssl != NULL && key_test >=0)
				{
					SSL_set_fd(ssl, socket);
					/* would define cert checker here if we were doing it */
					initialized = SSL_connect(ssl, NULL, NULL); 
				}
			}
		#endif
		if(initialized >= 0)
		{
			connection_data = (ssl_connection_data*)malloc(sizeof(ssl_connection_data));
			connection_data->socket = socket;
			connection_data->ssl = ssl;
			connection_data->ctx = ctx;
		}
		else
		{
			close(socket);
			SSL_free(ssl);
			SSL_CTX_free(ctx);
			#ifdef USE_MATRIXSSL
				matrixSslClose();
			#endif

		}
	}


	return connection_data;
}
static int read_https(void* connection_data, char* read_buffer, int read_length)
{
	ssl_connection_data *cd = (ssl_connection_data*)connection_data;
	int bytes_read = -1;
	
	
	alarm_socket = cd->socket;	
	signal(SIGALRM, alarm_triggered );
	alarm(__ewget_timeout_seconds);
	bytes_read = SSL_read(cd->ssl, read_buffer, read_length);
	alarm(0);

	return bytes_read;

}
static int write_https(void* connection_data, char* data, int data_length)
{
	ssl_connection_data *cd = (ssl_connection_data*)connection_data;
	return SSL_write(cd->ssl, data, data_length);
}
static void destroy_connection_https(void* connection_data)
{
	if(connection_data != NULL)
	{
		ssl_connection_data *cd = (ssl_connection_data*)connection_data;
		if(alarm_socket_closed == 0)
		{
			close(cd->socket);
		}
		alarm_socket_closed = 0;
		
		SSL_free(cd->ssl);
		SSL_CTX_free(cd->ctx);
		free(cd);
		#ifdef USE_MATRIXSSL
			matrixSslClose();
		#endif
	}
}

#endif /* end HAVE_SSL definitions */

