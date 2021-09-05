#!/bin/sh
# This is a template copy it by: ./README.sh | xclip -selection c
# to https://openwrt.org/docs/guide-user/services/webserver/nginx#configuration


NGINX_UTIL="/usr/bin/nginx-util"

EXAMPLE_COM="example.com"

MSG="
/* Created by the following bash script that includes the source of some files:
 * https://github.com/openwrt/packages/net/nginx-util/files/README.sh
 */"

eval $("${NGINX_UTIL}" get_env)

code() {
    local file
    [ $# -gt 1 ] && file="$2" || file="$(basename "$1")"
    printf "<file nginx %s>\n%s</file>" "$1" "$(cat "${file}")";
}

ifConfEcho() {
    sed -nE "s/^\s*$1=\s*(\S*)\s*\\\\$/\n$2 \"\1\";/p" ../../nginx/Makefile;
}

cat <<EOF





===== Configuration =====${MSG}



The official Documentation contains a
[[https://docs.nginx.com/nginx/admin-guide/|Admin Guide]].
Here we will look at some often used configuration parts and how we handle them
at OpenWrt.
At different places there are references to the official
[[https://docs.nginx.com/nginx/technical-specs/|Technical Specs]]
for further reading.

**tl;dr:** When starting Nginx by ''/etc/init.d/nginx'', it creates its main
configuration dynamically based on a minimal template and the
[[docs:guide-user:base-system:uci|🡒UCI]] configuration.

The UCI ''/etc/config/nginx'' contains initially:
| ''config server '${LAN_NAME}''' | \
Default server for the LAN, which includes all ''${CONF_DIR}*.locations''. |
| ''config server '_redirect2ssl''' | \
Redirects inexistent URLs to HTTPS. |

It enables also the ''${CONF_DIR}'' directory for further configuration:
| ''${CONF_DIR}\$NAME.conf'' | \
Is included in the main configuration. \
It is prioritized over a UCI ''config server '\$NAME' ''. |
| ''${CONF_DIR}\$NAME.locations'' | \
Is include in the ''${LAN_NAME}'' server and can be re-used for others, too. |
| ''$(dirname "${CONF_DIR}")/restrict_locally'' | \
Is include in the ''${LAN_NAME}'' server and allows only accesses from LAN. |

Setup configuration (for a server ''\$NAME''):
| ''$(basename ${NGINX_UTIL}) [${ADD_SSL_FCT}|del_ssl] \$NAME''  | \
Add/remove a self-signed certificate and corresponding directives. |
| ''uci set nginx.\$NAME.access_log='logd openwrt''' | \
Writes accesses to Openwrt’s \
[[docs:guide-user:base-system:log.essentials|🡒logd]]. |
| ''uci set nginx.\$NAME.error_log='logd' '' | \
Writes errors to Openwrt’s \
[[docs:guide-user:base-system:log.essentials|🡒logd]]. |
| ''uci [set|add_list] nginx.\$NAME.key='value' '' | \
Becomes a ''key value;'' directive if the //key// does not start with //uci_//. |
| ''uci set nginx.\$NAME=[disable|server]'' |\
Disable/enable inclusion in the dynamic conf.|
| ''uci set nginx.global.uci_enable=false'' | \
Use a custom ''${NGINX_CONF}'' rather than a dynamic conf. |



==== Basic ====${MSG}


We modify the configuration by changing servers saved in the UCI configuration
at ''/etc/config/nginx'' and/or by creating different configuration files in the
''${CONF_DIR}'' directory.
These files use the file extensions ''.locations'' and ''.conf'' plus ''.crt''
and ''.key'' for SSL certificates and keys.((
We can disable a single configuration file in ''${CONF_DIR}'' by giving it
another extension, e.g., by adding ''.disabled''.))
For the new configuration to take effect, we must reload it by:

<code bash>service nginx reload</code>

For OpenWrt we use a special initial configuration, which is explained in the
section [[#openwrt_s_defaults|🡓OpenWrt’s Defaults]].
So, we can make a site available at a specific URL in the **LAN** by creating a
''.locations'' file in the directory ''${CONF_DIR}''.
Such a file consists just of some
[[https://nginx.org/en/docs/http/ngx_http_core_module.html#location|
location blocks]].
Under the latter link, you can find also the official documentation for all
available directives of the HTTP core of Nginx.
Look for //location// in the Context list.

The following example provides a simple template, see at the end for
different [[#locations_for_apps|🡓Locations for Apps]]((look for
[[https://github.com/search?utf8=%E2%9C%93&q=repo%3Aopenwrt%2Fpackages
+extension%3Alocations&type=Code&ref=advsearch&l=&l=|
other packages using a .locations file]], too.)):

<code nginx ${CONF_DIR}example.locations>
location /ex/am/ple {
	access_log off; # default: not logging accesses.
	# access_log /proc/self/fd/1 openwrt; # use logd (init forwards stdout).
	# error_log stderr; # default: logging to logd (init forwards stderr).
	error_log /dev/null; # disable error logging after config file is read.
	# (state path of a file for access_log/error_log to the file instead.)
	index index.html;
}
# location /eg/static { … }
</code>

All location blocks in all ''.locations'' files must use different URLs,
since they are all included in the ''${LAN_NAME}'' server that is part of the
[[#openwrt_s_defaults|🡓OpenWrt’s Defaults]].((
We reserve the ''location /'' for making LuCI available under the root URL,
e.g. [[https://192.168.1.1/|192.168.1.1/]].
All other sites shouldn’t use the root ''location /'' without suffix.))
We should use the root URL for other sites than LuCI only on **other** domain
names, e.g., we could make a site available at https://${EXAMPLE_COM}/.
In order to do that, we create [[#new_server_parts|🡓New Server Parts]] for all
domain names.
We can also activate SSL thereby, see
[[#ssl_server_parts|🡓SSL Server Parts]].
We use such server parts also for publishing sites to the internet (WAN)
instead of making them available just locally (in the LAN).

Via ''${CONF_DIR}*.conf'' files we can add directives to the //http// part of
the configuration.
If you would change the configuration ''$(basename "${UCI_CONF}").template''
instead, it is not updated to new package's versions anymore.
Although it is not recommended, you can also disable the whole UCI config and
create your own ''${NGINX_CONF}''; then invoke:

<code bash>uci set nginx.global.uci_enable=false</code>



==== New Server Parts ====${MSG}


For making the router reachable from the WAN at a registered domain name,
it is not enough letting the
[[docs:guide-user:firewall:firewall_configuration|🡒firewall]] accept requests
(typically on ports 80 and 443) and giving the name server the internet IP
address of the router (maybe updated automatically by a
[[docs:guide-user:services:ddns:client|🡒DDNS Client]]).

We also need to set up virtual hosting for this domain name by creating an
appropriate server section in ''/etc/config/nginx''
(or in a ''${CONF_DIR}*.conf'' file, which cannot be changed using UCI).
All such parts are included in the main configuration of OpenWrt
([[#openwrt_s_defaults|🡓OpenWrt’s Defaults]]).

In the server part, we state the domain as
[[https://nginx.org/en/docs/http/ngx_http_core_module.html#server_name|
server_name]].
The link points to the same document as for the location blocks in the
[[#basic|🡑Basic Configuration]]: the official documentation for all available
directives of the HTTP core of Nginx.
This time look for //server// in the Context list, too.
The server part should also contain similar location blocks as
++before.|
We can re-include a ''.locations'' file that is included in the server part for
the LAN by default.
Then the site is reachable under the same path at both domains, e.g. by
https://192.168.1.1/ex/am/ple as well as by https://${EXAMPLE_COM}/ex/am/ple.
++

We can add directives to a server in the UCI configuration by invoking
''uci [set|add_list] nginx.${EXAMPLE_COM//./_}.key=value''.
If the //key// is not starting with //uci_//, it becomes a ''key value;''
++directive.|
Although the UCI config does not support nesting like Nginx, we can add a whole
block as //value//.
++

We cannot use dots in a //key// name other than in the //value//.
In the following example we replace the dot in //${EXAMPLE_COM}// by an
underscore for the UCI name of the server, but not for Nginx's //server_name//:

<code bash>
uci add nginx server &&
uci rename nginx.@server[-1]=${EXAMPLE_COM//./_} &&
uci add_list nginx.${EXAMPLE_COM//./_}.listen='80' &&
uci add_list nginx.${EXAMPLE_COM//./_}.listen='[::]:80' &&
uci set nginx.${EXAMPLE_COM//./_}.server_name='${EXAMPLE_COM}' &&
uci add_list nginx.${EXAMPLE_COM//./_}.include=\
'$(basename ${CONF_DIR})/${EXAMPLE_COM}.locations'
# uci add_list nginx.${EXAMPLE_COM//./_}.location='/ { … }' \
# root location for this server.
</code>

We can disable respective re-enable this server again by:

<code bash>
uci set nginx.${EXAMPLE_COM//./_}=disable # respective: \
uci set nginx.${EXAMPLE_COM//./_}=server
</code>

These changes are made in the RAM (and can be used until a reboot), we can save
them permanently by:

<code bash>uci commit nginx</code>

For creating a similar ''${CONF_DIR}${EXAMPLE_COM}.conf'', we can adopt the
following:

<code nginx ${CONF_DIR}${EXAMPLE_COM}.conf>
server {
	listen 80;
	listen [::]:80;
	server_name ${EXAMPLE_COM};
	include '$(basename ${CONF_DIR})/${EXAMPLE_COM}.locations';
	# location / { … } # root location for this server.
}
</code>

[[#openwrt_s_defaults|🡓OpenWrt’s Defaults]] include the UCI server
''config server '_redirect2ssl' ''.
It  acts as //default_server// for HTTP and redirects requests for inexistent
URLs to HTTPS.
For making another domain name accessible to all addresses, the corresponding
server part should listen on port //80// and contain the FQDN as
//server_name//, cf. the official documentation on
[[https://nginx.org/en/docs/http/request_processing.html|request_processing]].

Furthermore, there is a UCI server named ''${LAN_NAME}''.
It is the //default_server// for HTTPS and allows connections from LAN only.
It includes the file ''$(dirname "${CONF_DIR}")/restrict_locally'' with
appropriate //allow/deny// directives, cf. the official documentation on
[[https://nginx.org/en/docs/http/ngx_http_access_module.html|limiting access]].



==== SSL Server Parts ====${MSG}


For enabling HTTPS for a domain we need a SSL certificate as well as its key and
add them by the directives //ssl_certificate// respective
//ssl_certificate_key// to the server part of the domain
([[https://nginx.org/en/docs/http/configuring_https_servers.html#sni|TLS SNI]]
is supported by default).
The rest of the configuration is similar as for general
[[#new_server_parts|🡑New Server Parts]].
We only have to adjust the listen directives by adding the //ssl// parameter and
changing the port from //80// to //443//.

The official documentation of the SSL module contains an
[[https://nginx.org/en/docs/http/ngx_http_ssl_module.html#example|
example]] with some optimizations.
We can extend an existing UCI server section similarly, e.g., for the above
''config server '${EXAMPLE_COM//./_}' '' we invoke:

<code bash>
# Instead of 'del_list' the listen* entries, we could use '443 ssl' beforehand.
uci del_list nginx.${EXAMPLE_COM//./_}.listen='80' &&
uci del_list nginx.${EXAMPLE_COM//./_}.listen='[::]:80' &&
uci add_list nginx.${EXAMPLE_COM//./_}.listen='443 ssl' &&
uci add_list nginx.${EXAMPLE_COM//./_}.listen='[::]:443 ssl' &&
uci set nginx.${EXAMPLE_COM//./_}.ssl_certificate=\
'${CONF_DIR}${EXAMPLE_COM}.crt' &&
uci set nginx.${EXAMPLE_COM//./_}.ssl_certificate_key=\
'${CONF_DIR}${EXAMPLE_COM}.key' &&
uci set nginx.${EXAMPLE_COM//./_}.ssl_session_cache=\
'${SSL_SESSION_CACHE_ARG}' &&
uci set nginx.${EXAMPLE_COM//./_}.ssl_session_timeout=\
'${SSL_SESSION_TIMEOUT_ARG}' &&
uci commit nginx
</code>

For making the server in ''${CONF_DIR}${EXAMPLE_COM}.conf'' available
via SSL, we can make similar changes there.

The following command creates a **self-signed** SSL certificate and changes the
corresponding configuration:

<code bash>$(basename "${NGINX_UTIL}") ${ADD_SSL_FCT} ${EXAMPLE_COM}</code>

  - If a ''$(basename "${CONF_DIR}")/${EXAMPLE_COM}.conf'' file exists, it\
    adds //ssl_*// directives and changes the //listen// directives there.\
    Else it does that similarly to the example above for a ++selected UCI\
    server.| Hereby it searches the UCI config first for a server with the\
    given name and then for a server whose //server_name// contains the name.\
    For //${EXAMPLE_COM}// it is the latter as a UCI key cannot have dots.++
  - It checks if there is a certificate with key for '${EXAMPLE_COM}' that is\
    valid for at least 13 months or tries to create a self-signed one.
  - When cron is activated, it installs a cron job for renewing the self-signed\
    certificate every year if needed, too. We can activate cron by: \
    <code bash>service cron enable && service cron start</code>

This can be undone by invoking:

<code bash>$(basename "${NGINX_UTIL}") del_ssl ${EXAMPLE_COM}</code>

For using an SSL certificate and key that are managed otherwise, there is:

<code bash>$(basename "${NGINX_UTIL}") add_ssl ${EXAMPLE_COM} "\$MANAGER" \
"/absolute/path/to/crt" "/absolute/path/to/key"</code>

It only adds //ssl_*// directives and changes the //listen// directives in
the appropriate configuration, but does not create or change the certificate
or its key. This can be reverted by:

<code bash>$(basename "${NGINX_UTIL}") del_ssl ${EXAMPLE_COM} "\$MANAGER"</code>

For example [[https://github.com/ndilieto/uacme|uacme]] or
[[https://github.com/Neilpang/acme.sh|acme.sh]] can be used for creating an SSL
certificate signed by Let’s Encrypt and changing the config
++accordingly.|
They call ''$(basename "${NGINX_UTIL}") add_ssl \$FQDN acme \$CRT \$KEY''
internally.++
We can install them by:

<code bash>
opkg update && opkg install uacme #or: acme #and for LuCI: luci-app-acme
</code>

[[#openwrt_s_defaults|🡓OpenWrt’s Defaults]] include a UCI server for the LAN:
''config server '${LAN_NAME}' ''.
It has //ssl_*// directives prepared for a self-signed((Let’s Encrypt (and other
CAs) cannot sign certificates of a **local** server.))
SSL certificate, which is created on the first start of Nginx.
The server listens on all addresses, is the //default_server// for HTTPS and
allows connections from LAN only (by including the file ''restrict_locally''
with //allow/deny// directives, cf. the official documentation on
[[https://nginx.org/en/docs/http/ngx_http_access_module.html|limiting access]]).

For making another domain name accessible to all addresses, the corresponding
SSL server part should listen on port //443// and contain the FQDN as
//server_name//, cf. the official documentation on
[[https://nginx.org/en/docs/http/request_processing.html|request_processing]].

Furthermore, there is also a UCI server named ''_redirect2ssl'', which listens
on all addresses, acts as //default_server// for HTTP and redirects requests for
inexistent URLs to HTTPS.



==== OpenWrt’s Defaults ====${MSG}


Since Nginx is compiled with these presets, we can pretend that the main
configuration will always contain the following directives
(though we can overwrite them):

<code nginx>$(ifConfEcho --pid-path pid)\
$(ifConfEcho --lock-path lock_file)\
$(ifConfEcho --error-log-path error_log)\
$(false && ifConfEcho --http-log-path access_log)\
$(ifConfEcho --http-proxy-temp-path proxy_temp_path)\
$(ifConfEcho --http-client-body-temp-path client_body_temp_path)\
$(ifConfEcho --http-fastcgi-temp-path fastcgi_temp_path)\
</code>

When starting or reloading the Nginx service, the ''/etc/init.d/nginx'' script
sets also the following directives
(so we cannot change them in the used configuration file):

<code nginx>
daemon off; # procd expects services to run in the foreground
</code>

Then, the init sript creates the main configuration
''$(basename "${UCI_CONF}")'' dynamically from the template:

$(code "${UCI_CONF}.template")

So, the access log is turned off by default and we can look at the error log
by ''logread'', as init.d script forwards stderr and stdout to the
[[docs:guide-user:base-system:log.essentials|🡒runtime log]].
We can set the //error_log// and //access_log// to files, where the log
messages are forwarded to instead (after the configuration is read).
And for redirecting the access log of a //server// or //location// to the logd,
too, we insert the following directive in the corresponding block:

<code nginx>	access_log /proc/self/fd/1 openwrt;</code>

If we setup a server through UCI, we can use the options //error_log// and/or
//access_log// also with the special path
++'logd'.|
When initializing the Nginx service, this special path is replaced by //stderr//
respective ///proc/self/fd/1// (which are forwarded to the runtime log).
++

For creating the configuration from the template shown above, Nginx’s init
script replaces the comment ''#UCI_HTTP_CONFIG'' by all UCI servers.
For each server section in the the UCI configuration, it basically copies all
options into a Nginx //server { … }// part, in detail:
  * Options starting with ''uci_'' are skipped. Currently there is only\
  the ''option ${MANAGE_SSL}=…'' in ++usage.| It is set to\
  //'self-signed'// when invoking\
  ''$(basename ${NGINX_UTIL}) ${ADD_SSL_FCT} \$NAME''.\
  Then the corresponding certificate is re-newed if it is about to expire.\
  All those certificates are checked on the initialization of the Nginx service\
  and if Cron is available, it is deployed for checking them annually, too.++
  * All other lists or options of the form ''key='value' '' are written\
  one-to-one as ''key value;'' directives to the configuration file.\
  Just the path //logd// has a special meaning for the logging directives\
  (described in the previous paragraph).

The init.d script of Nginx uses the //$(basename ${NGINX_UTIL})// for creating
the configuration file
++in RAM.|
The main configuration ''${UCI_CONF}'' is a symbolic link to this place
(it is a dead link if the Nginx service is not running).
++

We could use a custom configuration created at ''${NGINX_CONF}'' instead of the
dynamic configuration, too.((
For using a custom configuration at ''${NGINX_CONF}'', we execute
<code bash>uci set nginx.global.uci_enable='false' </code>
Then the rest of the UCI config is ignored and //init.d// will not create the
main configuration dynamically from the template anymore.
Invoking ''$(basename ${NGINX_UTIL}) [${ADD_SSL_FCT}|del_ssl] \$FQDN''
will still try to change a server in ''$(basename "${CONF_DIR}")/\$FQDN.conf''
(this is less reliable than for a UCI config as it uses regular expressions, not
a complete parser for the Nginx configuration).))
This is not encouraged since you cannot setup servers using UCI anymore.
Rather, we can put custom configuration parts to ''.conf'' files in the
''${CONF_DIR}'' directory.
The main configuration pulls in all ''$(basename "${CONF_DIR}")/*.conf'' files
into the //http {…}// block behind the created UCI servers.

The initial UCI config is enabled and contains two server section:

$(code "/etc/config/nginx" "nginx.config")

While the LAN server is the //default_server// for HTTPS, the server
redirecting requests for an inexistent ''server_name'' from HTTP to HTTPS acts
as //default_server// if there is ++no other|;
it uses an invalid name for that, more in the official documentation on
[[https://nginx.org/en/docs/http/request_processing.html|request_processing]]
++.

The LAN server pulls in all ''.locations'' files from the directory
''${CONF_DIR}''.
We can install the location parts of different sites there (see
[[#basic|🡑Basic Configuration]]) and re-include them into other servers.
This is needed especially for making them available to the WAN
([[#new_server_parts|🡑New Server Parts]]).
The LAN server listens for all addresses on port //443// and restricts the
access to local addresses by including:
$(code "$(dirname "${CONF_DIR}")/restrict_locally")

When starting or reloading the Nginx service, the init.d looks which UCI servers
have set ''option ${MANAGE_SSL} 'self-signed' '', e.g. the LAN server.
For all those servers it checks if there is a certificate that is still valid
for 13 months or (re-)creates a self-signed one.
If there is any such server, it installs also a cron job that checks the
corresponding certificates once a year.
The option ''${MANAGE_SSL}'' is set to //'self-signed'// respectively removed
from a UCI server named ''${EXAMPLE_COM//./_}'' by the following
(see [[#ssl_server_parts|🡑SSL Server Parts]], too):

<code bash>
$(basename ${NGINX_UTIL}) ${ADD_SSL_FCT} ${EXAMPLE_COM//./_} \
# respectively: \
$(basename ${NGINX_UTIL}) del_ssl ${EXAMPLE_COM//./_}
</code>


EOF
