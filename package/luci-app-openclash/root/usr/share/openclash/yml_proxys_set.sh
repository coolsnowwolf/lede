#!/bin/sh
. /lib/functions.sh
. /usr/share/openclash/ruby.sh
. /usr/share/openclash/log.sh

set_lock() {
   exec 886>"/tmp/lock/openclash_proxies_set.lock" 2>/dev/null
   flock -x 886 2>/dev/null
}

del_lock() {
   flock -u 886 2>/dev/null
   rm -rf "/tmp/lock/openclash_proxies_set.lock"
}

SERVER_FILE="/tmp/yaml_servers.yaml"
PROXY_PROVIDER_FILE="/tmp/yaml_provider.yaml"
servers_if_update=$(uci -q get openclash.config.servers_if_update)
config_auto_update=$(uci -q get openclash.config.auto_update)
CONFIG_FILE=$(uci -q get openclash.config.config_path)
CONFIG_NAME=$(echo "$CONFIG_FILE" |awk -F '/' '{print $5}' 2>/dev/null)
UPDATE_CONFIG_FILE=$(uci -q get openclash.config.config_update_path)
UPDATE_CONFIG_NAME=$(echo "$UPDATE_CONFIG_FILE" |awk -F '/' '{print $5}' 2>/dev/null)
UCI_DEL_LIST="uci -q del_list openclash.config.new_servers_group"
UCI_ADD_LIST="uci -q add_list openclash.config.new_servers_group"
UCI_SET="uci -q set openclash.config."
MIX_PROXY=$(uci -q get openclash.config.mix_proxies)
servers_name="/tmp/servers_name.list"
proxy_provider_name="/tmp/provider_name.list"
set_lock

if [ ! -z "$UPDATE_CONFIG_FILE" ]; then
   CONFIG_FILE="$UPDATE_CONFIG_FILE"
   CONFIG_NAME="$UPDATE_CONFIG_NAME"
fi

if [ -z "$CONFIG_FILE" ]; then
	CONFIG_FILE="/etc/openclash/config/$(ls -lt /etc/openclash/config/ | grep -E '.yaml|.yml' | head -n 1 |awk '{print $9}')"
	CONFIG_NAME=$(echo "$CONFIG_FILE" |awk -F '/' '{print $5}' 2>/dev/null)
fi

if [ -z "$CONFIG_NAME" ]; then
   CONFIG_FILE="/etc/openclash/config/config.yaml"
   CONFIG_NAME="config.yaml"
fi

yml_other_rules_del()
{
	 local section="$1"
   local enabled config
   config_get_bool "enabled" "$section" "enabled" "1"
   config_get "config" "$section" "config" ""
   config_get "rule_name" "$section" "rule_name" ""
   
   if [ "$enabled" = "0" ] || [ "$config" != "$2" ] || [ "$rule_name" != "$3" ]; then
      return
   else
      uci -q set openclash."$section".enabled=0
   fi
}
#写入代理集到配置文件
yml_proxy_provider_set()
{
   local section="$1"
   config_get_bool "enabled" "$section" "enabled" "1"
   config_get "config" "$section" "config" ""
   config_get "type" "$section" "type" ""
   config_get "name" "$section" "name" ""
   config_get "path" "$section" "path" ""
   config_get "provider_filter" "$section" "provider_filter" ""
   config_get "provider_url" "$section" "provider_url" ""
   config_get "provider_interval" "$section" "provider_interval" ""
   config_get "health_check" "$section" "health_check" ""
   config_get "health_check_url" "$section" "health_check_url" ""
   config_get "health_check_interval" "$section" "health_check_interval" ""
   
   if [ "$enabled" = "0" ]; then
      return
   fi

   if [ -z "$type" ]; then
      return
   fi
   
   if [ -z "$name" ]; then
      return
   fi
   
   if [ "$path" != "./proxy_provider/$name.yaml" ] && [ "$type" = "http" ]; then
      path="./proxy_provider/$name.yaml"
   elif [ -z "$path" ]; then
      return
   fi
   
   if [ -z "$health_check" ]; then
      return
   fi
   
   if [ ! -z "$if_game_proxy" ] && [ "$if_game_proxy" != "$name" ] && [ "$if_game_proxy_type" = "proxy-provider" ]; then
      return
   fi
   
   if [ "$MIX_PROXY" != "1" ] && [ ! -z "$config" ] && [ "$config" != "$CONFIG_NAME" ] && [ "$config" != "all" ]; then
      return
   fi
   
   #避免重复代理集
   if [ "$config" = "$CONFIG_NAME" ] || [ "$config" = "all" ]; then
      if [ -n "$(grep -w "path: $path" "$PROXY_PROVIDER_FILE" 2>/dev/null)" ]; then
         return
      elif [ "$(grep -w "^$name$" "$proxy_provider_name" |wc -l 2>/dev/null)" -ge 2 ] && [ -z "$(grep -w "path: $path" "$PROXY_PROVIDER_FILE" 2>/dev/null)" ]; then
      	 sed -i "1,/^${name}$/{//d}" "$proxy_provider_name" 2>/dev/null
         return
      fi
   fi
   
   LOG_OUT "Start Writing【$CONFIG_NAME - $type - $name】Proxy-provider To Config File..."
   echo "$name" >> /tmp/Proxy_Provider
   
cat >> "$PROXY_PROVIDER_FILE" <<-EOF
  $name:
    type: $type
    path: "$path"
EOF
   if [ -n "$provider_filter" ]; then
cat >> "$PROXY_PROVIDER_FILE" <<-EOF
    filter: "$provider_filter"
EOF
   fi
   if [ -n "$provider_url" ]; then
cat >> "$PROXY_PROVIDER_FILE" <<-EOF
    url: "$provider_url"
    interval: $provider_interval
EOF
   fi
cat >> "$PROXY_PROVIDER_FILE" <<-EOF
    health-check:
      enable: $health_check
      url: "$health_check_url"
      interval: $health_check_interval
EOF

}

set_alpn()
{
   if [ -z "$1" ]; then
      return
   fi
cat >> "$SERVER_FILE" <<-EOF
      - '$1'
EOF
}

set_http_path()
{
   if [ -z "$1" ]; then
      return
   fi
cat >> "$SERVER_FILE" <<-EOF
        - '$1'
EOF
}

set_h2_host()
{
   if [ -z "$1" ]; then
      return
   fi
cat >> "$SERVER_FILE" <<-EOF
        - '$1'
EOF
}

set_ws_headers()
{
   if [ -z "$1" ]; then
      return
   fi
cat >> "$SERVER_FILE" <<-EOF
        $1
EOF
}

#写入服务器节点到配置文件
yml_servers_set()
{

   local section="$1"
   config_get_bool "enabled" "$section" "enabled" "1"
   config_get "config" "$section" "config" ""
   config_get "type" "$section" "type" ""
   config_get "name" "$section" "name" ""
   config_get "server" "$section" "server" ""
   config_get "port" "$section" "port" ""
   config_get "cipher" "$section" "cipher" ""
   config_get "cipher_ssr" "$section" "cipher_ssr" ""
   config_get "password" "$section" "password" ""
   config_get "securitys" "$section" "securitys" ""
   config_get "udp" "$section" "udp" ""
   config_get "obfs" "$section" "obfs" ""
   config_get "obfs_ssr" "$section" "obfs_ssr" ""
   config_get "obfs_param" "$section" "obfs_param" ""
   config_get "obfs_vmess" "$section" "obfs_vmess" ""
   config_get "obfs_trojan" "$section" "obfs_trojan" ""
   config_get "protocol" "$section" "protocol" ""
   config_get "protocol_param" "$section" "protocol_param" ""
   config_get "host" "$section" "host" ""
   config_get "mux" "$section" "mux" ""
   config_get "custom" "$section" "custom" ""
   config_get "tls" "$section" "tls" ""
   config_get "skip_cert_verify" "$section" "skip_cert_verify" ""
   config_get "path" "$section" "path" ""
   config_get "alterId" "$section" "alterId" ""
   config_get "uuid" "$section" "uuid" ""
   config_get "auth_name" "$section" "auth_name" ""
   config_get "auth_pass" "$section" "auth_pass" ""
   config_get "psk" "$section" "psk" ""
   config_get "obfs_snell" "$section" "obfs_snell" ""
   config_get "snell_version" "$section" "snell_version" ""
   config_get "sni" "$section" "sni" ""
   config_get "alpn" "$section" "alpn" ""
   config_get "http_path" "$section" "http_path" ""
   config_get "keep_alive" "$section" "keep_alive" ""
   config_get "servername" "$section" "servername" ""
   config_get "h2_path" "$section" "h2_path" ""
   config_get "h2_host" "$section" "h2_host" ""
   config_get "grpc_service_name" "$section" "grpc_service_name" ""
   config_get "ws_opts_path" "$section" "ws_opts_path" ""
   config_get "ws_opts_headers" "$section" "ws_opts_headers" ""
   config_get "max_early_data" "$section" "max_early_data" ""
   config_get "early_data_header_name" "$section" "early_data_header_name" ""
   config_get "trojan_ws_path" "$section" "trojan_ws_path" ""
   config_get "trojan_ws_headers" "$section" "trojan_ws_headers" ""
   config_get "interface_name" "$section" "interface_name" ""
   config_get "routing_mark" "$section" "routing_mark" ""
   config_get "obfs_vless" "$section" "obfs_vless" ""
   config_get "vless_flow" "$section" "vless_flow" ""
   config_get "http_headers" "$section" "http_headers" ""
   config_get "hysteria_protocol" "$section" "hysteria_protocol" ""
   config_get "hysteria_up" "$section" "hysteria_up" ""
   config_get "hysteria_down" "$section" "hysteria_down" ""
   config_get "hysteria_alpn" "$section" "hysteria_alpn" ""
   config_get "hysteria_obfs" "$section" "hysteria_obfs" ""
   config_get "hysteria_auth" "$section" "hysteria_auth" ""
   config_get "hysteria_auth_str" "$section" "hysteria_auth_str" ""
   config_get "hysteria_ca" "$section" "hysteria_ca" ""
   config_get "hysteria_ca_str" "$section" "hysteria_ca_str" ""
   config_get "recv_window_conn" "$section" "recv_window_conn" ""
   config_get "recv_window" "$section" "recv_window" ""
   config_get "disable_mtu_discovery" "$section" "disable_mtu_discovery" ""
   config_get "xudp" "$section" "xudp" ""
   config_get "packet_encoding" "$section" "packet_encoding" ""
   config_get "global_padding" "$section" "global_padding" ""
   config_get "authenticated_length" "$section" "authenticated_length" ""
   config_get "wg_ip" "$section" "wg_ip" ""
   config_get "wg_ipv6" "$section" "wg_ipv6" ""
   config_get "private_key" "$section" "private_key" ""
   config_get "public_key" "$section" "public_key" ""
   config_get "preshared_key" "$section" "preshared_key" ""
   config_get "wg_dns" "$section" "wg_dns" ""
   config_get "public_key" "$section" "public_key" ""
   config_get "preshared_key" "$section" "preshared_key" ""
   config_get "wg_mtu" "$section" "wg_mtu" ""
   config_get "tc_ip" "$section" "tc_ip" ""
   config_get "tc_token" "$section" "tc_token" ""
   config_get "udp_relay_mode" "$section" "udp_relay_mode" ""
   config_get "congestion_controller" "$section" "congestion_controller" ""
   config_get "tc_alpn" "$section" "tc_alpn" ""
   config_get "disable_sni" "$section" "disable_sni" ""
   config_get "reduce_rtt" "$section" "reduce_rtt" ""
   config_get "heartbeat_interval" "$section" "heartbeat_interval" ""
   config_get "request_timeout" "$section" "request_timeout" ""
   config_get "max_udp_relay_packet_size" "$section" "max_udp_relay_packet_size" ""
   config_get "fast_open" "$section" "fast_open" ""
   config_get "fingerprint" "$section" "fingerprint" ""
   config_get "ports" "$section" "ports" ""
   config_get "hop_interval" "$section" "hop_interval" ""
   config_get "max_open_streams" "$section" "max_open_streams" ""
   config_get "obfs_password" "$section" "obfs_password" ""
   config_get "packet_addr" "$section" "packet_addr" ""
   config_get "client_fingerprint" "$section" "client_fingerprint" ""
   config_get "ip_version" "$section" "ip_version" ""
   config_get "tfo" "$section" "tfo" ""
   config_get "udp_over_tcp" "$section" "udp_over_tcp" ""
   config_get "reality_public_key" "$section" "reality_public_key" ""
   config_get "reality_short_id" "$section" "reality_short_id" ""
   config_get "obfs_version_hint" "$section" "obfs_version_hint" ""
   config_get "obfs_restls_script" "$section" "obfs_restls_script" ""
   config_get "multiplex" "$section" "multiplex" ""
   config_get "multiplex_protocol" "$section" "multiplex_protocol" ""
   config_get "multiplex_max_connections" "$section" "multiplex_max_connections" ""
   config_get "multiplex_min_streams" "$section" "multiplex_min_streams" ""
   config_get "multiplex_max_streams" "$section" "multiplex_max_streams" ""
   config_get "multiplex_padding" "$section" "multiplex_padding" ""
   config_get "multiplex_statistic" "$section" "multiplex_statistic" ""
   config_get "multiplex_only_tcp" "$section" "multiplex_only_tcp" ""
   config_get "other_parameters" "$section" "other_parameters" ""

   if [ "$enabled" = "0" ]; then
      return
   fi

   if [ -z "$type" ]; then
      return
   fi
   
   if [ -z "$name" ]; then
      return
   fi
   
   if [ -z "$server" ]; then
      return
   fi
   
   if [ -z "$port" ]; then
      return
   fi
   
   if [ -z "$password" ]; then
   	 if [ "$type" = "ss" ] || [ "$type" = "trojan" ] || [ "$type" = "ssr" ]; then
        return
     fi
   fi
   
   if [ ! -z "$if_game_proxy" ] && [ "$if_game_proxy" != "$name" ] && [ "$if_game_proxy_type" = "proxy" ]; then
      return
   fi
   
   if [ "$MIX_PROXY" != "1" ] && [ ! -z "$config" ] && [ "$config" != "$CONFIG_NAME" ] && [ "$config" != "all" ]; then
      return
   fi
   
   #避免重复节点
   if [ "$config" = "$CONFIG_NAME" ] || [ "$config" = "all" ]; then
      if [ "$(grep -w "^$name$" "$servers_name" |wc -l 2>/dev/null)" -ge 2 ] && [ -n "$(grep -w "name: \"$name\"" "$SERVER_FILE" 2>/dev/null)" ]; then
         return
      fi
   fi
   
   if [ "$config" = "$CONFIG_NAME" ] || [ "$config" = "all" ]; then
      if [ -n "$(grep -w "name: \"$name\"" "$SERVER_FILE" 2>/dev/null)" ]; then
         return
      elif [ "$(grep -w "^$name$" "$servers_name" |wc -l 2>/dev/null)" -ge 2 ] && [ -z "$(grep -w "name: \"$name\"" "$SERVER_FILE" 2>/dev/null)" ]; then
      	 sed -i "1,/^${name}$/{//d}" "$servers_name" 2>/dev/null
         return
      fi
   fi
   LOG_OUT "Start Writing【$CONFIG_NAME - $type - $name】Proxy To Config File..."
   
   if [ "$obfs" != "none" ] && [ -n "$obfs" ]; then
      if [ "$obfs" = "websocket" ]; then
         obfss="plugin: v2ray-plugin"
      elif [ "$obfs" = "shadow-tls" ]; then
        obfss="plugin: shadow-tls"
      elif [ "$obfs" = "restls" ]; then
        obfss="plugin: restls"
      else
         obfss="plugin: obfs"
      fi
   else
      obfss=""
   fi
   
   if [ "$obfs_vless" = "ws" ]; then
      obfs_vless="network: ws"
   fi
   
   if [ "$obfs_vless" = "grpc" ]; then
      obfs_vless="network: grpc"
   fi

   if [ "$obfs_vless" = "tcp" ]; then
      obfs_vless="network: tcp"
   fi
   
   if [ "$obfs_vmess" = "websocket" ]; then
      obfs_vmess="network: ws"
   fi
   
   if [ "$obfs_vmess" = "http" ]; then
      obfs_vmess="network: http"
   fi
   
   if [ "$obfs_vmess" = "h2" ]; then
      obfs_vmess="network: h2"
   fi
   
   if [ "$obfs_vmess" = "grpc" ]; then
      obfs_vmess="network: grpc"
   fi
   
   if [ ! -z "$custom" ] && [ "$type" = "vmess" ]; then
      custom="Host: \"$custom\""
   fi
   
   if [ ! -z "$path" ]; then
      if [ "$type" != "vmess" ]; then
         path="path: \"$path\""
      elif [ "$obfs_vmess" = "network: ws" ]; then
         path="ws-path: \"$path\""
      fi
   fi

#ss
   if [ "$type" = "ss" ]; then
cat >> "$SERVER_FILE" <<-EOF
  - name: "$name"
    type: $type
    server: "$server"
    port: $port
    cipher: $cipher
    password: "$password"
EOF
      if [ ! -z "$udp" ]; then
cat >> "$SERVER_FILE" <<-EOF
    udp: $udp
EOF
     fi
     if [ ! -z "$udp_over_tcp" ]; then
cat >> "$SERVER_FILE" <<-EOF
    udp-over-tcp: $udp_over_tcp
EOF
     fi
     if [ ! -z "$obfss" ]; then
cat >> "$SERVER_FILE" <<-EOF
    $obfss
    plugin-opts:
EOF
    if [ "$obfs" != "shadow-tls" ] && [ "$obfs" != "restls" ]; then
cat >> "$SERVER_FILE" <<-EOF
      mode: $obfs
EOF
    fi
        if [ ! -z "$host" ]; then
cat >> "$SERVER_FILE" <<-EOF
      host: "$host"
EOF
        fi
        if [  "$obfss" = "plugin: shadow-tls" ]; then
           if [ ! -z "$obfs_password" ]; then
cat >> "$SERVER_FILE" <<-EOF
      password: "$obfs_password"
EOF
           fi
           if [ ! -z "$fingerprint" ]; then
cat >> "$SERVER_FILE" <<-EOF
      fingerprint: "$fingerprint"
EOF
           fi
        fi
        if [  "$obfss" = "plugin: restls" ]; then
           if [ ! -z "$obfs_password" ]; then
cat >> "$SERVER_FILE" <<-EOF
      password: "$obfs_password"
EOF
           fi
           if [ ! -z "$obfs_version_hint" ]; then
cat >> "$SERVER_FILE" <<-EOF
      version-hint: "$obfs_version_hint"
EOF
           fi
           if [ ! -z "$obfs_restls_script" ]; then
cat >> "$SERVER_FILE" <<-EOF
      restls-script: "$obfs_restls_script"
EOF
           fi
        fi
        if [  "$obfss" = "plugin: v2ray-plugin" ]; then
           if [ ! -z "$tls" ]; then
cat >> "$SERVER_FILE" <<-EOF
      tls: $tls
EOF
           fi
           if [ ! -z "$skip_cert_verify" ]; then
cat >> "$SERVER_FILE" <<-EOF
      skip-cert-verify: $skip_cert_verify
EOF
           fi
           if [ ! -z "$path" ]; then
cat >> "$SERVER_FILE" <<-EOF
      $path
EOF
           fi
           if [ ! -z "$mux" ]; then
cat >> "$SERVER_FILE" <<-EOF
      mux: $mux
EOF
           fi
           if [ ! -z "$custom" ]; then
cat >> "$SERVER_FILE" <<-EOF
      headers:
        custom: $custom
EOF
           fi
           if [ ! -z "$fingerprint" ]; then
cat >> "$SERVER_FILE" <<-EOF
      fingerprint: "$fingerprint"
EOF
           fi
        fi
     fi
   fi
   
#ssr
if [ "$type" = "ssr" ]; then
cat >> "$SERVER_FILE" <<-EOF
  - name: "$name"
    type: $type
    server: "$server"
    port: $port
    cipher: $cipher_ssr
    password: "$password"
    obfs: "$obfs_ssr"
    protocol: "$protocol"
EOF
   if [ ! -z "$obfs_param" ]; then
cat >> "$SERVER_FILE" <<-EOF
    obfs-param: $obfs_param
EOF
   fi
   if [ ! -z "$protocol_param" ]; then
cat >> "$SERVER_FILE" <<-EOF
    protocol-param: $protocol_param
EOF
   fi
   if [ ! -z "$udp" ]; then
cat >> "$SERVER_FILE" <<-EOF
    udp: $udp
EOF
   fi
fi

#vmess
   if [ "$type" = "vmess" ]; then
cat >> "$SERVER_FILE" <<-EOF
  - name: "$name"
    type: $type
    server: "$server"
    port: $port
    uuid: $uuid
    alterId: $alterId
    cipher: $securitys
EOF
      if [ ! -z "$udp" ]; then
cat >> "$SERVER_FILE" <<-EOF
    udp: $udp
EOF
      fi
      if [ ! -z "$xudp" ]; then
cat >> "$SERVER_FILE" <<-EOF
    xudp: $xudp
EOF
      fi
      if [ ! -z "$packet_encoding" ]; then
cat >> "$SERVER_FILE" <<-EOF
    packet-encoding: "$packet_encoding"
EOF
      fi
      if [ ! -z "$global_padding" ]; then
cat >> "$SERVER_FILE" <<-EOF
    global-padding: $global_padding
EOF
      fi
      if [ ! -z "$authenticated_length" ]; then
cat >> "$SERVER_FILE" <<-EOF
    authenticated-length: $authenticated_length
EOF
      fi
      if [ ! -z "$skip_cert_verify" ]; then
cat >> "$SERVER_FILE" <<-EOF
    skip-cert-verify: $skip_cert_verify
EOF
      fi
      if [ ! -z "$tls" ]; then
cat >> "$SERVER_FILE" <<-EOF
    tls: $tls
EOF
      fi
      if [ ! -z "$fingerprint" ]; then
cat >> "$SERVER_FILE" <<-EOF
    fingerprint: "$fingerprint"
EOF
      fi
      if [ ! -z "$client_fingerprint" ]; then
cat >> "$SERVER_FILE" <<-EOF
    client-fingerprint: "$client_fingerprint"
EOF
      fi
      if [ ! -z "$servername" ] && [ "$tls" = "true" ]; then
cat >> "$SERVER_FILE" <<-EOF
    servername: "$servername"
EOF
      fi
      if [ "$obfs_vmess" != "none" ]; then
cat >> "$SERVER_FILE" <<-EOF
    $obfs_vmess
EOF
         if [ "$obfs_vmess" = "network: ws" ]; then
            if [ ! -z "$path" ]; then
cat >> "$SERVER_FILE" <<-EOF
    $path
EOF
            fi
            if [ ! -z "$custom" ]; then
cat >> "$SERVER_FILE" <<-EOF
    ws-headers:
      $custom
EOF
            fi
            if [ -n "$ws_opts_path" ] || [ -n "$ws_opts_headers" ] || [ -n "$max_early_data" ] || [ -n "$early_data_header_name" ]; then
cat >> "$SERVER_FILE" <<-EOF
    ws-opts:
EOF
               if [ -n "$ws_opts_path" ]; then
cat >> "$SERVER_FILE" <<-EOF
      path: "$ws_opts_path"
EOF
               fi
               if [ -n "$ws_opts_headers" ]; then
cat >> "$SERVER_FILE" <<-EOF
      headers:
EOF
                  config_list_foreach "$section" "ws_opts_headers" set_ws_headers
               fi
               if [ -n "$max_early_data" ]; then
cat >> "$SERVER_FILE" <<-EOF
      max-early-data: $max_early_data
EOF
               fi
               if [ -n "$early_data_header_name" ]; then
cat >> "$SERVER_FILE" <<-EOF
      early-data-header-name: "$early_data_header_name"
EOF
               fi
            fi
         fi
         if [ "$obfs_vmess" = "network: http" ]; then
            if [ ! -z "$http_path" ]; then
cat >> "$SERVER_FILE" <<-EOF
    http-opts:
      method: "GET"
      path:
EOF
               config_list_foreach "$section" "http_path" set_http_path
            fi
            if [ "$keep_alive" = "true" ]; then
cat >> "$SERVER_FILE" <<-EOF
      headers:
        Connection:
          - keep-alive
EOF
            fi
         fi
         #h2
         if [ "$obfs_vmess" = "network: h2" ]; then
            if [ ! -z "$h2_host" ]; then
cat >> "$SERVER_FILE" <<-EOF
    h2-opts:
      host:
EOF
               config_list_foreach "$section" "h2_host" set_h2_host
            fi
            if [ ! -z "$h2_path" ]; then
cat >> "$SERVER_FILE" <<-EOF
      path: $h2_path
EOF
            fi
         fi
         if [ ! -z "$grpc_service_name" ] && [ "$obfs_vmess" = "network: grpc" ]; then
cat >> "$SERVER_FILE" <<-EOF
    grpc-opts:
      grpc-service-name: "$grpc_service_name"
EOF
         fi
      fi
   fi

#Tuic
   if [ "$type" = "tuic" ]; then
cat >> "$SERVER_FILE" <<-EOF
  - name: "$name"
    type: $type
    server: "$server"
    port: $port
EOF
      if [ -n "$tc_ip" ]; then
cat >> "$SERVER_FILE" <<-EOF
    ip: "$tc_ip"
EOF
      fi
      if [ -n "$tc_token" ]; then
cat >> "$SERVER_FILE" <<-EOF
    token: "$tc_token"
EOF
      fi
      if [ -n "$udp_relay_mode" ]; then
cat >> "$SERVER_FILE" <<-EOF
    udp-relay-mode: "$udp_relay_mode"
EOF
      fi
      if [ -n "$congestion_controller" ]; then
cat >> "$SERVER_FILE" <<-EOF
    congestion-controller: "$congestion_controller"
EOF
      fi
      if [ -n "$tc_alpn" ]; then
cat >> "$SERVER_FILE" <<-EOF
    alpn:
EOF
      config_list_foreach "$section" "tc_alpn" set_alpn
      fi
      if [ -n "$disable_sni" ]; then
cat >> "$SERVER_FILE" <<-EOF
    disable-sni: $disable_sni
EOF
      fi
      if [ -n "$reduce_rtt" ]; then
cat >> "$SERVER_FILE" <<-EOF
    reduce-rtt: $reduce_rtt
EOF
      fi
      if [ -n "$fast_open" ]; then
cat >> "$SERVER_FILE" <<-EOF
    fast-open: $fast_open
EOF
      fi
      if [ -n "$heartbeat_interval" ]; then
cat >> "$SERVER_FILE" <<-EOF
    heartbeat-interval: $heartbeat_interval
EOF
      fi
      if [ -n "$request_timeout" ]; then
cat >> "$SERVER_FILE" <<-EOF
    request-timeout: $request_timeout
EOF
      fi
      if [ -n "$max_udp_relay_packet_size" ]; then
cat >> "$SERVER_FILE" <<-EOF
    max-udp-relay-packet-size: $max_udp_relay_packet_size
EOF
      fi
      if [ -n "$max_open_streams" ]; then
cat >> "$SERVER_FILE" <<-EOF
    max-open-streams: $max_open_streams
EOF
      fi
   fi

#WireGuard
   if [ "$type" = "wireguard" ]; then
cat >> "$SERVER_FILE" <<-EOF
  - name: "$name"
    type: $type
    server: "$server"
    port: $port
EOF
      if [ -n "$wg_ip" ]; then
cat >> "$SERVER_FILE" <<-EOF
    ip: "$wg_ip"
EOF
      fi
      if [ -n "$wg_ipv6" ]; then
cat >> "$SERVER_FILE" <<-EOF
    ipv6: "$wg_ipv6"
EOF
      fi
      if [ -n "$private_key" ]; then
cat >> "$SERVER_FILE" <<-EOF
    private-key: "$private_key"
EOF
      fi
      if [ -n "$public_key" ]; then
cat >> "$SERVER_FILE" <<-EOF
    public-key: "$public_key"
EOF
      fi
      if [ -n "$preshared_key" ]; then
cat >> "$SERVER_FILE" <<-EOF
    preshared-key: "$preshared_key"
EOF
      fi
      if [ -n "$preshared_key" ]; then
cat >> "$SERVER_FILE" <<-EOF
    dns:
EOF
      config_list_foreach "$section" "wg_dns" set_alpn
      fi
      if [ -n "$wg_mtu" ]; then
cat >> "$SERVER_FILE" <<-EOF
    mtu: "$wg_mtu"
EOF
      fi
      if [ -n "$udp" ]; then
cat >> "$SERVER_FILE" <<-EOF
    udp: $udp
EOF
      fi
   fi

#hysteria
   if [ "$type" = "hysteria" ]; then
cat >> "$SERVER_FILE" <<-EOF
  - name: "$name"
    type: $type
    server: "$server"
    port: $port
    protocol: $hysteria_protocol
EOF
      if [ -n "$hysteria_up" ]; then
cat >> "$SERVER_FILE" <<-EOF
    up: "$hysteria_up"
EOF
      fi
      if [ -n "$hysteria_down" ]; then
cat >> "$SERVER_FILE" <<-EOF
    down: "$hysteria_down"
EOF
      fi
      if [ -n "$skip_cert_verify" ]; then
cat >> "$SERVER_FILE" <<-EOF
    skip-cert-verify: $skip_cert_verify
EOF
      fi
      if [ -n "$sni" ]; then
cat >> "$SERVER_FILE" <<-EOF
    sni: "$sni"
EOF
      fi
      if [ -n "$hysteria_alpn" ]; then
         if [ -z "$(echo $hysteria_alpn |grep ' ')" ]; then
cat >> "$SERVER_FILE" <<-EOF
    alpn: 
      - "$hysteria_alpn"
EOF
         else
cat >> "$SERVER_FILE" <<-EOF
    alpn:
EOF
      config_list_foreach "$section" "hysteria_alpn" set_alpn
         fi
      fi
      if [ -n "$hysteria_obfs" ]; then
cat >> "$SERVER_FILE" <<-EOF
    obfs: "$hysteria_obfs"
EOF
      fi
      if [ -n "$hysteria_auth" ]; then
cat >> "$SERVER_FILE" <<-EOF
    auth: "$hysteria_auth"
EOF
      fi
      if [ -n "$hysteria_auth_str" ]; then
cat >> "$SERVER_FILE" <<-EOF
    auth-str: "$hysteria_auth_str"
EOF
      fi
      if [ -n "$hysteria_ca" ]; then
cat >> "$SERVER_FILE" <<-EOF
    ca: "$hysteria_ca"
EOF
      fi
      if [ -n "$hysteria_ca_str" ]; then
cat >> "$SERVER_FILE" <<-EOF
    ca-str: "$hysteria_ca_str"
EOF
      fi
      if [ -n "$recv_window_conn" ]; then
cat >> "$SERVER_FILE" <<-EOF
    recv-window-conn: "$recv_window_conn"
EOF
      fi
      if [ -n "$recv_window" ]; then
cat >> "$SERVER_FILE" <<-EOF
    recv-window: "$recv_window"
EOF
      fi
      if [ -n "$disable_mtu_discovery" ]; then
cat >> "$SERVER_FILE" <<-EOF
    disable-mtu-discovery: $disable_mtu_discovery
EOF
      fi
      if [ -n "$fast_open" ]; then
cat >> "$SERVER_FILE" <<-EOF
    fast-open: $fast_open
EOF
      fi
      if [ -n "$fingerprint" ]; then
cat >> "$SERVER_FILE" <<-EOF
    fingerprint: "$fingerprint"
EOF
      fi
      if [ -n "$ports" ]; then
cat >> "$SERVER_FILE" <<-EOF
    ports: $ports
EOF
      fi
      if [ -n "$hop_interval" ]; then
cat >> "$SERVER_FILE" <<-EOF
    hop-interval: $hop_interval
EOF
      fi
   fi

#vless
   if [ "$type" = "vless" ]; then
cat >> "$SERVER_FILE" <<-EOF
  - name: "$name"
    type: $type
    server: "$server"
    port: $port
    uuid: $uuid
EOF
      if [ ! -z "$udp" ]; then
cat >> "$SERVER_FILE" <<-EOF
    udp: $udp
EOF
      fi
      if [ ! -z "$xudp" ]; then
cat >> "$SERVER_FILE" <<-EOF
    xudp: $xudp
EOF
      fi
      if [ ! -z "$packet_addr" ]; then
cat >> "$SERVER_FILE" <<-EOF
    packet-addr: $packet_addr
EOF
      fi
      if [ ! -z "$packet_encoding" ]; then
cat >> "$SERVER_FILE" <<-EOF
    packet-encoding: "$packet_encoding"
EOF
      fi
      if [ ! -z "$skip_cert_verify" ]; then
cat >> "$SERVER_FILE" <<-EOF
    skip-cert-verify: $skip_cert_verify
EOF
      fi
      if [ ! -z "$tls" ]; then
cat >> "$SERVER_FILE" <<-EOF
    tls: $tls
EOF
      fi
      if [ ! -z "$fingerprint" ]; then
cat >> "$SERVER_FILE" <<-EOF
    fingerprint: "$fingerprint"
EOF
      fi
      if [ ! -z "$client_fingerprint" ]; then
cat >> "$SERVER_FILE" <<-EOF
    client-fingerprint: "$client_fingerprint"
EOF
      fi
      if [ ! -z "$servername" ]; then
cat >> "$SERVER_FILE" <<-EOF
    servername: "$servername"
EOF
      fi
      if [ -n "$obfs_vless" ]; then
cat >> "$SERVER_FILE" <<-EOF
    $obfs_vless
EOF
         if [ "$obfs_vless" = "network: ws" ]; then
            if [ -n "$ws_opts_path" ] || [ -n "$ws_opts_headers" ]; then
cat >> "$SERVER_FILE" <<-EOF
    ws-opts:
EOF
               if [ -n "$ws_opts_path" ]; then
cat >> "$SERVER_FILE" <<-EOF
      path: "$ws_opts_path"
EOF
               fi
               if [ -n "$ws_opts_headers" ]; then
cat >> "$SERVER_FILE" <<-EOF
      headers:
EOF
                  config_list_foreach "$section" "ws_opts_headers" set_ws_headers
               fi
            fi
         fi
         if [ ! -z "$grpc_service_name" ] && [ "$obfs_vless" = "network: grpc" ]; then
cat >> "$SERVER_FILE" <<-EOF
    grpc-opts:
      grpc-service-name: "$grpc_service_name"
EOF
            if [ -n "$reality_public_key" ] || [ -n "$reality_short_id" ]; then
cat >> "$SERVER_FILE" <<-EOF
    reality-opts:
EOF
            fi
            if [ -n "$reality_public_key" ]; then
cat >> "$SERVER_FILE" <<-EOF
      public-key: "$reality_public_key"
EOF
            fi
            if [ -n "$reality_short_id" ]; then
cat >> "$SERVER_FILE" <<-EOF
      short-id: "$reality_short_id"
EOF
            fi
         fi
         if [ "$obfs_vless" = "network: tcp" ]; then
            if [ ! -z "$vless_flow" ]; then
cat >> "$SERVER_FILE" <<-EOF
    flow: "$vless_flow"
EOF
            fi
            if [ -n "$reality_public_key" ] || [ -n "$reality_short_id" ]; then
cat >> "$SERVER_FILE" <<-EOF
    reality-opts:
EOF
            fi
            if [ -n "$reality_public_key" ]; then
cat >> "$SERVER_FILE" <<-EOF
      public-key: "$reality_public_key"
EOF
            fi
            if [ -n "$reality_short_id" ]; then
cat >> "$SERVER_FILE" <<-EOF
      short-id: "$reality_short_id"
EOF
            fi
         fi
      fi
   fi
   
#socks5
   if [ "$type" = "socks5" ]; then
cat >> "$SERVER_FILE" <<-EOF
  - name: "$name"
    type: $type
    server: "$server"
    port: $port
EOF
      if [ ! -z "$auth_name" ]; then
cat >> "$SERVER_FILE" <<-EOF
    username: "$auth_name"
EOF
      fi
      if [ ! -z "$auth_pass" ]; then
cat >> "$SERVER_FILE" <<-EOF
    password: "$auth_pass"
EOF
      fi
      if [ ! -z "$udp" ]; then
cat >> "$SERVER_FILE" <<-EOF
    udp: $udp
EOF
      fi
      if [ ! -z "$skip_cert_verify" ]; then
cat >> "$SERVER_FILE" <<-EOF
    skip-cert-verify: $skip_cert_verify
EOF
      fi
      if [ ! -z "$tls" ]; then
cat >> "$SERVER_FILE" <<-EOF
    tls: $tls
EOF
      fi
      if [ ! -z "$fingerprint" ]; then
cat >> "$SERVER_FILE" <<-EOF
    fingerprint: "$fingerprint"
EOF
      fi
   fi

#http
   if [ "$type" = "http" ]; then
cat >> "$SERVER_FILE" <<-EOF
  - name: "$name"
    type: $type
    server: "$server"
    port: $port
EOF
      if [ ! -z "$auth_name" ]; then
cat >> "$SERVER_FILE" <<-EOF
    username: "$auth_name"
EOF
      fi
      if [ ! -z "$auth_pass" ]; then
cat >> "$SERVER_FILE" <<-EOF
    password: "$auth_pass"
EOF
      fi
      if [ ! -z "$skip_cert_verify" ]; then
cat >> "$SERVER_FILE" <<-EOF
    skip-cert-verify: $skip_cert_verify
EOF
      fi
      if [ ! -z "$tls" ]; then
cat >> "$SERVER_FILE" <<-EOF
    tls: $tls
EOF
      fi
      if [ ! -z "$sni" ]; then
cat >> "$SERVER_FILE" <<-EOF
    sni: "$sni"
EOF
      fi
      if [ -n "$http_headers" ]; then
cat >> "$SERVER_FILE" <<-EOF
    headers:
EOF
      config_list_foreach "$section" "http_headers" set_ws_headers
      fi
   fi

#trojan
   if [ "$type" = "trojan" ]; then
cat >> "$SERVER_FILE" <<-EOF
  - name: "$name"
    type: $type
    server: "$server"
    port: $port
    password: "$password"
EOF
   if [ ! -z "$udp" ]; then
cat >> "$SERVER_FILE" <<-EOF
    udp: $udp
EOF
   fi
   if [ ! -z "$sni" ]; then
cat >> "$SERVER_FILE" <<-EOF
    sni: "$sni"
EOF
   fi
   if [ ! -z "$alpn" ]; then
cat >> "$SERVER_FILE" <<-EOF
    alpn:
EOF
      config_list_foreach "$section" "alpn" set_alpn
   fi
   if [ ! -z "$skip_cert_verify" ]; then
cat >> "$SERVER_FILE" <<-EOF
    skip-cert-verify: $skip_cert_verify
EOF
   fi
   if [ ! -z "$fingerprint" ]; then
cat >> "$SERVER_FILE" <<-EOF
  fingerprint: "$fingerprint"
EOF
   fi
   if [ ! -z "$client_fingerprint" ]; then
cat >> "$SERVER_FILE" <<-EOF
  client-fingerprint: "$client_fingerprint"
EOF
   fi
   if [ ! -z "$grpc_service_name" ]; then
cat >> "$SERVER_FILE" <<-EOF
    network: grpc
    grpc-opts:
      grpc-service-name: "$grpc_service_name"
EOF
   fi
   if [ "$obfs_trojan" = "ws" ]; then
      if [ -n "$trojan_ws_path" ] || [ -n "$trojan_ws_headers" ]; then
cat >> "$SERVER_FILE" <<-EOF
    network: ws
    ws-opts:
EOF
      fi
      if [ -n "$trojan_ws_path" ]; then
cat >> "$SERVER_FILE" <<-EOF
      path: "$trojan_ws_path"
EOF
      fi
      if [ -n "$trojan_ws_headers" ]; then
cat >> "$SERVER_FILE" <<-EOF
      headers:
EOF
         config_list_foreach "$section" "trojan_ws_headers" set_ws_headers
      fi
   fi
   fi

#snell
   if [ "$type" = "snell" ]; then
cat >> "$SERVER_FILE" <<-EOF
  - name: "$name"
    type: $type
    server: "$server"
    port: $port
    psk: $psk
EOF
   if [ -n "$snell_version" ]; then
cat >> "$SERVER_FILE" <<-EOF
    version: "$snell_version"
EOF
   fi
   if [ "$obfs_snell" != "none" ] && [ ! -z "$host" ]; then
cat >> "$SERVER_FILE" <<-EOF
    obfs-opts:
      mode: $obfs_snell
      host: "$host"
EOF
   fi
   fi

#ip_version
   if [ ! -z "$ip_version" ]; then
cat >> "$SERVER_FILE" <<-EOF
    ip-version: "$ip_version"
EOF
   fi

#TFO
   if [ ! -z "$tfo" ]; then
cat >> "$SERVER_FILE" <<-EOF
    tfo: $tfo
EOF
   fi

#Multiplex
   if [ ! -z "$multiplex" ]; then
cat >> "$SERVER_FILE" <<-EOF
    smux:
      enabled: $multiplex
EOF
      if [ -n "$multiplex_protocol" ]; then
cat >> "$SERVER_FILE" <<-EOF
    protocol: $multiplex_protocol
EOF
      fi
      if [ -n "$multiplex_max_connections" ]; then
cat >> "$SERVER_FILE" <<-EOF
    max-connections: $multiplex_max_connections
EOF
      fi
      if [ -n "$multiplex_min_streams" ]; then
cat >> "$SERVER_FILE" <<-EOF
    min-streams: $multiplex_min_streams
EOF
      fi
      if [ -n "$multiplex_max_streams" ]; then
cat >> "$SERVER_FILE" <<-EOF
    max-streams: $multiplex_max_streams
EOF
      fi
      if [ -n "$multiplex_padding" ]; then
cat >> "$SERVER_FILE" <<-EOF
    padding: $multiplex_padding
EOF
      fi
      if [ -n "$multiplex_statistic" ]; then
cat >> "$SERVER_FILE" <<-EOF
    statistic: $multiplex_statistic
EOF
      fi
      if [ -n "$multiplex_only_tcp" ]; then
cat >> "$SERVER_FILE" <<-EOF
    only-tcp: $multiplex_only_tcp
EOF
      fi
   fi

#interface-name
   if [ -n "$interface_name" ]; then
cat >> "$SERVER_FILE" <<-EOF
    interface-name: "$interface_name"
EOF
   fi

#routing_mark
   if [ -n "$routing_mark" ]; then
cat >> "$SERVER_FILE" <<-EOF
    routing-mark: "$routing_mark"
EOF
   fi

#other_parameters
   if [ -n "$other_parameters" ]; then
      echo -e "$other_parameters" >> "$SERVER_FILE"
   fi
}

new_servers_group_set()
{
   local section="$1"
   config_get_bool "enabled" "$section" "enabled" "1"
   config_get "name" "$section" "name" ""
   
   if [ "$enabled" = "0" ]; then
      return
   fi
   
   if [ -z "$name" ] || [ "$(echo $name.yaml)" != "$CONFIG_NAME" ]; then
      return
   fi
   
   new_servers_group_set=1
   
}

yml_servers_name_get()
{
	 local section="$1"
   config_get "name" "$section" "name" ""
   [ ! -z "$name" ] && {
      echo "$name" >>"$servers_name"
   }
}

yml_proxy_provider_name_get()
{
	 local section="$1"
   config_get "name" "$section" "name" ""
   [ ! -z "$name" ] && {
      echo "$name" >>"$proxy_provider_name"
   }
}

#创建配置文件
if_game_proxy="$1"
if_game_proxy_type="$2"
#创建对比文件防止重复
config_load "openclash"
config_foreach yml_servers_name_get "servers"
config_foreach yml_proxy_provider_name_get "proxy-provider"
#判断是否启用保留配置
config_foreach new_servers_group_set "config_subscribe"
#proxy-provider
LOG_OUT "Start Writing【$CONFIG_NAME】Proxy-providers Setting..."
echo "proxy-providers:" >$PROXY_PROVIDER_FILE
rm -rf /tmp/Proxy_Provider
config_foreach yml_proxy_provider_set "proxy-provider"
sed -i "s/^ \{0,\}/      - /" /tmp/Proxy_Provider 2>/dev/null #添加参数
if [ "$(grep "-" /tmp/Proxy_Provider 2>/dev/null |wc -l)" -eq 0 ]; then
   rm -rf $PROXY_PROVIDER_FILE
   rm -rf /tmp/Proxy_Provider
fi
rm -rf $proxy_provider_name

#proxy
rule_sources=$(uci -q get openclash.config.rule_sources)
create_config=$(uci -q get openclash.config.create_config)
LOG_OUT "Start Writing【$CONFIG_NAME】Proxies Setting..."
echo "proxies:" >$SERVER_FILE
config_foreach yml_servers_set "servers"
egrep '^ {0,}-' $SERVER_FILE |grep name: |awk -F 'name: ' '{print $2}' |sed 's/,.*//' 2>/dev/null >/tmp/Proxy_Server 2>&1
if [ -s "/tmp/Proxy_Server" ]; then
   sed -i "s/^ \{0,\}/      - /" /tmp/Proxy_Server 2>/dev/null #添加参数
else
   rm -rf $SERVER_FILE
   rm -rf /tmp/Proxy_Server
fi
rm -rf $servers_name

#一键创建配置文件
if [ "$rule_sources" = "ConnersHua" ] && [ "$servers_if_update" != "1" ] && [ -z "$if_game_proxy" ]; then
LOG_OUT "Creating By Using Connershua (rule set) Rules..."
echo "proxy-groups:" >>$SERVER_FILE
cat >> "$SERVER_FILE" <<-EOF
  - name: Auto - UrlTest
    type: url-test
EOF
if [ -f "/tmp/Proxy_Server" ]; then
cat >> "$SERVER_FILE" <<-EOF
    proxies:
EOF
fi
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
    url: http://cp.cloudflare.com/generate_204
    interval: "600"
    tolerance: "150"
  - name: Proxy
    type: select
    proxies:
      - Auto - UrlTest
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Domestic
    type: select
    proxies:
      - DIRECT
      - Proxy
  - name: Others
    type: select
    proxies:
      - Proxy
      - DIRECT
      - Domestic
  - name: Asian TV
    type: select
    proxies:
      - DIRECT
      - Proxy
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Global TV
    type: select
    proxies:
      - Proxy
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
config_load "openclash"
config_foreach yml_other_rules_del "other_rules" "$CONFIG_NAME" "ConnersHua"
uci_name_tmp=$(uci -q add openclash other_rules)
uci_set="uci -q set openclash.$uci_name_tmp."
${UCI_SET}rule_source="1"
${uci_set}enable="1"
${uci_set}rule_name="ConnersHua"
${uci_set}config="$CONFIG_NAME"
${uci_set}GlobalTV="Global TV"
${uci_set}AsianTV="Asian TV"
${uci_set}Proxy="Proxy"
${uci_set}AdBlock="AdBlock"
${uci_set}Domestic="Domestic"
${uci_set}Others="Others"

[ "$config_auto_update" -eq 1 ] && [ "$new_servers_group_set" -eq 1 ] && {
	${UCI_SET}servers_update="1"
	${UCI_DEL_LIST}="all" >/dev/null 2>&1
	${UCI_DEL_LIST}="Auto - UrlTest" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Auto - UrlTest" >/dev/null 2>&1
	${UCI_DEL_LIST}="Proxy" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Proxy" >/dev/null 2>&1
	${UCI_DEL_LIST}="Asian TV" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Asian TV" >/dev/null 2>&1
	${UCI_DEL_LIST}="Global TV" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Global TV" >/dev/null 2>&1
}
elif [ "$rule_sources" = "lhie1" ] && [ "$servers_if_update" != "1" ] && [ -z "$if_game_proxy" ]; then
LOG_OUT "Creating By Using lhie1 Rules..."
echo "proxy-groups:" >>$SERVER_FILE
cat >> "$SERVER_FILE" <<-EOF
  - name: Auto - UrlTest
    type: url-test
EOF
if [ -f "/tmp/Proxy_Server" ]; then
cat >> "$SERVER_FILE" <<-EOF
    proxies:
EOF
fi
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
    url: http://cp.cloudflare.com/generate_204
    interval: "600"
    tolerance: "150"
  - name: Proxy
    type: select
    proxies:
      - Auto - UrlTest
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Domestic
    type: select
    proxies:
      - DIRECT
      - Proxy
  - name: Others
    type: select
    proxies:
      - Proxy
      - DIRECT
      - Domestic
  - name: Microsoft
    type: select
    proxies:
      - DIRECT
      - Proxy
EOF
cat >> "$SERVER_FILE" <<-EOF
  - name: ChatGPT
    type: select
    proxies:
      - Proxy
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Apple
    type: select
    proxies:
      - DIRECT
      - Proxy
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Apple TV
    type: select
    proxies:
      - Proxy
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Google FCM
    type: select
    proxies:
      - DIRECT
      - Proxy
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Scholar
    type: select
    proxies:
      - DIRECT
      - Proxy
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Bilibili
    type: select
    proxies:
      - Asian TV
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Bahamut
    type: select
    proxies:
      - Global TV
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: HBO Max
    type: select
    proxies:
      - Global TV
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: HBO Go
    type: select
    proxies:
      - Global TV
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Pornhub
    type: select
    proxies:
      - Global TV
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Netflix
    type: select
    proxies:
      - Global TV
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Disney
    type: select
    proxies:
      - Global TV
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Youtube
    type: select
    disable-udp: true
    proxies:
      - Global TV
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Discovery Plus
    type: select
    proxies:
      - Global TV
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: DAZN
    type: select
    proxies:
      - Global TV
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Spotify
    type: select
    proxies:
      - Global TV
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Steam
    type: select
    proxies:
      - DIRECT
      - Proxy
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: miHoYo
    type: select
    proxies:
      - DIRECT
      - Proxy
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: AdBlock
    type: select
    proxies:
      - REJECT
      - DIRECT
      - Proxy
  - name: Asian TV
    type: select
    proxies:
      - DIRECT
      - Proxy
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Global TV
    type: select
    proxies:
      - Proxy
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Speedtest
    type: select
    proxies:
      - Proxy
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Telegram
    type: select
    proxies:
      - Proxy
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Crypto
    type: select
    proxies:
      - Proxy
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Discord
    type: select
    proxies:
      - Proxy
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: PayPal
    type: select
    proxies:
      - DIRECT
      - Proxy
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
config_load "openclash"
config_foreach yml_other_rules_del "other_rules" "$CONFIG_NAME" "lhie1"
uci_name_tmp=$(uci -q add openclash other_rules)
uci_set="uci -q set openclash.$uci_name_tmp."
${UCI_SET}rule_source="1"
${uci_set}enable="1"
${uci_set}rule_name="lhie1"
${uci_set}config="$CONFIG_NAME"
${uci_set}GlobalTV="Global TV"
${uci_set}AsianTV="Asian TV"
${uci_set}Proxy="Proxy"
${uci_set}Youtube="Youtube"
${uci_set}Bilibili="Bilibili"
${uci_set}Bahamut="Bahamut"
${uci_set}HBOMax="HBO Max"
${uci_set}HBOGo="HBO Go"
${uci_set}Pornhub="Pornhub"
${uci_set}Apple="Apple"
${uci_set}AppleTV="Apple TV"
${uci_set}GoogleFCM="Google FCM"
${uci_set}Scholar="Scholar"
${uci_set}Microsoft="Microsoft"
${uci_set}ChatGPT="ChatGPT"
${uci_set}Netflix="Netflix"
${uci_set}Discovery="Discovery Plus"
${uci_set}DAZN="DAZN"
${uci_set}Disney="Disney"
${uci_set}Spotify="Spotify"
${uci_set}Steam="Steam"
${uci_set}miHoYo="miHoYo"
${uci_set}AdBlock="AdBlock"
${uci_set}Speedtest="Speedtest"
${uci_set}Telegram="Telegram"
${uci_set}Crypto="Crypto"
${uci_set}Discord="Discord"
${uci_set}PayPal="PayPal"
${uci_set}Domestic="Domestic"
${uci_set}Others="Others"

[ "$config_auto_update" -eq 1 ] && [ "$new_servers_group_set" -eq 1 ] && {
	${UCI_SET}servers_update="1"
	${UCI_DEL_LIST}="all" >/dev/null 2>&1
	${UCI_DEL_LIST}="Auto - UrlTest" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Auto - UrlTest" >/dev/null 2>&1
	${UCI_DEL_LIST}="Proxy" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Proxy" >/dev/null 2>&1
	${UCI_DEL_LIST}="Youtube" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Youtube" >/dev/null 2>&1
	${UCI_DEL_LIST}="Bilibili" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Bilibili" >/dev/null 2>&1
	${UCI_DEL_LIST}="Bahamut" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Bahamut" >/dev/null 2>&1
	${UCI_DEL_LIST}="HBO Max" >/dev/null 2>&1 && ${UCI_ADD_LIST}="HBO Max" >/dev/null 2>&1
	${UCI_DEL_LIST}="HBO Go" >/dev/null 2>&1 && ${UCI_ADD_LIST}="HBO Go" >/dev/null 2>&1
	${UCI_DEL_LIST}="Pornhub" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Pornhub" >/dev/null 2>&1
	${UCI_DEL_LIST}="Asian TV" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Asian TV" >/dev/null 2>&1
	${UCI_DEL_LIST}="Global TV" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Global TV" >/dev/null 2>&1
	${UCI_DEL_LIST}="Netflix" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Netflix" >/dev/null 2>&1
	${UCI_DEL_LIST}="Discovery Plus" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Discovery Plus" >/dev/null 2>&1
	${UCI_DEL_LIST}="DAZN" >/dev/null 2>&1 && ${UCI_ADD_LIST}="DAZN" >/dev/null 2>&1
  ${UCI_DEL_LIST}="ChatGPT" >/dev/null 2>&1 && ${UCI_ADD_LIST}="ChatGPT" >/dev/null 2>&1
  ${UCI_DEL_LIST}="Apple TV" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Apple TV" >/dev/null 2>&1
	${UCI_DEL_LIST}="Google FCM" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Google FCM" >/dev/null 2>&1
	${UCI_DEL_LIST}="Scholar" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Scholar" >/dev/null 2>&1
	${UCI_DEL_LIST}="Disney" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Disney" >/dev/null 2>&1
	${UCI_DEL_LIST}="Spotify" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Spotify" >/dev/null 2>&1
	${UCI_DEL_LIST}="Steam" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Steam" >/dev/null 2>&1
  ${UCI_DEL_LIST}="miHoYo" >/dev/null 2>&1 && ${UCI_ADD_LIST}="miHoYo" >/dev/null 2>&1
	${UCI_DEL_LIST}="Telegram" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Telegram" >/dev/null 2>&1
  ${UCI_DEL_LIST}="Crypto" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Crypto" >/dev/null 2>&1
  ${UCI_DEL_LIST}="Discord" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Discord" >/dev/null 2>&1
	${UCI_DEL_LIST}="PayPal" >/dev/null 2>&1 && ${UCI_ADD_LIST}="PayPal" >/dev/null 2>&1
	${UCI_DEL_LIST}="Speedtest" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Speedtest" >/dev/null 2>&1
  ${UCI_DEL_LIST}="Others" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Others" >/dev/null 2>&1
}
elif [ "$rule_sources" = "ConnersHua_return" ] && [ "$servers_if_update" != "1" ] && [ -z "$if_game_proxy" ]; then
LOG_OUT "Creating By Using ConnersHua Return Rules..."
echo "proxy-groups:" >>$SERVER_FILE
cat >> "$SERVER_FILE" <<-EOF
  - name: Auto - UrlTest
    type: url-test
EOF
if [ -f "/tmp/Proxy_Server" ]; then
cat >> "$SERVER_FILE" <<-EOF
    proxies:
EOF
fi
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
    url: http://cp.cloudflare.com/generate_204
    interval: "600"
    tolerance: "150"
  - name: Proxy
    type: select
    proxies:
      - Auto - UrlTest
      - DIRECT
EOF
cat /tmp/Proxy_Server >> $SERVER_FILE 2>/dev/null
if [ -f "/tmp/Proxy_Provider" ]; then
cat >> "$SERVER_FILE" <<-EOF
    use:
EOF
fi
cat /tmp/Proxy_Provider >> $SERVER_FILE 2>/dev/null
cat >> "$SERVER_FILE" <<-EOF
  - name: Others
    type: select
    proxies:
      - Proxy
      - DIRECT
EOF
config_load "openclash"
config_foreach yml_other_rules_del "other_rules" "$CONFIG_NAME" "ConnersHua_return"
uci_name_tmp=$(uci -q add openclash other_rules)
uci_set="uci -q set openclash.$uci_name_tmp."
${UCI_SET}rule_source="1"
${uci_set}enable="1"
${uci_set}rule_name="ConnersHua_return"
${uci_set}config="$CONFIG_NAME"
${uci_set}Proxy="Proxy"
${uci_set}Others="Others"
[ "$config_auto_update" -eq 1 ] && [ "$new_servers_group_set" -eq 1 ] && {
	${UCI_SET}servers_update="1"
	${UCI_DEL_LIST}="all" >/dev/null 2>&1
	${UCI_DEL_LIST}="Auto - UrlTest" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Auto - UrlTest" >/dev/null 2>&1
	${UCI_DEL_LIST}="Proxy" >/dev/null 2>&1 && ${UCI_ADD_LIST}="Proxy" >/dev/null 2>&1
}
fi

if [ "$create_config" != "0" ] && [ "$servers_if_update" != "1" ] && [ -z "$if_game_proxy" ]; then
   echo "rules:" >>$SERVER_FILE
   LOG_OUT "Config File【$CONFIG_NAME】Created Successful, Updating Proxies, Proxy-providers, Groups..."
   cat "$PROXY_PROVIDER_FILE" > "$CONFIG_FILE" 2>/dev/null
   cat "$SERVER_FILE" >> "$CONFIG_FILE" 2>/dev/null
   /usr/share/openclash/yml_groups_get.sh >/dev/null 2>&1
elif [ -z "$if_game_proxy" ]; then
   LOG_OUT "Proxies, Proxy-providers, Groups Edited Successful, Updating Config File【$CONFIG_NAME】..."
   config_hash=$(ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "Value = YAML.load_file('$CONFIG_FILE'); puts Value" 2>/dev/null)
   if [ "$config_hash" != "false" ] && [ -n "$config_hash" ]; then
      ruby_cover "$CONFIG_FILE" "['proxies']" "$SERVER_FILE" "proxies"
      ruby_cover "$CONFIG_FILE" "['proxy-providers']" "$PROXY_PROVIDER_FILE" "proxy-providers"
      ruby_cover "$CONFIG_FILE" "['proxy-groups']" "/tmp/yaml_groups.yaml" "proxy-groups"
   else
      cat "$SERVER_FILE" "$PROXY_PROVIDER_FILE" "/tmp/yaml_groups.yaml" > "$CONFIG_FILE" 2>/dev/null
   fi
fi

if [ -z "$if_game_proxy" ]; then
   rm -rf $SERVER_FILE 2>/dev/null
   rm -rf $PROXY_PROVIDER_FILE 2>/dev/null
   rm -rf /tmp/yaml_groups.yaml 2>/dev/null
   LOG_OUT "Config File【$CONFIG_NAME】Write Successful!"
   SLOG_CLEAN
fi
rm -rf /tmp/Proxy_Server 2>/dev/null
rm -rf /tmp/Proxy_Provider 2>/dev/null
del_lock
${UCI_SET}enable=1 2>/dev/null
[ "$(uci -q get openclash.config.servers_if_update)" == "0" ] && [ -z "$if_game_proxy" ] && /etc/init.d/openclash restart >/dev/null 2>&1
${UCI_SET}servers_if_update=0
uci -q commit openclash
