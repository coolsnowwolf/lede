
/*
 * Copyright (C) Roman Arutyunyan
 */


#ifndef _NGX_RTMP_H_INCLUDED_
#define _NGX_RTMP_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_connect.h>
#include <nginx.h>
#include <ngx_http.h>

#include "ngx_rtmp_amf.h"
#include "ngx_rtmp_bandwidth.h"
#include "ngx_http_client.h"
#include "ngx_netcall.h"
#include "ngx_map.h"


#if (NGX_WIN32)
typedef __int8              int8_t;
typedef unsigned __int8     uint8_t;
#endif


#if (NGX_PCRE)

typedef struct {
    ngx_uint_t                    capture;
    ngx_int_t                     index;
} ngx_rtmp_regex_variable_t;

typedef struct {
    ngx_regex_t                *regex;
    ngx_uint_t                  ncaptures;
    ngx_rtmp_regex_variable_t  *variables;
    ngx_uint_t                  nvariables;
    ngx_str_t                   name;
} ngx_rtmp_regex_t;

typedef struct {
    ngx_rtmp_regex_t             *regex;
    void                         *value;
} ngx_rtmp_map_regex_t;

#endif


typedef struct {
    void                  **main_conf;
    void                  **srv_conf;
    void                  **app_conf;
} ngx_rtmp_conf_ctx_t;


typedef struct {
    void                   *addrs;
    ngx_uint_t              naddrs;
} ngx_rtmp_port_t;


typedef struct {
    int                     family;
    in_port_t               port;
    ngx_array_t             addrs;       /* array of ngx_rtmp_conf_addr_t */
} ngx_rtmp_conf_port_t;


#define NGX_RTMP_VERSION                3

#define NGX_LOG_DEBUG_RTMP              NGX_LOG_DEBUG_CORE

#define NGX_RTMP_DEFAULT_CHUNK_SIZE     128


/* RTMP message types */
#define NGX_RTMP_MSG_CHUNK_SIZE         1
#define NGX_RTMP_MSG_ABORT              2
#define NGX_RTMP_MSG_ACK                3
#define NGX_RTMP_MSG_USER               4
#define NGX_RTMP_MSG_ACK_SIZE           5
#define NGX_RTMP_MSG_BANDWIDTH          6
#define NGX_RTMP_MSG_EDGE               7
#define NGX_RTMP_MSG_AUDIO              8
#define NGX_RTMP_MSG_VIDEO              9
#define NGX_RTMP_MSG_AMF3_META          15
#define NGX_RTMP_MSG_AMF3_SHARED        16
#define NGX_RTMP_MSG_AMF3_CMD           17
#define NGX_RTMP_MSG_AMF_META           18
#define NGX_RTMP_MSG_AMF_SHARED         19
#define NGX_RTMP_MSG_AMF_CMD            20
#define NGX_RTMP_MSG_AGGREGATE          22
#define NGX_RTMP_MSG_MAX                22

#define NGX_RTMP_CONNECT                NGX_RTMP_MSG_MAX + 1
#define NGX_RTMP_DISCONNECT             NGX_RTMP_MSG_MAX + 2
#define NGX_RTMP_HANDSHAKE_DONE         NGX_RTMP_MSG_MAX + 3
#define NGX_MPEGTS_MSG_AUDIO            NGX_RTMP_MSG_MAX + 4
#define NGX_MPEGTS_MSG_VIDEO            NGX_RTMP_MSG_MAX + 5
#define NGX_MPEGTS_MSG_PATPMT           NGX_RTMP_MSG_MAX + 6
#define NGX_MPEGTS_MSG_M3U8             NGX_RTMP_MSG_MAX + 7
#define NGX_MPEGTS_MSG_CLOSE            NGX_RTMP_MSG_MAX + 8
#define NGX_RTMP_MAX_EVENT              NGX_RTMP_MSG_MAX + 9


/* RMTP control message types */
#define NGX_RTMP_USER_STREAM_BEGIN      0
#define NGX_RTMP_USER_STREAM_EOF        1
#define NGX_RTMP_USER_STREAM_DRY        2
#define NGX_RTMP_USER_SET_BUFLEN        3
#define NGX_RTMP_USER_RECORDED          4
#define NGX_RTMP_USER_PING_REQUEST      6
#define NGX_RTMP_USER_PING_RESPONSE     7
#define NGX_RTMP_USER_UNKNOWN           8
#define NGX_RTMP_USER_BUFFER_END        31


/* Chunk header:
 *   max 3  basic header
 * + max 11 message header
 * + max 4  extended header (timestamp) */
#define NGX_RTMP_MAX_CHUNK_HEADER       18

#define IS_IRAP(_nal_type_) (_nal_type_ >= 16 && _nal_type_ <= 23)

/* Audio codecs */
enum {
    /* Uncompressed codec id is actually 0,
     * but we use another value for consistency */
    NGX_RTMP_AUDIO_UNCOMPRESSED     = 16,
    NGX_RTMP_AUDIO_ADPCM            = 1,
    NGX_RTMP_AUDIO_MP3              = 2,
    NGX_RTMP_AUDIO_LINEAR_LE        = 3,
    NGX_RTMP_AUDIO_NELLY16          = 4,
    NGX_RTMP_AUDIO_NELLY8           = 5,
    NGX_RTMP_AUDIO_NELLY            = 6,
    NGX_RTMP_AUDIO_G711A            = 7,
    NGX_RTMP_AUDIO_G711U            = 8,
    NGX_RTMP_AUDIO_AAC              = 10,
    NGX_RTMP_AUDIO_SPEEX            = 11,
    NGX_RTMP_AUDIO_MP3_8            = 14,
    NGX_RTMP_AUDIO_DEVSPEC          = 15,
};


/* Video codecs */
enum {
    NGX_RTMP_VIDEO_JPEG             = 1,
    NGX_RTMP_VIDEO_SORENSON_H263    = 2,
    NGX_RTMP_VIDEO_SCREEN           = 3,
    NGX_RTMP_VIDEO_ON2_VP6          = 4,
    NGX_RTMP_VIDEO_ON2_VP6_ALPHA    = 5,
    NGX_RTMP_VIDEO_SCREEN2          = 6,
    NGX_RTMP_VIDEO_H264             = 7,
    NGX_RTMP_VIDEO_H265             = 12
};

extern ngx_int_t NGX_RTMP_HEVC_CODEC_ID;

typedef struct {
    uint32_t                csid;       /* chunk stream id */
    uint32_t                timestamp;  /* timestamp (delta) */
    uint32_t                mlen;       /* message length */
    uint8_t                 type;       /* message type id */
    uint32_t                msid;       /* message stream id */
} ngx_rtmp_header_t;


typedef struct {
    ngx_rtmp_header_t       hdr;
    uint32_t                dtime;
    uint32_t                len;        /* current fragment length */
    uint8_t                 ext;
    ngx_chain_t            *in;
} ngx_rtmp_stream_t;


typedef struct ngx_rtmp_frame_s     ngx_rtmp_frame_t;

struct ngx_rtmp_frame_s {
    ngx_rtmp_header_t       hdr;
    ngx_flag_t              av_header;
    ngx_flag_t              keyframe;
    ngx_flag_t              mandatory;
    ngx_uint_t              ref;

    ngx_rtmp_frame_t       *next;
    ngx_chain_t            *chain;
};

typedef struct ngx_mpegts_frame_s   ngx_mpegts_frame_t;

struct ngx_mpegts_frame_s {
    uint64_t                    pts;
    uint64_t                    dts;
    ngx_uint_t                  pid;
    ngx_uint_t                  sid;
    ngx_uint_t                  cc;
    unsigned                    key:1;
    ngx_uint_t                  ref;

    ngx_uint_t                  type;
    ngx_uint_t                  length;
    size_t                      pos;

    ngx_mpegts_frame_t         *next;
    ngx_chain_t                *chain;
};

/* disable zero-sized array warning by msvc */

#if (NGX_WIN32)
#pragma warning(push)
#pragma warning(disable:4200)
#endif

#define NGX_RTMP_LIVE       0
#define NGX_HTTP_FLV_LIVE   1
#define NGX_HLS_LIVE        2
#define NGX_MPEGTS_LIVE     3

typedef struct ngx_rtmp_session_s  ngx_rtmp_session_t;

#define NGX_RTMP_MAX_MERGE_FRAME    64

typedef ngx_chain_t * (* ngx_rtmp_prepared_pt)(ngx_rtmp_session_t *s);

typedef struct ngx_live_stream_s    ngx_live_stream_t;
typedef struct ngx_live_server_s    ngx_live_server_t;
typedef struct ngx_rtmp_addr_conf_s ngx_rtmp_addr_conf_t;

#define NGX_LIVE_INIT               0
#define NGX_LIVE_HANDSHAKE_DONE     1
#define NGX_LIVE_CONNECT            2
#define NGX_LIVE_CREATE_STREAM      3
#define NGX_LIVE_PUBLISH            4
#define NGX_LIVE_PLAY               5
#define NGX_LIVE_AV                 6
#define NGX_LIVE_CLOSE              7

#define NGX_LIVE_INTERNAL_ERR       0
#define NGX_LIVE_NORMAL_CLOSE       1
#define NGX_LIVE_RTMP_SEND_ERR      2
#define NGX_LIVE_RTMP_SEND_TIMEOUT  3
#define NGX_LIVE_FLV_SEND_ERR       4
#define NGX_LIVE_FLV_SEND_TIMEOUT   5
#define NGX_LIVE_RTMP_RECV_ERR      6
#define NGX_LIVE_FLV_RECV_ERR       7
#define NGX_LIVE_RELAY_TRANSIT      8
#define NGX_LIVE_RELAY_TIMEOUT      9
#define NGX_LIVE_CONTROL_DROP       10
#define NGX_LIVE_DROP_IDLE          11
#define NGX_LIVE_OCLP_NOTIFY_ERR    12
#define NGX_LIVE_OCLP_RELAY_ERR     13
#define NGX_LIVE_OCLP_PARA_ERR      14
#define NGX_LIVE_RELAY_CLOSE        15
#define NGX_LIVE_PROCESS_EXIT       16

struct ngx_rtmp_session_s {
    ngx_atomic_uint_t       number;
    struct sockaddr        *sockaddr;
    ngx_msec_t              roll_back;

    unsigned int            destroyed:1;

    ngx_flag_t              pause;

    ngx_int_t               acodec;
    ngx_int_t               vcodec;
    uint32_t                signature;  /* "RTMP" */ /* <-- FIXME wtf */

    ngx_event_t             close;

    ngx_pool_t             *pool;
    ngx_log_t              *log;

    ngx_rtmp_addr_conf_t   *addr_conf;

    void                  **ctx;
    void                  **main_conf;
    void                  **srv_conf;
    void                  **app_conf;

    ngx_live_server_t      *live_server;
    ngx_live_stream_t      *live_stream;

    ngx_str_t               remote_addr_text;
    ngx_str_t              *addr_text;
    int                     connected;

    ngx_http_variable_value_t *variables;

#if (NGX_PCRE)
    ngx_uint_t              ncaptures;
    int                    *captures;
    u_char                 *captures_data;
#endif

#if (nginx_version >= 1007005)
    ngx_queue_t             posted_dry_events;
#else
    ngx_event_t            *posted_dry_events;
#endif

    /* client buffer time in msec */
    uint32_t                buflen;
    uint32_t                ack_size;

    ngx_str_t               groupid;

    /* app/name */
    ngx_str_t               stream;

    /* stream name in publish or play*/
    ngx_str_t               name;
    ngx_str_t               pargs;  /* play or publish args */

    /* connection parameters */
    ngx_str_t               app;
    ngx_str_t               args;
    ngx_str_t               flashver;
    ngx_str_t               swf_url;
    ngx_str_t               tc_url;
    uint32_t                acodecs;
    uint32_t                vcodecs;
    ngx_str_t               page_url;

    /* middleware */
    ngx_str_t               scheme;
    ngx_str_t               domain;
    ngx_str_t               serverid;

    /* handshake data */
    ngx_buf_t              *hs_buf;
    u_char                 *hs_digest;
    unsigned                hs_old:1;
    ngx_uint_t              hs_stage;

    /* connection timestamps */
    ngx_msec_t              epoch;
    ngx_msec_t              peer_epoch;
    ngx_msec_t              base_time;
    uint32_t                current_time;

    /* ping */
    ngx_event_t             ping_evt;
    unsigned                ping_active:1;
    unsigned                ping_reset:1;

    /* auto-pushed? */
    unsigned                interprocess:1;
    unsigned                static_pull:1;
    unsigned                relay:1;
    unsigned                played:1;
    unsigned                published:1;
    unsigned                closed:1;
    unsigned                publishing:1;
    unsigned                finalized:1;

    /* live type: 0- RTMP 1- http-flv 2- hls */
    unsigned                live_type:2;
    ngx_uint_t              status;
    ngx_http_request_t     *request;

    unsigned                flv_state;

    ngx_uint_t              flv_version;
    ngx_uint_t              flv_flags;
    unsigned                flv_data_offset;
    unsigned                flv_tagsize;
    unsigned                flv_first_pts;

    /* input stream 0 (reserved by RTMP spec)
     * is used as free chain link */

    ngx_rtmp_stream_t      *in_streams;
    uint32_t                in_csid;
    ngx_uint_t              in_chunk_size;
    ngx_pool_t             *in_pool;
    uint32_t                in_bytes;
    uint32_t                in_last_ack;

    ngx_pool_t             *in_old_pool;
    ngx_int_t               in_chunk_size_changing;

    ngx_connection_t       *connection;

    /* merge frame and send */
    ngx_mpegts_frame_t     *prepare_mpegts_frame[NGX_RTMP_MAX_MERGE_FRAME];
    ngx_rtmp_frame_t       *prepare_frame[NGX_RTMP_MAX_MERGE_FRAME];
    ngx_chain_t            *merge[NGX_RTMP_MAX_MERGE_FRAME];
    ngx_uint_t              nframe;
    ngx_rtmp_prepared_pt    prepare_handler;

    /* for trace and statistics */
    ngx_int_t               notify_status;
    ngx_uint_t              finalize_reason;
    ngx_uint_t              stage;
    ngx_msec_t              init_time;
    ngx_msec_t              handshake_done_time;
    ngx_msec_t              connect_time;
    ngx_msec_t              create_stream_time;
    ngx_msec_t              ptime;                  /* publish or play time */
    ngx_msec_t              first_data;             /* audio video or metadata*/
    ngx_msec_t              first_metadata;
    ngx_msec_t              first_audio;
    ngx_msec_t              first_video;
    ngx_msec_t              close_stream_time;

    /* circular buffer of RTMP message pointers */
    ngx_msec_t              timeout;
    uint32_t                out_bytes;
    size_t                  out_pos, out_last;
    ngx_chain_t            *out_chain;
    unsigned                out_buffer:1;
    size_t                  out_queue;
    size_t                  out_cork;
    ngx_mpegts_frame_t    **mpegts_out;
    ngx_rtmp_frame_t       *out[0];
};

/* live stream manage */
#define NGX_LIVE_SERVERID_LEN   512
#define NGX_LIVE_STREAM_LEN     512

typedef struct ngx_rtmp_core_ctx_s      ngx_rtmp_core_ctx_t;
typedef struct ngx_rtmp_live_ctx_s      ngx_rtmp_live_ctx_t;
typedef struct ngx_mpegts_live_ctx_s    ngx_mpegts_live_ctx_t;
typedef struct ngx_hls_live_ctx_s       ngx_hls_live_ctx_t;

struct ngx_rtmp_core_ctx_s {
    ngx_rtmp_core_ctx_t    *next;
    ngx_rtmp_session_t     *session;

    unsigned                publishing:1;
};


#define NGX_RTMP_MAX_OCLP   8
#define NGX_RTMP_MAX_PUSH   8


struct ngx_live_stream_s {
    u_char                      name[NGX_LIVE_STREAM_LEN];

    ngx_int_t                   pslot;

    ngx_rtmp_core_ctx_t        *publish_ctx;
    ngx_rtmp_core_ctx_t        *play_ctx;

    /* notify */
    ngx_netcall_ctx_t          *stream_nctx;

    ngx_live_stream_t          *next;

    /* for live */
    ngx_map_t                   pubctx;
    ngx_rtmp_live_ctx_t        *ctx;
    ngx_mpegts_live_ctx_t      *mpegts_ctx;
    ngx_hls_live_ctx_t         *hls_ctx;
    ngx_rtmp_bandwidth_t        bw_in;
    ngx_rtmp_bandwidth_t        bw_in_audio;
    ngx_rtmp_bandwidth_t        bw_in_video;
    ngx_rtmp_bandwidth_t        bw_out;
    ngx_msec_t                  epoch;
    unsigned                    active:1;
    unsigned                    publishing:1;
    unsigned                    notify_meta:1;
};

struct ngx_live_server_s {
    u_char                      serverid[NGX_LIVE_SERVERID_LEN];
    ngx_uint_t                  n_stream;
    ngx_flag_t                  deleted;

    ngx_live_server_t          *next;

    ngx_live_stream_t         **streams;
};


ngx_live_server_t *ngx_live_create_server(ngx_str_t *serverid);
ngx_live_server_t *ngx_live_fetch_server(ngx_str_t *serverid);
void ngx_live_delete_server(ngx_str_t *serverid);

ngx_live_stream_t *ngx_live_create_stream(ngx_str_t *serverid,
        ngx_str_t *stream);
ngx_live_stream_t *ngx_live_fetch_stream(ngx_str_t *serverid,
        ngx_str_t *stream);
void ngx_live_delete_stream(ngx_str_t *serverid, ngx_str_t *stream);

void ngx_live_create_ctx(ngx_rtmp_session_t *s, unsigned publishing);
void ngx_live_delete_ctx(ngx_rtmp_session_t *s);

void ngx_live_print();

#if (NGX_WIN32)
#pragma warning(pop)
#endif


/* handler result code:
 *  NGX_ERROR - error
 *  NGX_OK    - success, may continue
 *  NGX_DONE  - success, input parsed, reply sent; need no
 *      more calls on this event */
typedef ngx_int_t (*ngx_rtmp_handler_pt)(ngx_rtmp_session_t *s,
        ngx_rtmp_header_t *h, ngx_chain_t *in);


typedef struct {
    ngx_str_t               name;
    ngx_rtmp_handler_pt     handler;
} ngx_rtmp_amf_handler_t;


typedef struct {
    ngx_flag_t              fast_reload;

    ngx_array_t             servers;    /* ngx_rtmp_core_srv_conf_t */

    ngx_array_t             events[NGX_RTMP_MAX_EVENT];

    ngx_hash_t              amf_hash;
    ngx_array_t             amf_arrays;
    ngx_array_t             amf;

    ngx_uint_t              server_names_hash_max_size;
    ngx_uint_t              server_names_hash_bucket_size;

    ngx_hash_t              variables_hash;

    ngx_array_t             variables;         /* ngx_http_variable_t */
    ngx_array_t             prefix_variables;  /* ngx_http_variable_t */
    ngx_uint_t              ncaptures;

    ngx_uint_t              variables_hash_max_size;
    ngx_uint_t              variables_hash_bucket_size;

    ngx_hash_keys_arrays_t *variables_keys;

    ngx_array_t            *ports;  /* ngx_rtmp_conf_port_t */
} ngx_rtmp_core_main_conf_t;


/* global main conf for stats */
extern ngx_rtmp_core_main_conf_t   *ngx_rtmp_core_main_conf;


typedef struct {
    ngx_array_t             applications; /* ngx_rtmp_core_app_conf_t */
    ngx_str_t               name;
    ngx_uint_t              merge_frame;
    ngx_flag_t              tcp_nodelay;
    void                  **app_conf;
    ngx_uint_t              hevc_codec;
} ngx_rtmp_core_app_conf_t;


typedef struct ngx_rtmp_core_srv_conf_s {
    ngx_array_t             applications; /* ngx_rtmp_core_app_conf_t */

    ngx_rtmp_core_app_conf_t *default_app;

    ngx_msec_t              timeout;
    ngx_msec_t              ping;
    ngx_msec_t              ping_timeout;
    ngx_flag_t              so_keepalive;
    ngx_int_t               max_streams;

    ngx_uint_t              ack_window;

    ngx_int_t               chunk_size;
    ngx_pool_t             *pool;
    ngx_chain_t            *free;
    ngx_chain_t            *free_hs;
    size_t                  max_message;
    ngx_flag_t              play_time_fix;
    ngx_flag_t              publish_time_fix;
    ngx_flag_t              busy;
    size_t                  out_queue;
    size_t                  out_cork;
    ngx_msec_t              buflen;

    ngx_rtmp_conf_ctx_t    *ctx;

    unsigned                listen:1;
#if (NGX_PCRE)
    unsigned                captures:1;
#endif

    ngx_str_t               server_name;
    ngx_str_t               serverid;

    /* array of the ngx_rtmp_server_name_t, "server_name" directive */
    ngx_array_t             server_names;
} ngx_rtmp_core_srv_conf_t;


typedef struct {
#if (NGX_PCRE)
    ngx_rtmp_regex_t       *regex;
#endif
    ngx_rtmp_core_srv_conf_t *server; /* virtual name server conf */
    ngx_str_t               name;
} ngx_rtmp_server_name_t;


typedef struct {
    ngx_hash_combined_t     names;

    ngx_uint_t              nregex;
    ngx_rtmp_server_name_t *regex;
} ngx_rtmp_virtual_names_t;


struct ngx_rtmp_addr_conf_s {
    ngx_rtmp_core_srv_conf_t *default_server;
    ngx_rtmp_virtual_names_t *virtual_names;

    ngx_str_t               addr_text;
    unsigned                proxy_protocol:1;
};

typedef struct {
    in_addr_t               addr;
    ngx_rtmp_addr_conf_t    conf;
} ngx_rtmp_in_addr_t;


#if (NGX_HAVE_INET6)

typedef struct {
    struct in6_addr         addr6;
    ngx_rtmp_addr_conf_t    conf;
} ngx_rtmp_in6_addr_t;

#endif


typedef struct {
    struct sockaddr        *sockaddr;
    socklen_t               socklen;

    unsigned                set:1;
    unsigned                default_server:1;
    unsigned                bind:1;
    unsigned                wildcard:1;
#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
    unsigned                ipv6only:2;
#endif
    unsigned                reuseport:1;
    unsigned                so_keepalive:2;
    unsigned                proxy_protocol:1;
#if (NGX_HAVE_KEEPALIVE_TUNABLE)
    int                     tcp_keepidle;
    int                     tcp_keepintvl;
    int                     tcp_keepcnt;
#endif

    u_char                  addr[NGX_SOCKADDR_STRLEN + 1];
} ngx_rtmp_listen_opt_t;


typedef struct {
    ngx_rtmp_listen_opt_t   opt;

    ngx_hash_t              hash;
    ngx_hash_wildcard_t    *wc_head;
    ngx_hash_wildcard_t    *wc_tail;

#if (NGX_PCRE)
    ngx_uint_t              nregex;
    ngx_rtmp_server_name_t *regex;
#endif

    /* the default server configuration for this address:port */
    ngx_rtmp_core_srv_conf_t   *default_server;
    ngx_array_t             servers;    /* array of ngx_http_core_srv_conf_t */
} ngx_rtmp_conf_addr_t;


/* nginx dynamic conf */
typedef struct {
    ngx_str_t               serverid;
} ngx_rtmp_core_srv_dconf_t;


typedef struct {
    ngx_int_t             (*preconfiguration)(ngx_conf_t *cf);
    ngx_int_t             (*postconfiguration)(ngx_conf_t *cf);

    void                 *(*create_main_conf)(ngx_conf_t *cf);
    char                 *(*init_main_conf)(ngx_conf_t *cf, void *conf);

    void                 *(*create_srv_conf)(ngx_conf_t *cf);
    char                 *(*merge_srv_conf)(ngx_conf_t *cf, void *prev,
                                    void *conf);

    void                 *(*create_app_conf)(ngx_conf_t *cf);
    char                 *(*merge_app_conf)(ngx_conf_t *cf, void *prev,
                                    void *conf);
} ngx_rtmp_module_t;

#define NGX_RTMP_MODULE                 0x504D5452     /* "RTMP" */

#define NGX_RTMP_MAIN_CONF              0x02000000
#define NGX_RTMP_SRV_CONF               0x04000000
#define NGX_RTMP_APP_CONF               0x08000000
#define NGX_RTMP_REC_CONF               0x10000000


#define NGX_RTMP_MAIN_CONF_OFFSET  offsetof(ngx_rtmp_conf_ctx_t, main_conf)
#define NGX_RTMP_SRV_CONF_OFFSET   offsetof(ngx_rtmp_conf_ctx_t, srv_conf)
#define NGX_RTMP_APP_CONF_OFFSET   offsetof(ngx_rtmp_conf_ctx_t, app_conf)


#define ngx_rtmp_get_module_ctx(s, module)     (s)->ctx[module.ctx_index]
#define ngx_rtmp_set_ctx(s, c, module)         s->ctx[module.ctx_index] = c;
#define ngx_rtmp_delete_ctx(s, module)         s->ctx[module.ctx_index] = NULL;


#define ngx_rtmp_get_module_main_conf(s, module)                             \
    (s)->main_conf[module.ctx_index]
#define ngx_rtmp_get_module_srv_conf(s, module)  (s)->srv_conf[module.ctx_index]
#define ngx_rtmp_get_module_app_conf(s, module)  ((s)->app_conf ? \
    (s)->app_conf[module.ctx_index] : NULL)

#define ngx_rtmp_conf_get_module_main_conf(cf, module)                       \
    ((ngx_rtmp_conf_ctx_t *) cf->ctx)->main_conf[module.ctx_index]
#define ngx_rtmp_conf_get_module_srv_conf(cf, module)                        \
    ((ngx_rtmp_conf_ctx_t *) cf->ctx)->srv_conf[module.ctx_index]
#define ngx_rtmp_conf_get_module_app_conf(cf, module)                        \
    ((ngx_rtmp_conf_ctx_t *) cf->ctx)->app_conf[module.ctx_index]

#define ngx_rtmp_cycle_get_module_main_conf(cycle, module)                   \
    (cycle->conf_ctx[ngx_rtmp_module.index] ?                                \
        ((ngx_rtmp_conf_ctx_t *) cycle->conf_ctx[ngx_rtmp_module.index])     \
            ->main_conf[module.ctx_index]:                                   \
        NULL)

/* for virtual server */
#if (NGX_PCRE)
ngx_rtmp_regex_t *ngx_rtmp_regex_compile(ngx_conf_t *cf,
    ngx_regex_compile_t *rc);
ngx_int_t ngx_rtmp_regex_exec(ngx_rtmp_session_t *s, ngx_rtmp_regex_t *re,
    ngx_str_t *str);
#endif
ngx_int_t ngx_rtmp_add_listen(ngx_conf_t *cf, ngx_rtmp_core_srv_conf_t *cscf,
    ngx_rtmp_listen_opt_t *lsopt);
ngx_int_t ngx_rtmp_set_virtual_server(ngx_rtmp_session_t *s, ngx_str_t *host);


#ifdef NGX_DEBUG
char* ngx_rtmp_message_type(uint8_t type);
char* ngx_rtmp_user_message_type(uint16_t evt);
#endif


typedef struct {
    ngx_array_t                 urls; // ngx_live_relay_url_t

    ngx_str_t                   domain;
    ngx_str_t                   app;
    ngx_str_t                   name;
    ngx_str_t                   pargs;
    ngx_str_t                   referer;    // rtmp page_url
    ngx_str_t                   user_agent; // rtmp flashver

    ngx_str_t                   stream;

    void                       *tag; // module create relay
} ngx_live_relay_t;


void ngx_rtmp_init_connection(ngx_connection_t *c);
ngx_rtmp_session_t *ngx_rtmp_create_session(ngx_rtmp_addr_conf_t *addr_conf);
ngx_rtmp_session_t *ngx_rtmp_create_relay_session(ngx_rtmp_session_t *s,
    void *tag);
ngx_rtmp_session_t *ngx_rtmp_create_static_session(ngx_live_relay_t *relay,
    ngx_rtmp_addr_conf_t *addr_conf, void *tag);

void ngx_rtmp_set_combined_log(ngx_rtmp_session_t *s, void *d,
    ngx_log_handler_pt h);
void ngx_rtmp_init_session(ngx_rtmp_session_t *s, ngx_connection_t *c);
void ngx_rtmp_finalize_session(ngx_rtmp_session_t *s);
void ngx_rtmp_finalize_fake_session(ngx_rtmp_session_t *s);
void ngx_rtmp_handshake(ngx_rtmp_session_t *s);
void ngx_rtmp_client_handshake(ngx_rtmp_session_t *s, unsigned async);
void ngx_rtmp_free_handshake_buffers(ngx_rtmp_session_t *s);
void ngx_rtmp_cycle(ngx_rtmp_session_t *s);
void ngx_rtmp_reset_ping(ngx_rtmp_session_t *s);
ngx_int_t ngx_rtmp_fire_event(ngx_rtmp_session_t *s, ngx_uint_t evt,
        ngx_rtmp_header_t *h, ngx_chain_t *in);

ngx_int_t ngx_rtmp_set_chunk_size(ngx_rtmp_session_t *s, ngx_uint_t size);


/* Bit reverse: we need big-endians in many places  */
void * ngx_rtmp_rmemcpy(void *dst, const void* src, size_t n);

#define ngx_rtmp_rcpymem(dst, src, n) \
    (((u_char*)ngx_rtmp_rmemcpy(dst, src, n)) + (n))


static ngx_inline uint16_t
ngx_rtmp_r16(uint16_t n)
{
    return (n << 8) | (n >> 8);
}


static ngx_inline uint32_t
ngx_rtmp_r32(uint32_t n)
{
    return (n << 24) | ((n << 8) & 0xff0000) | ((n >> 8) & 0xff00) | (n >> 24);
}


static ngx_inline uint64_t
ngx_rtmp_r64(uint64_t n)
{
    return (uint64_t) ngx_rtmp_r32((uint32_t) n) << 32 |
                      ngx_rtmp_r32((uint32_t) (n >> 32));
}


ngx_int_t
ngx_rtmp_find_virtual_server(ngx_rtmp_virtual_names_t *virtual_names,
    ngx_str_t *host, ngx_rtmp_core_srv_conf_t **cscfp);

/* Receiving messages */
ngx_int_t ngx_rtmp_receive_message(ngx_rtmp_session_t *s,
        ngx_rtmp_header_t *h, ngx_chain_t *in);
ngx_int_t ngx_rtmp_protocol_message_handler(ngx_rtmp_session_t *s,
        ngx_rtmp_header_t *h, ngx_chain_t *in);
ngx_int_t ngx_rtmp_user_message_handler(ngx_rtmp_session_t *s,
        ngx_rtmp_header_t *h, ngx_chain_t *in);
ngx_int_t ngx_rtmp_aggregate_message_handler(ngx_rtmp_session_t *s,
        ngx_rtmp_header_t *h, ngx_chain_t *in);
ngx_int_t ngx_rtmp_amf_message_handler(ngx_rtmp_session_t *s,
        ngx_rtmp_header_t *h, ngx_chain_t *in);
ngx_int_t ngx_rtmp_amf_shared_object_handler(ngx_rtmp_session_t *s,
        ngx_rtmp_header_t *h, ngx_chain_t *in);


/* Shared output buffers */

ngx_int_t ngx_rtmp_prepare_merge_frame(ngx_rtmp_session_t *s);
void ngx_rtmp_free_merge_frame(ngx_rtmp_session_t *s);

void ngx_rtmp_shared_append_chain(ngx_rtmp_frame_t *frame, size_t size,
        ngx_chain_t *cl, ngx_flag_t mandatory);
ngx_rtmp_frame_t *ngx_rtmp_shared_alloc_frame(size_t size, ngx_chain_t *cl,
        ngx_flag_t mandatory);
void ngx_rtmp_shared_free_frame(ngx_rtmp_frame_t *frame);

#define ngx_rtmp_shared_acquire_frame(frame) ++frame->ref;

ngx_chain_t *ngx_rtmp_shared_state(ngx_http_request_t *r);

void ngx_mpegts_shared_append_chain(ngx_mpegts_frame_t *frame, ngx_chain_t *cl,
        ngx_flag_t mandatory);
ngx_mpegts_frame_t *ngx_rtmp_shared_alloc_mpegts_frame(ngx_chain_t *cl,
        ngx_flag_t mandatory);
void ngx_rtmp_shared_free_mpegts_frame(ngx_mpegts_frame_t *frame);

#define ngx_rtmp_shared_acquire_mpegts_frame(frame) ++frame->ref;

/* Sending messages */
ngx_int_t ngx_rtmp_send_message(ngx_rtmp_session_t *s, ngx_rtmp_frame_t *out,
        ngx_uint_t priority);

/* GOP */
ngx_int_t ngx_rtmp_gop_cache(ngx_rtmp_session_t *s, ngx_rtmp_frame_t *frame);
ngx_int_t ngx_rtmp_gop_send(ngx_rtmp_session_t *s, ngx_rtmp_session_t *ss);

/* RTMP Relation server */
ngx_rtmp_addr_conf_t *ngx_rtmp_find_related_addr_conf(ngx_cycle_t *cycle,
        ngx_str_t *addr);

/* core */
ngx_int_t ngx_rtmp_arg(ngx_rtmp_session_t *s, u_char *name, size_t len,
        ngx_str_t *value);

/* Note on priorities:
 * the bigger value the lower the priority.
 * priority=0 is the highest */


#define NGX_RTMP_LIMIT_SOFT         0
#define NGX_RTMP_LIMIT_HARD         1
#define NGX_RTMP_LIMIT_DYNAMIC      2

/* Protocol control messages */
ngx_rtmp_frame_t *ngx_rtmp_create_chunk_size(ngx_rtmp_session_t *s,
        uint32_t chunk_size);
ngx_rtmp_frame_t *ngx_rtmp_create_abort(ngx_rtmp_session_t *s,
        uint32_t csid);
ngx_rtmp_frame_t *ngx_rtmp_create_ack(ngx_rtmp_session_t *s,
        uint32_t seq);
ngx_rtmp_frame_t *ngx_rtmp_create_ack_size(ngx_rtmp_session_t *s,
        uint32_t ack_size);
ngx_rtmp_frame_t *ngx_rtmp_create_bandwidth(ngx_rtmp_session_t *s,
        uint32_t ack_size, uint8_t limit_type);

ngx_int_t ngx_rtmp_send_chunk_size(ngx_rtmp_session_t *s,
        uint32_t chunk_size);
ngx_int_t ngx_rtmp_send_abort(ngx_rtmp_session_t *s,
        uint32_t csid);
ngx_int_t ngx_rtmp_send_ack(ngx_rtmp_session_t *s,
        uint32_t seq);
ngx_int_t ngx_rtmp_send_ack_size(ngx_rtmp_session_t *s,
        uint32_t ack_size);
ngx_int_t ngx_rtmp_send_bandwidth(ngx_rtmp_session_t *s,
        uint32_t ack_size, uint8_t limit_type);

/* User control messages */
ngx_rtmp_frame_t *ngx_rtmp_create_stream_begin(ngx_rtmp_session_t *s,
        uint32_t msid);
ngx_rtmp_frame_t *ngx_rtmp_create_stream_eof(ngx_rtmp_session_t *s,
        uint32_t msid);
ngx_rtmp_frame_t *ngx_rtmp_create_stream_dry(ngx_rtmp_session_t *s,
        uint32_t msid);
ngx_rtmp_frame_t *ngx_rtmp_create_set_buflen(ngx_rtmp_session_t *s,
        uint32_t msid, uint32_t buflen_msec);
ngx_rtmp_frame_t *ngx_rtmp_create_recorded(ngx_rtmp_session_t *s,
        uint32_t msid);
ngx_rtmp_frame_t *ngx_rtmp_create_ping_request(ngx_rtmp_session_t *s,
        uint32_t timestamp);
ngx_rtmp_frame_t *ngx_rtmp_create_ping_response(ngx_rtmp_session_t *s,
        uint32_t timestamp);

ngx_int_t ngx_rtmp_send_stream_begin(ngx_rtmp_session_t *s,
        uint32_t msid);
ngx_int_t ngx_rtmp_send_stream_eof(ngx_rtmp_session_t *s,
        uint32_t msid);
ngx_int_t ngx_rtmp_send_stream_dry(ngx_rtmp_session_t *s,
        uint32_t msid);
ngx_int_t ngx_rtmp_send_set_buflen(ngx_rtmp_session_t *s,
        uint32_t msid, uint32_t buflen_msec);
ngx_int_t ngx_rtmp_send_recorded(ngx_rtmp_session_t *s,
        uint32_t msid);
ngx_int_t ngx_rtmp_send_ping_request(ngx_rtmp_session_t *s,
        uint32_t timestamp);
ngx_int_t ngx_rtmp_send_ping_response(ngx_rtmp_session_t *s,
        uint32_t timestamp);

/* AMF sender/receiver */
ngx_int_t ngx_rtmp_append_amf(ngx_rtmp_session_t *s,
        ngx_chain_t **first, ngx_chain_t **last,
        ngx_rtmp_amf_elt_t *elts, size_t nelts);
ngx_int_t ngx_rtmp_receive_amf(ngx_rtmp_session_t *s, ngx_chain_t *in,
        ngx_rtmp_amf_elt_t *elts, size_t nelts);

ngx_rtmp_frame_t *ngx_rtmp_create_amf(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
        ngx_rtmp_amf_elt_t *elts, size_t nelts);
ngx_int_t ngx_rtmp_send_amf(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
        ngx_rtmp_amf_elt_t *elts, size_t nelts);

/* AMF status sender */
ngx_rtmp_frame_t *ngx_rtmp_create_error(ngx_rtmp_session_t *s, char *code,
        char* level, char *desc);
ngx_rtmp_frame_t *ngx_rtmp_create_status(ngx_rtmp_session_t *s, char *code,
        char* level, char *desc);
ngx_rtmp_frame_t *ngx_rtmp_create_play_status(ngx_rtmp_session_t *s, char *code,
        char* level, ngx_uint_t duration, ngx_uint_t bytes);
ngx_rtmp_frame_t *ngx_rtmp_create_sample_access(ngx_rtmp_session_t *s);

ngx_int_t ngx_rtmp_send_error(ngx_rtmp_session_t *s, char *code,
        char* level, char *desc);
ngx_int_t ngx_rtmp_send_status(ngx_rtmp_session_t *s, char *code,
        char* level, char *desc);
ngx_int_t ngx_rtmp_send_play_status(ngx_rtmp_session_t *s, char *code,
        char* level, ngx_uint_t duration, ngx_uint_t bytes);
ngx_int_t ngx_rtmp_send_sample_access(ngx_rtmp_session_t *s);
void ngx_rtmp_mpegts_mux(ngx_rtmp_session_t *s);


/* Frame types */
#define NGX_RTMP_VIDEO_KEY_FRAME            1
#define NGX_RTMP_VIDEO_INTER_FRAME          2
#define NGX_RTMP_VIDEO_DISPOSABLE_FRAME     3


static ngx_inline ngx_int_t
ngx_rtmp_get_video_frame_type(ngx_chain_t *in)
{
    return (in->buf->pos[0] & 0xf0) >> 4;
}


static ngx_inline ngx_int_t
ngx_rtmp_is_codec_header(ngx_chain_t *in)
{
    return in->buf->pos + 1 < in->buf->last && in->buf->pos[1] == 0;
}


extern ngx_rtmp_bandwidth_t                 ngx_rtmp_bw_out;
extern ngx_rtmp_bandwidth_t                 ngx_rtmp_bw_in;


extern ngx_uint_t                           ngx_rtmp_naccepted;
#if (nginx_version >= 1007011)
extern ngx_queue_t                          ngx_rtmp_init_queue;
#elif (nginx_version >= 1007005)
extern ngx_thread_volatile ngx_queue_t      ngx_rtmp_init_queue;
#else
extern ngx_thread_volatile ngx_event_t     *ngx_rtmp_init_queue;
#endif

extern ngx_uint_t                           ngx_rtmp_max_module;
extern ngx_module_t                         ngx_rtmp_module;
extern ngx_module_t                         ngx_rtmp_core_module;

#endif /* _NGX_RTMP_H_INCLUDED_ */
