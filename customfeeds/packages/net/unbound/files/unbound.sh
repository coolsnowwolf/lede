#!/bin/sh
##############################################################################
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# Copyright (C) 2016 Eric Luehrsen
#
##############################################################################
#
# Unbound is a full featured recursive server with many options. The UCI
# provided tries to simplify and bundle options. This should make Unbound
# easier to deploy. Even light duty routers may resolve recursively instead of
# depending on a stub with the ISP. The UCI also attempts to replicate dnsmasq
# features as used in base LEDE/OpenWrt. If there is a desire for more
# detailed tuning, then manual conf file overrides are also made available.
#
##############################################################################

# while useful (sh)ellcheck is pedantic and noisy
# shellcheck disable=1091,2002,2004,2034,2039,2086,2094,2140,2154,2155

UB_B_AUTH_ROOT=0
UB_B_DNS_ASSIST=0
UB_B_DNSSEC=0
UB_B_DNS64=0
UB_B_EXT_STATS=0
UB_B_GATE_NAME=0
UB_B_HIDE_BIND=1
UB_B_IF_AUTO=1
UB_B_LOCL_BLCK=0
UB_B_LOCL_SERV=1
UB_B_MAN_CONF=0
UB_B_NTP_BOOT=1
UB_B_QUERY_MIN=0
UB_B_QRY_MINST=0
UB_B_SLAAC6_MAC=0

UB_D_CONTROL=0
UB_D_DOMAIN_TYPE=static
UB_D_DHCP_LINK=none
UB_D_EXTRA_DNS=0
UB_D_LAN_FQDN=0
UB_D_PRIV_BLCK=1
UB_D_PROTOCOL=mixed
UB_D_RESOURCE=small
UB_D_RECURSION=passive
UB_D_VERBOSE=1
UB_D_WAN_FQDN=0

UB_IP_DNS64="64:ff9b::/96"

UB_N_EDNS_SIZE=1232
UB_N_RX_PORT=53
UB_N_ROOT_AGE=9
UB_N_THREADS=1
UB_N_RATE_LMT=0

UB_TTL_MIN=120
UB_TXT_DOMAIN=lan
UB_TXT_HOSTNAME=thisrouter

##############################################################################

# reset as a combo with UB_B_NTP_BOOT and some time stamp files
UB_B_READY=1

# keep track of assignments during inserted resource records
UB_LIST_NETW_ALL=""
UB_LIST_NETW_LAN=""
UB_LIST_NETW_WAN=""
UB_LIST_INSECURE=""
UB_LIST_ZONE_SERVERS=""
UB_LIST_ZONE_NAMES=""

##############################################################################

. /lib/functions.sh
. /lib/functions/network.sh

. /usr/lib/unbound/defaults.sh
. /usr/lib/unbound/dnsmasq.sh
. /usr/lib/unbound/iptools.sh

##############################################################################

bundle_all_networks() {
  local cfg="$1"
  local ifname ifdashname validip
  local subnet subnets subnets4 subnets6

  network_get_subnets  subnets4 "$cfg"
  network_get_subnets6 subnets6 "$cfg"
  network_get_device   ifname   "$cfg"

  ifdashname="${ifname//./-}"
  subnets="$subnets4 $subnets6"


  if [ -n "$subnets" ] ; then
    for subnet in $subnets ; do
      validip=$( valid_subnet_any $subnet )


      if [ "$validip" = "ok" ] ; then
        UB_LIST_NETW_ALL="$UB_LIST_NETW_ALL $ifdashname@$subnet"
      fi
    done
  fi
}

##############################################################################

bundle_dhcp_networks() {
  local cfg="$1"
  local interface ifsubnet ifname ifdashname ignore

  config_get_bool    ignore "$cfg" ignore 0
  config_get         interface "$cfg" interface ""
  network_get_device ifname "$interface"
  ifdashname="${ifname//./-}"


  if [ $ignore -eq 0 ] && [ -n "$ifdashname" ] \
  && [ -n "$UB_LIST_NETW_ALL" ] ; then
    for ifsubnet in $UB_LIST_NETW_ALL ; do
      case $ifsubnet in
        "${ifdashname}"@*)
          # Special GLA protection for local block; ULA protected default
          UB_LIST_NETW_LAN="$UB_LIST_NETW_LAN $ifsubnet"
          ;;
      esac
    done
  fi
}

##############################################################################

bundle_lan_networks() {
  local interface="$1"
  local ifsubnet ifname ifdashname

  network_get_device ifname "$interface"
  ifdashname="${ifname//./-}"


  if [ -n "$ifdashname" ] && [ -n "$UB_LIST_NETW_ALL" ] ; then
    for ifsubnet in $UB_LIST_NETW_ALL ; do
      case $ifsubnet in
        "${ifdashname}"@*)
          # Special GLA protection for local block; ULA protected default
          UB_LIST_NETW_LAN="$UB_LIST_NETW_LAN $ifsubnet"
          ;;
      esac
    done
  fi
}

##############################################################################

bundle_wan_networks() {
  local interface="$1"
  local ifsubnet ifname ifdashname

  network_get_device ifname "$interface"
  ifdashname="${ifname//./-}"


  if [ -n "$ifdashname" ] && [ -n "$UB_LIST_NETW_ALL" ] ; then
    for ifsubnet in $UB_LIST_NETW_ALL ; do
      case $UB_LIST_NETW_LAN in
        *"${ifsubnet}"*)
          # If LAN, then not WAN ... scripts might become complex
          ;;

        *)
          case $ifsubnet in
            "${ifdashname}"@*)
              # Special GLA protection for local block; ULA protected default
              UB_LIST_NETW_WAN="$UB_LIST_NETW_WAN $ifsubnet"
              ;;
          esac
          ;;
      esac
    done
  fi
}

##############################################################################

bundle_resolv_conf_servers() {
  local resolvers=$( awk '/nameserver/ { print $2 }' $UB_RESOLV_AUTO )
  UB_LIST_ZONE_SERVERS="$UB_LIST_ZONE_SERVERS $resolvers"
}

##############################################################################

bundle_zone_names() {
  UB_LIST_ZONE_NAMES="$UB_LIST_ZONE_NAMES $1"
}

##############################################################################

bundle_zone_servers() {
  UB_LIST_ZONE_SERVERS="$UB_LIST_ZONE_SERVERS $1"
}

##############################################################################

bundle_domain_insecure() {
  UB_LIST_INSECURE="$UB_LIST_INSECURE $1"
}

##############################################################################

unbound_mkdir() {
  local filestuff


  if [ "$UB_D_DHCP_LINK" = "odhcpd" ] ; then
    local dhcp_origin=$( uci_get dhcp.@odhcpd[0].leasefile )
    local dhcp_dir=$( dirname $dhcp_origin )


    if [ ! -d "$dhcp_dir" ] ; then
      # make sure odhcpd has a directory to write (not done itself, yet)
      mkdir -p "$dhcp_dir"
    fi
  fi


  if [ -f $UB_RKEY_FILE ] ; then
    filestuff=$( cat $UB_RKEY_FILE )


    case "$filestuff" in
      *"state=2 [  VALID  ]"*)
        # Lets not lose RFC 5011 tracking if we don't have to
        cp -p $UB_RKEY_FILE $UB_RKEY_FILE.keep
        ;;
    esac
  fi


  # Blind copy /etc/unbound to /var/lib/unbound
  mkdir -p $UB_VARDIR
  rm -f $UB_VARDIR/dhcp_*
  touch $UB_TOTAL_CONF
  cp -p $UB_ETCDIR/*.conf $UB_VARDIR/
  cp -p $UB_ETCDIR/root.* $UB_VARDIR/


  if [ ! -f $UB_RHINT_FILE ] ; then
    if [ -f /usr/share/dns/root.hints ] ; then
      # Debian-like package dns-root-data
      cp -p /usr/share/dns/root.hints $UB_RHINT_FILE

    elif [ $UB_B_READY -eq 0 ] ; then
      logger -t unbound -s "default root hints (built in root-servers.net)"
    fi
  fi


  if [ ! -f $UB_RKEY_FILE ] ; then
    if [ -f /usr/share/dns/root.key ] ; then
      # Debian-like package dns-root-data
      cp -p /usr/share/dns/root.key $UB_RKEY_FILE

    elif [ -x $UB_ANCHOR ] ; then
      $UB_ANCHOR -a $UB_RKEY_FILE

    elif [ $UB_B_READY -eq 0 ] ; then
      logger -t unbound -s "default trust anchor (built in root DS record)"
    fi
  fi


  if [ -f $UB_RKEY_FILE.keep ] ; then
    # root.key.keep is reused if newest
    cp -u $UB_RKEY_FILE.keep $UB_RKEY_FILE
    rm -f $UB_RKEY_FILE.keep
  fi


  # Ensure access and prepare to jail
  chown -R unbound:unbound $UB_VARDIR
  chmod 755 $UB_VARDIR
  chmod 644 $UB_VARDIR/*


  if [ -x /usr/sbin/unbound-control-setup ] ; then
    if [ ! -f $UB_CTLKEY_FILE ] || [ ! -f $UB_CTLPEM_FILE ] \
    || [ ! -f $UB_SRVKEY_FILE ] || [ ! -f $UB_SRVPEM_FILE ] ; then
      case "$UB_D_CONTROL" in
        [2-3])
          # unbound-control-setup for encrypt opt. 2 and 3, but not 4 "static"
          /usr/sbin/unbound-control-setup -d $UB_ETCDIR

          chown -R unbound:unbound  $UB_CTLKEY_FILE $UB_CTLPEM_FILE \
                                    $UB_SRVKEY_FILE $UB_SRVPEM_FILE

          chmod 640 $UB_CTLKEY_FILE $UB_CTLPEM_FILE \
                    $UB_SRVKEY_FILE $UB_SRVPEM_FILE
          ;;
      esac
    fi
  fi


  if [ -f "$UB_TIME_FILE" ] ; then
    # NTP is done so its like you actually had an RTC
    UB_B_READY=1
    UB_B_NTP_BOOT=0

  elif [ $UB_B_NTP_BOOT -eq 0 ] ; then
    # time is considered okay on this device (ignore /etc/hotplug/ntpd/unbound)
    date -Is > $UB_TIME_FILE
    UB_B_READY=0
    UB_B_NTP_BOOT=0

  else
    # DNSSEC-TIME will not reconcile
    UB_B_READY=0
    UB_B_NTP_BOOT=1
  fi
}

##############################################################################

unbound_control() {
  echo "# $UB_CTRL_CONF generated by UCI $( date -Is )" > $UB_CTRL_CONF


  if [ $UB_D_CONTROL -gt 1 ] ; then
    if [ ! -f $UB_CTLKEY_FILE ] || [ ! -f $UB_CTLPEM_FILE ] \
    || [ ! -f $UB_SRVKEY_FILE ] || [ ! -f $UB_SRVPEM_FILE ] ; then
      # Key files need to be present; if unbound-control-setup was found, then
      # they might have been made during unbound_makedir() above.
      UB_D_CONTROL=0
    fi
  fi


  case "$UB_D_CONTROL" in
    1)
      {
        # Local Host Only Unencrypted Remote Control
        echo "remote-control:"
        echo "  control-enable: yes"
        echo "  control-use-cert: no"
        echo "  control-interface: 127.0.0.1"
        echo "  control-interface: ::1"
        echo
      } >> $UB_CTRL_CONF
      ;;

    2)
      {
        # Local Host Only Encrypted Remote Control
        echo "remote-control:"
        echo "  control-enable: yes"
        echo "  control-use-cert: yes"
        echo "  control-interface: 127.0.0.1"
        echo "  control-interface: ::1"
        echo "  server-key-file: $UB_SRVKEY_FILE"
        echo "  server-cert-file: $UB_SRVPEM_FILE"
        echo "  control-key-file: $UB_CTLKEY_FILE"
        echo "  control-cert-file: $UB_CTLPEM_FILE"
        echo
      } >> $UB_CTRL_CONF
      ;;

    [3-4])
      {
        # Network Encrypted Remote Control
        # (3) may auto setup and (4) must have static key/pem files
        # TODO: add UCI list for interfaces to bind
        echo "remote-control:"
        echo "  control-enable: yes"
        echo "  control-use-cert: yes"
        echo "  control-interface: 0.0.0.0"
        echo "  control-interface: ::0"
        echo "  server-key-file: $UB_SRVKEY_FILE"
        echo "  server-cert-file: $UB_SRVPEM_FILE"
        echo "  control-key-file: $UB_CTLKEY_FILE"
        echo "  control-cert-file: $UB_CTLPEM_FILE"
        echo
      } >> $UB_CTRL_CONF
      ;;
  esac
}

##############################################################################

unbound_zone() {
  local cfg=$1
  local servers_ip=""
  local servers_host=""
  local zone_sym zone_name zone_type zone_enabled zone_file
  local tls_upstream fallback
  local server port tls_port tls_index tls_suffix url_dir dns_ast

  if [ ! -f "$UB_ZONE_CONF" ] ; then
    echo "# $UB_ZONE_CONF generated by UCI $( date -Is )" > $UB_ZONE_CONF
  fi


  config_get_bool zone_enabled  "$cfg" enabled 0


  if [ $zone_enabled -eq 1 ] ; then
    # these lists are built for each zone; empty to start
    UB_LIST_ZONE_NAMES=""
    UB_LIST_ZONE_SERVERS=""

    config_get  zone_type "$cfg" zone_type ""
    config_get  port      "$cfg" port ""
    config_get  tls_index "$cfg" tls_index ""
    config_get  tls_port  "$cfg" tls_port 853
    config_get  url_dir   "$cfg" url_dir ""
    config_get  dns_ast   "$cfg" dns_assist none

    config_get_bool resolv_conf   "$cfg" resolv_conf 0
    config_get_bool fallback      "$cfg" fallback 1
    config_get_bool tls_upstream  "$cfg" tls_upstream 0

    config_list_foreach "$cfg" zone_name  bundle_zone_names
    config_list_foreach "$cfg" server     bundle_zone_servers

    # string formating for Unbound syntax
    tls_suffix="${tls_port:+@${tls_port}${tls_index:+#${tls_index}}}"
    [ $fallback -eq 0 ]     && fallback=no     || fallback=yes
    [ $tls_upstream -eq 0 ] && tls_upstream=no || tls_upstream=yes


    if [ $resolv_conf -eq 1 ] ; then
      bundle_resolv_conf_servers
    fi

  else
    zone_type=skip
  fi


  case "$dns_ast" in
  bind)
    if [ -x /usr/sbin/bind ] && [ -x /etc/init.d/bind ] ; then
      if /etc/init.d/bind enabled ; then
        dns_ast=1
      else
        dns_ast=0
      fi
    else
      dns_ast=0
    fi
    ;;

  dnsmasq)
    if [ -x /usr/sbin/dnsmasq ] && [ -x /etc/init.d/dnsmasq ] ; then
      if /etc/init.d/dnsmasq enabled ; then
        dns_ast=1
      else
        dns_ast=0
      fi
    else
      dns_ast=0
    fi
    ;;

  htpps-dns-proxy | https-dns-proxy)
    if [ -x /usr/sbin/https-dns-proxy ] \
    && [ -x /etc/init.d/https-dns-proxy ] ; then
      if /etc/init.d/https-dns-proxy ; then
        dns_ast=1
      else
        dns_ast=0
      fi
    else
      dns_ast=0
    fi
    ;;

  ipset-dns)
    if [ -x /usr/sbin/ipset-dns ] && [ -x /etc/init.d/ipset-dns ] ; then
      if /etc/init.d/ipset-dns enabled ; then
        dns_ast=1
      else
        dns_ast=0
      fi
    else
      dns_ast=0
    fi
    ;;

  nsd)
    if [ -x /usr/sbin/nsd ] && [ -x /etc/init.d/nsd ] ; then
      if /etc/init.d/nsd enabled ; then
        dns_ast=1
      else
        dns_ast=0
      fi
    else
      dns_ast=0
    fi
    ;;

  unprotected-loop)
    # Soft brick risk. The server you are looking to connect to may be offline
    # and cause loop error: procd, sysupgrade, package order, and other issues.
    dns_ast=1
    ;;

  *)
    # Unbound has a local forward blocking option, default on, instead of loop
    # detection. If it is released, then it may be a soft brick risk.
    dns_ast=0
    ;;
  esac


  if [ $dns_ast -gt 0 ] ; then
    UB_B_DNS_ASSIST=1
  fi


  case $zone_type in
    auth_zone)
      if [ $UB_B_NTP_BOOT -eq 0 ] && [ -n "$UB_LIST_ZONE_NAMES" ] \
      && { [ -n "$url_dir" ] || [ -n "$UB_LIST_ZONE_SERVERS" ] ; } ; then
        # Note AXFR may have large downloads. If NTP restart is configured,
        # then this can cause procd to force a process kill.
        for zone_name in $UB_LIST_ZONE_NAMES ; do
          if [ "$zone_name" = "." ] ; then
            zone_sym=.
            zone_name=root
            zone_file=root.zone
          else
            zone_sym=$zone_name
            zone_file=$zone_name.zone
            zone_file=${zone_file//../.}
          fi


          {
            # generate an auth-zone: with switches for prefetch cache
            echo "auth-zone:"
            echo "  name: $zone_sym"
            for server in $UB_LIST_ZONE_SERVERS ; do
              echo "  master: $server${port:+@${port}}"
            done
            if [ -n "$url_dir" ] ; then
              echo "  url: $url_dir$zone_file"
            fi
            echo "  fallback-enabled: $fallback"
            echo "  for-downstream: no"
            echo "  for-upstream: yes"
            echo "  zonefile: $zone_file"
            echo
          } >> $UB_ZONE_CONF
        done
      fi
      ;;

    forward_zone)
      if [ ! -f $UB_TLS_ETC_FILE ] && [ "$tls_upstream" = "yes" ] ; then
        logger -p 4 -t unbound -s \
          "Forward-zone TLS benefits from authentication in package 'ca-bundle'"
      fi


      if [ -n "$UB_LIST_ZONE_NAMES" ] && [ -n "$UB_LIST_ZONE_SERVERS" ] ; then
        for server in $UB_LIST_ZONE_SERVERS ; do
          if [ "$( valid_subnet_any $server )" = "ok" ] \
          || { [ "$( local_subnet $server )" = "ok" ] \
            && [ $dns_ast -gt 0 ] ; } ; then
            case $server in
              *@[0-9]*|*#[A-Za-z0-9]*)
                # unique Unbound option for server address
                servers_ip="$servers_ip $server"
                ;;

              *)
                if [ "$tls_upstream" = "yes" ] ; then
                  servers_ip="$servers_ip $server$tls_suffix"
                else
                  servers_ip="$servers_ip $server${port:+@${port}}"
                fi
                ;;
            esac

          else
            case $server in
              127.*|::0*)
                # soft brick loop back risk see DNS assist above
                echo "do nothing" >/dev/null
                ;;

              *@[0-9]*|*#[A-Za-z0-9]*)
                # unique Unbound option for server host name
                servers_host="$servers_host $server"
                ;;

              *)
                if [ "$tls_upstream" = "yes" ] ; then
                  servers_host="$servers_host $server${tls_port:+@${tls_port}}"
                else
                  servers_host="$servers_host $server${port:+@${port}}"
                fi
                ;;
            esac
          fi
        done


        for zonename in $UB_LIST_ZONE_NAMES ; do
          {
            # generate a forward-zone with or without tls
            echo "forward-zone:"
            echo "  name: $zonename"
            for server in $servers_host ; do
              echo "  forward-host: $server"
            done
            for server in $servers_ip ; do
              echo "  forward-addr: $server"
            done
            echo "  forward-first: $fallback"
            echo "  forward-tls-upstream: $tls_upstream"
            echo
          } >> $UB_ZONE_CONF
        done
      fi
      ;;

    stub_zone)
      if [ -n "$UB_LIST_ZONE_NAMES" ] && [ -n "$UB_LIST_ZONE_SERVERS" ] ; then
        for zonename in $UB_LIST_ZONE_NAMES ; do
          {
            # generate a stub-zone: or ensure short cut to authority NS
            echo "stub-zone:"
            echo "  name: $zonename"
            for server in $UB_LIST_ZONE_SERVERS ; do
              echo "  stub-addr: $server${port:+@${port}}"
            done
            echo "  stub-first: $fallback"
            echo
          } >> $UB_ZONE_CONF
        done
      fi
      ;;

    *)
      {
        echo " # Special zone $zonename was not enabled or had UCI conflicts."
        echo
      } >> $UB_ZONE_CONF
      ;;
  esac
}

##############################################################################

unbound_conf() {
  local rt_mem rt_conn rt_buff modulestring domain ifsubnet moduleopts

  {
    # server: for this whole function
    echo "# $UB_CORE_CONF generated by UCI $( date -Is )"
    echo "server:"
    echo "  username: unbound"
    echo "  chroot: $UB_VARDIR"
    echo "  directory: $UB_VARDIR"
    echo "  pidfile: $UB_PIDFILE"
  } > $UB_CORE_CONF


  if [ -f "$UB_TLS_ETC_FILE" ] ; then
    # TLS cert bundle for upstream forwarder and https zone files
    # This is loaded before drop to root, so pull from /etc/ssl
    echo "  tls-cert-bundle: $UB_TLS_ETC_FILE" >> $UB_CORE_CONF
  fi


  if [ -f "$UB_RHINT_FILE" ] ; then
    # Optional hints if found
    echo "  root-hints: $UB_RHINT_FILE" >> $UB_CORE_CONF
  fi


  if [ $UB_B_DNSSEC -gt 0 ] && [ -f "$UB_RKEY_FILE" ] ; then
    {
      echo "  auto-trust-anchor-file: $UB_RKEY_FILE"
      echo
    } >> $UB_CORE_CONF

  else
    echo >> $UB_CORE_CONF
  fi


  if [ $UB_N_THREADS -gt 1 ] \
  && $PROG -V | grep -q "Linked libs:.*libevent" ; then
    # heavy variant using "threads" may need substantial resources
    echo "  num-threads: 2" >> $UB_CORE_CONF
  else
    # light variant with one "process" is much more efficient with light traffic
    echo "  num-threads: 1" >> $UB_CORE_CONF
  fi


  {
    # Limited threading (2) with one shared slab
    echo "  msg-cache-slabs: 1"
    echo "  rrset-cache-slabs: 1"
    echo "  infra-cache-slabs: 1"
    echo "  key-cache-slabs: 1"
    echo "  ratelimit-slabs: 1"
    echo "  ip-ratelimit-slabs: 1"
    echo
    # Logging
    echo "  use-syslog: yes"
    echo "  statistics-interval: 0"
    echo "  statistics-cumulative: no"
  } >> $UB_CORE_CONF


  if [ $UB_D_VERBOSE -ge 0 ] && [ $UB_D_VERBOSE -le 5 ] ; then
    echo "  verbosity: $UB_D_VERBOSE" >> $UB_CORE_CONF
  fi


  if [ $UB_B_EXT_STATS -gt 0 ] ; then
    {
      # store more data in memory for unbound-control to report
      echo "  extended-statistics: yes"
      echo
    } >> $UB_CORE_CONF

  else
    {
      # store Less
      echo "  extended-statistics: no"
      echo
    } >> $UB_CORE_CONF
  fi


  if [ $UB_B_IF_AUTO -gt 0 ] ; then
    echo "  interface-automatic: yes" >> $UB_CORE_CONF
  fi


  if [ $UB_B_DNS_ASSIST -gt 0 ] ; then
    echo "  do-not-query-localhost: no" >> $UB_CORE_CONF
  fi


  {
    # avoid interference with SPI/NAT on both reserved and common server ports
    echo "  edns-buffer-size: $UB_N_EDNS_SIZE"
    echo "  port: $UB_N_RX_PORT"
    echo "  outgoing-port-permit: 10240-65535"
  } >> $UB_CORE_CONF


  case "$UB_D_PROTOCOL" in
    ip4_only)
      {
        echo "  do-ip4: yes"
        echo "  do-ip6: no"
        echo
      } >> $UB_CORE_CONF
      ;;

    ip6_only)
      {
        echo "  do-ip4: no"
        echo "  do-ip6: yes"
        echo
      } >> $UB_CORE_CONF
      ;;

    ip6_local)
      {
        # answer your local IPv6 network but avoid broken ISP IPv6
        echo "  do-ip4: yes"
        echo "  do-ip6: yes"
        echo "  prefer-ip4: yes"
        echo "  prefer-ip6: no"
        echo
      } >> $UB_CORE_CONF
      ;;

    ip6_prefer)
      {
        # RFC compliant dual stack
        echo "  do-ip4: yes"
        echo "  do-ip6: yes"
        echo "  prefer-ip4: no"
        echo "  prefer-ip6: yes"
        echo
      } >> $UB_CORE_CONF
      ;;

    mixed)
      {
        echo "  do-ip4: yes"
        echo "  do-ip6: yes"
        echo
      } >> $UB_CORE_CONF
      ;;

    *)
      if [ $UB_B_READY -eq 0 ] ; then
        logger -t unbound -s "default protocol configuration"
      fi
      ;;
  esac


  case "$UB_D_RESOURCE" in
    # Tiny - Unbound's recommended cheap hardware config
    tiny)   rt_mem=1  ; rt_conn=5  ; rt_buff=1 ;;
    # Small - Half RRCACHE and open ports
    small)  rt_mem=8  ; rt_conn=10 ; rt_buff=2 ;;
    # Medium - Nearly default but with some added balancintg
    medium) rt_mem=16 ; rt_conn=20 ; rt_buff=4 ;;
    # Large - Double medium
    large)  rt_mem=32 ; rt_conn=50 ; rt_buff=4 ;;
    # Whatever unbound does
    *) rt_mem=0 ; rt_conn=0 ;;
  esac


  if [ $rt_mem -gt 0 ] ; then
    {
      # Other harding and options for an embedded router
      echo "  harden-short-bufsize: yes"
      echo "  harden-large-queries: yes"
      echo "  harden-glue: yes"
      echo "  use-caps-for-id: no"
      echo
      # Set memory sizing parameters
      echo "  msg-buffer-size: $(($rt_buff*8192))"
      echo "  outgoing-range: $(($rt_conn*32))"
      echo "  num-queries-per-thread: $(($rt_conn*16))"
      echo "  outgoing-num-tcp: $(($rt_conn))"
      echo "  incoming-num-tcp: $(($rt_conn))"
      echo "  rrset-cache-size: $(($rt_mem*256))k"
      echo "  msg-cache-size: $(($rt_mem*128))k"
      echo "  stream-wait-size: $(($rt_mem*128))k"
      echo "  key-cache-size: $(($rt_mem*128))k"
      echo "  neg-cache-size: $(($rt_mem*32))k"
      echo "  ratelimit-size: $(($rt_mem*32))k"
      echo "  ip-ratelimit-size: $(($rt_mem*32))k"
      echo "  infra-cache-numhosts: $(($rt_mem*256))"
      echo
    } >> $UB_CORE_CONF

  elif [ $UB_B_READY -eq 0 ] ; then
    logger -t unbound -s "default memory configuration"
  fi


  # Assembly of module-config: options is tricky; order matters
  moduleopts="$( /usr/sbin/unbound -V )"
  modulestring="iterator"


  case $moduleopts in
  *with-python*)
    modulestring="python $modulestring"
    ;;
  esac


  if [ $UB_B_DNSSEC -gt 0 ] ; then
    if [ $UB_B_NTP_BOOT -gt 0 ] ; then
      # DNSSEC chicken and egg with getting NTP time
      echo "  val-override-date: -1" >> $UB_CORE_CONF
    fi


    {
      echo "  harden-dnssec-stripped: yes"
      echo "  val-clean-additional: yes"
      echo "  ignore-cd-flag: yes"
    } >> $UB_CORE_CONF


    modulestring="validator $modulestring"
  fi


  case $moduleopts in
  *enable-subnet*)
    modulestring="subnetcache $modulestring"
    ;;
  esac


  if [ $UB_B_DNS64 -gt 0 ] ; then
    echo "  dns64-prefix: $UB_IP_DNS64" >> $UB_CORE_CONF

    modulestring="dns64 $modulestring"
  fi


  {
    # Print final module string
    echo "  module-config: \"$modulestring\""
    echo
  }  >> $UB_CORE_CONF


  case "$UB_D_RECURSION" in
    passive)
      {
        # Some query privacy but "strict" will break some servers
        if [ $UB_B_QRY_MINST -gt 0 ] && [ "$UB_B_QUERY_MIN" -gt 0 ] ; then
          echo "  qname-minimisation: yes"
          echo "  qname-minimisation-strict: yes"
        elif [ $UB_B_QUERY_MIN -gt 0 ] ; then
          echo "  qname-minimisation: yes"
        else
          echo "  qname-minimisation: no"
        fi
        # Use DNSSEC to quickly understand NXDOMAIN ranges
        if [ $UB_B_DNSSEC -gt 0 ] ; then
          echo "  aggressive-nsec: yes"
          echo "  prefetch-key: no"
        fi
        # On demand fetching
        echo "  prefetch: no"
        echo "  target-fetch-policy: \"0 0 0 0 0\""
        echo
      } >> $UB_CORE_CONF
      ;;

    aggressive)
      {
        # Some query privacy but "strict" will break some servers
        if [ $UB_B_QRY_MINST -gt 0 ] && [ $UB_B_QUERY_MIN -gt 0 ] ; then
          echo "  qname-minimisation: yes"
          echo "  qname-minimisation-strict: yes"
        elif [ $UB_B_QUERY_MIN -gt 0 ] ; then
          echo "  qname-minimisation: yes"
        else
          echo "  qname-minimisation: no"
        fi
        # Use DNSSEC to quickly understand NXDOMAIN ranges
        if [ $UB_B_DNSSEC -gt 0 ] ; then
          echo "  aggressive-nsec: yes"
          echo "  prefetch-key: yes"
        fi
        # Prefetch what can be
        echo "  prefetch: yes"
        echo "  target-fetch-policy: \"3 2 1 0 0\""
        echo
      } >> $UB_CORE_CONF
      ;;

    *)
      if [ $UB_B_READY -eq 0 ] ; then
        logger -t unbound -s "default recursion configuration"
      fi
      ;;
  esac


  if [ 10 -lt $UB_N_RATE_LMT ] && [ $UB_N_RATE_LMT -lt 100000 ] ; then
    {
      # Protect the server from query floods which is helpful on weaker CPU
      # Per client rate limit is half the maximum to leave head room open
      echo "  ratelimit: $UB_N_RATE_LMT"
      echo "  ip-ratelimit: $(($UB_N_RATE_LMT/2))"
      echo
    } >> $UB_CORE_CONF
  fi


  {
    # Reload records more than 20 hours old
    # DNSSEC 5 minute bogus cool down before retry
    # Adaptive infrastructure info kept for 15 minutes
    echo "  cache-min-ttl: $UB_TTL_MIN"
    echo "  cache-max-ttl: 72000"
    echo "  cache-max-negative-ttl: $UB_NEG_TTL_MAX"
    echo "  val-bogus-ttl: 300"
    echo "  infra-host-ttl: 900"
    echo
  } >> $UB_CORE_CONF


  if [ $UB_B_HIDE_BIND -gt 0 ] ; then
    {
      # Block server id and version DNS TXT records
      echo "  hide-identity: yes"
      echo "  hide-version: yes"
      echo
    } >> $UB_CORE_CONF
  fi


  if [ $UB_D_PRIV_BLCK -gt 0 ] ; then
    {
      # Remove _upstream_ or global reponses with private addresses.
      # Unbounds own "local zone" and "forward zone" may still use these.
      # RFC1918, RFC3927, RFC4291, RFC6598, RFC6890
      echo "  private-address: 10.0.0.0/8"
      echo "  private-address: 100.64.0.0/10"
      echo "  private-address: 169.254.0.0/16"
      echo "  private-address: 172.16.0.0/12"
      echo "  private-address: 192.168.0.0/16"
      echo "  private-address: fc00::/7"
      echo "  private-address: fe80::/10"
      echo
    } >> $UB_CORE_CONF
  fi


  if [ -n "$UB_LIST_NETW_LAN" ] && [ $UB_D_PRIV_BLCK -gt 1 ] ; then
    {
      for ifsubnet in $UB_LIST_NETW_LAN ; do
        case $ifsubnet in
          *@[1-9][0-9a-f][0-9a-f][0-9a-f]:*:[0-9a-f]*)
            # Remove global DNS responses with your local network IP6 GLA
            echo "  private-address: ${ifsubnet#*@}"
            ;;
        esac
      done
      echo
    } >> $UB_CORE_CONF
  fi


  if [ $UB_B_LOCL_BLCK -gt 0 ] ; then
    {
      # Remove DNS reponses from upstream with loopback IP
      # Black hole DNS method for ad blocking, so consider...
      echo "  private-address: 127.0.0.0/8"
      echo "  private-address: ::1/128"
      echo
    } >> $UB_CORE_CONF
  fi


  if  [ -n "$UB_LIST_INSECURE" ] ; then
    {
      for domain in $UB_LIST_INSECURE ; do
        # Except and accept domains without (DNSSEC); work around broken domains
        echo "  domain-insecure: $domain"
      done
      echo
    } >> $UB_CORE_CONF
  fi


  if [ $UB_B_LOCL_SERV -gt 0 ] && [ -n "$UB_LIST_NETW_LAN" ] ; then
    {
      for ifsubnet in $UB_LIST_NETW_LAN ; do
        # Only respond to queries from subnets which have an interface.
        # Prevent DNS amplification attacks by not responding to the universe.
        echo "  access-control: ${ifsubnet#*@} allow"
      done
      echo "  access-control: 127.0.0.0/8 allow"
      echo "  access-control: ::1/128 allow"
      echo "  access-control: fe80::/10 allow"
      echo
    } >> $UB_CORE_CONF

  else
    {
      echo "  access-control: 0.0.0.0/0 allow"
      echo "  access-control: ::0/0 allow"
      echo
    } >> $UB_CORE_CONF
  fi
}

##############################################################################

unbound_hostname() {
  local ifsubnet ifarpa ifaddr ifname iffqdn
  local ulaprefix hostfqdn name names namerec ptrrec
  local zonetype=0

  echo "# $UB_HOST_CONF generated by UCI $( date -Is )" > $UB_HOST_CONF


  if [ "$UB_D_DHCP_LINK" = "dnsmasq" ] ; then
    {
      echo "# Local zone is handled by dnsmasq"
      echo
    } >> $UB_HOST_CONF

  elif [ -n "$UB_TXT_DOMAIN" ] \
    && { [ $UB_D_WAN_FQDN -gt 0 ] || [ $UB_D_LAN_FQDN -gt 0 ] ; } ; then
    case "$UB_D_DOMAIN_TYPE" in
      deny|inform_deny|refuse|static)
        {
          # type static means only this router has your domain
          echo "  domain-insecure: $UB_TXT_DOMAIN"
          echo "  private-domain: $UB_TXT_DOMAIN"
          echo "  local-zone: $UB_TXT_DOMAIN $UB_D_DOMAIN_TYPE"
          echo "  local-data: \"$UB_TXT_DOMAIN. $UB_XSOA\""
          echo "  local-data: \"$UB_TXT_DOMAIN. $UB_XNS\""
          echo "  local-data: '$UB_TXT_DOMAIN. $UB_XTXT'"
          echo
          if [ "$UB_TXT_DOMAIN" != "local" ] ; then
            # avoid involvement in RFC6762, unless it is the local zone name
            echo "  local-zone: local always_nxdomain"
            echo
          fi
        } >> $UB_HOST_CONF
        zonetype=2
        ;;

      inform|transparent|typetransparent)
        {
          # transparent will permit forward-zone: or stub-zone: clauses
          echo "  private-domain: $UB_TXT_DOMAIN"
          echo "  local-zone: $UB_TXT_DOMAIN $UB_D_DOMAIN_TYPE"
          echo
        } >> $UB_HOST_CONF
        zonetype=1
        ;;
    esac


    {
      # Hostname as TLD works, but not transparent through recursion (singular)
      echo "  domain-insecure: $UB_TXT_HOSTNAME"
      echo "  private-domain: $UB_TXT_HOSTNAME"
      echo "  local-zone: $UB_TXT_HOSTNAME static"
      echo "  local-data: \"$UB_TXT_HOSTNAME. $UB_XSOA\""
      echo "  local-data: \"$UB_TXT_HOSTNAME. $UB_XNS\""
      echo "  local-data: '$UB_TXT_HOSTNAME. $UB_XTXT'"
      echo
    } >> $UB_HOST_CONF


    if [ -n "$UB_LIST_NETW_WAN" ] ; then
      for ifsubnet in $UB_LIST_NETW_WAN ; do
        ifaddr=${ifsubnet#*@}
        ifaddr=${ifaddr%/*}
        ifarpa=$( host_ptr_any "$ifaddr" )


        if [ -n "$ifarpa" ] ; then
          if [ $UB_D_WAN_FQDN -gt 0 ] ; then
            {
              # Create a static zone for WAN host record only (singular)
              echo "  domain-insecure: $ifarpa"
              echo "  private-address: $ifaddr"
              echo "  local-zone: $ifarpa static"
              echo "  local-data: \"$ifarpa. $UB_XSOA\""
              echo "  local-data: \"$ifarpa. $UB_XNS\""
              echo "  local-data: '$ifarpa. $UB_MTXT'"
              echo
            } >> $UB_HOST_CONF

          elif [ $zonetype -gt 0 ] ; then
            {
              echo "  local-zone: $ifarpa transparent"
              echo
            } >> $UB_HOST_CONF
          fi
        fi
      done
    fi


    if  [ -n "$UB_LIST_NETW_LAN" ] ; then
      for ifsubnet in $UB_LIST_NETW_LAN ; do
        ifarpa=$( domain_ptr_any "${ifsubnet#*@}" )


        if [ -n "$ifarpa" ] ; then
          if [ $zonetype -eq 2 ] ; then
            {
              # Do NOT forward queries with your ip6.arpa or in-addr.arpa
              echo "  domain-insecure: $ifarpa"
              echo "  local-zone: $ifarpa static"
              echo "  local-data: \"$ifarpa. $UB_XSOA\""
              echo "  local-data: \"$ifarpa. $UB_XNS\""
              echo "  local-data: '$ifarpa. $UB_XTXT'"
              echo
            } >> $UB_HOST_CONF

          elif [ $zonetype -eq 1 ] && [ $UB_D_PRIV_BLCK -eq 0 ] ; then
            {
              echo "  local-zone: $ifarpa transparent"
              echo
            } >> $UB_HOST_CONF
          fi
        fi
      done
    fi


    ulaprefix=$( uci_get network.@globals[0].ula_prefix )
    ulaprefix=${ulaprefix%%:/*}
    hostfqdn="$UB_TXT_HOSTNAME.$UB_TXT_DOMAIN"


    if [ -z "$ulaprefix" ] ; then
      # Nonsense so this option isn't globbed below
      ulaprefix="fdno:such:addr::"
    fi


    if [ "$UB_LIST_NETW_LAN" ] && [ $UB_D_LAN_FQDN -gt 0 ] ; then
      for ifsubnet in $UB_LIST_NETW_LAN ; do
        ifaddr=${ifsubnet#*@}
        ifaddr=${ifaddr%/*}
        ifname=${ifsubnet%@*}
        iffqdn="$ifname.$hostfqdn"


        if [ $UB_D_LAN_FQDN -eq 4 ] ; then
          names="$iffqdn $hostfqdn $UB_TXT_HOSTNAME"
          ptrrec="  local-data-ptr: \"$ifaddr 300 $iffqdn\""
          echo "$ptrrec" >> $UB_HOST_CONF

        elif [ $UB_D_LAN_FQDN -eq 3 ] ; then
          names="$hostfqdn $UB_TXT_HOSTNAME"
          ptrrec="  local-data-ptr: \"$ifaddr 300 $hostfqdn\""
          echo "$ptrrec" >> $UB_HOST_CONF

        else
          names="$UB_TXT_HOSTNAME"
          ptrrec="  local-data-ptr: \"$ifaddr 300 $UB_TXT_HOSTNAME\""
          echo "$ptrrec" >> $UB_HOST_CONF
        fi


        for name in $names ; do
          case $ifaddr in
            "${ulaprefix}"*)
              # IP6 ULA only is assigned for OPTION 1
              namerec="  local-data: \"$name. 300 IN AAAA $ifaddr\""
              echo "$namerec" >> $UB_HOST_CONF
              ;;

            [1-9]*.*[0-9])
              namerec="  local-data: \"$name. 300 IN A $ifaddr\""
              echo "$namerec" >> $UB_HOST_CONF
              ;;

            *)
              if [ $UB_D_LAN_FQDN -gt 1 ] ; then
                # IP6 GLA is assigned for higher options
                namerec="  local-data: \"$name. 300 IN AAAA $ifaddr\""
                echo "$namerec" >> $UB_HOST_CONF
              fi
              ;;
          esac
        done
        echo >> $UB_HOST_CONF
      done
    fi


    if [ -n "$UB_LIST_NETW_WAN" ] && [ $UB_D_WAN_FQDN -gt 0 ] ; then
      for ifsubnet in $UB_LIST_NETW_WAN ; do
        ifaddr=${ifsubnet#*@}
        ifaddr=${ifaddr%/*}
        ifname=${ifsubnet%@*}
        iffqdn="$ifname.$hostfqdn"


        if [ $UB_D_WAN_FQDN -eq 4 ] ; then
          names="$iffqdn $hostfqdn $UB_TXT_HOSTNAME"
          ptrrec="  local-data-ptr: \"$ifaddr 300 $iffqdn\""
          echo "$ptrrec" >> $UB_HOST_CONF

        elif [ $UB_D_WAN_FQDN -eq 3 ] ; then
          names="$hostfqdn $UB_TXT_HOSTNAME"
          ptrrec="  local-data-ptr: \"$ifaddr 300 $hostfqdn\""
          echo "$ptrrec" >> $UB_HOST_CONF

        else
          names="$UB_TXT_HOSTNAME"
          ptrrec="  local-data-ptr: \"$ifaddr 300 $UB_TXT_HOSTNAME\""
          echo "$ptrrec" >> $UB_HOST_CONF
        fi


        for name in $names ; do
          case $ifaddr in
            "${ulaprefix}"*)
              # IP6 ULA only is assigned for OPTION 1
              namerec="  local-data: \"$name. 300 IN AAAA $ifaddr\""
              echo "$namerec" >> $UB_HOST_CONF
              ;;

            [1-9]*.*[0-9])
              namerec="  local-data: \"$name. 300 IN A $ifaddr\""
              echo "$namerec" >> $UB_HOST_CONF
              ;;

            *)
              if [ $UB_D_WAN_FQDN -gt 1 ] ; then
                # IP6 GLA is assigned for higher options
                namerec="  local-data: \"$name. 300 IN AAAA $ifaddr\""
                echo "$namerec" >> $UB_HOST_CONF
              fi
              ;;
          esac
        done
        echo >> $UB_HOST_CONF
      done
    fi
  fi # end if uci valid
}

##############################################################################

unbound_uci() {
  local cfg="$1"
  local hostnm

  hostnm=$( uci_get system.@system[0].hostname | awk '{print tolower($0)}' )
  UB_TXT_HOSTNAME=${hostnm:-thisrouter}

  config_get_bool UB_B_SLAAC6_MAC "$cfg" dhcp4_slaac6 0
  config_get_bool UB_B_DNS64      "$cfg" dns64 0
  config_get_bool UB_B_EXT_STATS  "$cfg" extended_stats 0
  config_get_bool UB_B_HIDE_BIND  "$cfg" hide_binddata 1
  config_get_bool UB_B_LOCL_SERV  "$cfg" localservice 1
  config_get_bool UB_B_MAN_CONF   "$cfg" manual_conf 0
  config_get_bool UB_B_QUERY_MIN  "$cfg" query_minimize 0
  config_get_bool UB_B_QRY_MINST  "$cfg" query_min_strict 0
  config_get_bool UB_B_AUTH_ROOT  "$cfg" prefetch_root 0
  config_get_bool UB_B_LOCL_BLCK  "$cfg" rebind_localhost 0
  config_get_bool UB_B_DNSSEC     "$cfg" validator 0
  config_get_bool UB_B_NTP_BOOT   "$cfg" validator_ntp 1
  config_get_bool UB_B_IF_AUTO    "$cfg" interface_auto 1

  config_get UB_IP_DNS64    "$cfg" dns64_prefix "64:ff9b::/96"

  config_get UB_N_EDNS_SIZE "$cfg" edns_size 1232
  config_get UB_N_RX_PORT   "$cfg" listen_port 53
  config_get UB_N_ROOT_AGE  "$cfg" root_age 9
  config_get UB_N_THREADS   "$cfg" num_threads 1
  config_get UB_N_RATE_LMT  "$cfg" rate_limit 0

  config_get UB_D_CONTROL     "$cfg" unbound_control 0
  config_get UB_D_DOMAIN_TYPE "$cfg" domain_type static
  config_get UB_D_DHCP_LINK   "$cfg" dhcp_link none
  config_get UB_D_EXTRA_DNS   "$cfg" add_extra_dns 0
  config_get UB_D_LAN_FQDN    "$cfg" add_local_fqdn 0
  config_get UB_D_PRIV_BLCK   "$cfg" rebind_protection 1
  config_get UB_D_PROTOCOL    "$cfg" protocol mixed
  config_get UB_D_RECURSION   "$cfg" recursion passive
  config_get UB_D_RESOURCE    "$cfg" resource small
  config_get UB_D_VERBOSE     "$cfg" verbosity 1
  config_get UB_D_WAN_FQDN    "$cfg" add_wan_fqdn 0

  config_get UB_TTL_MIN     "$cfg" ttl_min 120
  config_get UB_TXT_DOMAIN  "$cfg" domain lan
  config_get UB_NEG_TTL_MAX "$cfg" ttl_neg_max 1000

  config_list_foreach "$cfg" domain_insecure bundle_domain_insecure
  config_list_foreach "$cfg" iface_lan bundle_lan_networks
  config_list_foreach "$cfg" iface_wan bundle_wan_networks

  if [ "$UB_D_DHCP_LINK" = "none" ] ; then
    config_get_bool UB_B_DNSMASQ   "$cfg" dnsmasq_link_dns 0


    if [ $UB_B_DNSMASQ -gt 0 ] ; then
      UB_D_DHCP_LINK=dnsmasq


      if [ $UB_B_READY -eq 0 ] ; then
        logger -t unbound -s "Please use 'dhcp_link' selector instead"
      fi
    fi
  fi


  if [ "$UB_D_DHCP_LINK" = "dnsmasq" ] ; then
    if [ ! -x /usr/sbin/dnsmasq ] || [ ! -x /etc/init.d/dnsmasq ] ; then
      UB_D_DHCP_LINK=none
    else
      /etc/init.d/dnsmasq enabled || UB_D_DHCP_LINK=none
    fi


    if [ $UB_B_READY -eq 0 ] && [ "$UB_D_DHCP_LINK" = "none" ] ; then
      logger -t unbound -s "cannot forward to dnsmasq"
    fi
  fi


  if [ "$UB_D_DHCP_LINK" = "odhcpd" ] ; then
    if [ ! -x /usr/sbin/odhcpd ] || [ ! -x /etc/init.d/odhcpd ] ; then
      UB_D_DHCP_LINK=none
    else
      /etc/init.d/odhcpd enabled || UB_D_DHCP_LINK=none
    fi


    if [ $UB_B_READY -eq 0 ] && [ "$UB_D_DHCP_LINK" = "none" ] ; then
      logger -t unbound -s "cannot receive records from odhcpd"
    fi
  fi


  if [ $UB_N_EDNS_SIZE -lt 512 ] || [ 4096 -lt $UB_N_EDNS_SIZE ] ; then
    logger -t unbound -s "edns_size exceeds range, using default"
    UB_N_EDNS_SIZE=1232
  fi


  if [ $UB_N_RX_PORT -ne 53 ] \
  && { [ $UB_N_RX_PORT -lt 1024 ] || [ 10240 -lt $UB_N_RX_PORT ] ; } ; then
    logger -t unbound -s "privileged port or in 5 digits, using default"
    UB_N_RX_PORT=53
  fi


  if [ $UB_TTL_MIN -gt 1800 ] ; then
    logger -t unbound -s "ttl_min could have had awful side effects, using 300"
    UB_TTL_MIN=300
  fi
}

##############################################################################

unbound_include() {
  local adb_enabled
  local adb_files=$( ls $UB_VARDIR/adb_list.* 2>/dev/null )

  echo "# $UB_TOTAL_CONF generated by UCI $( date -Is )" > $UB_TOTAL_CONF


  if [ -f "$UB_CORE_CONF" ] ; then
    # Yes this all looks busy, but it is in TMPFS. Working on separate files
    # and piecing together is easier. UCI order is less constrained.
    cat $UB_CORE_CONF >> $UB_TOTAL_CONF
    rm  $UB_CORE_CONF
  fi


  if [ -f "$UB_HOST_CONF" ] ; then
    # UCI definitions of local host or local subnet
    cat $UB_HOST_CONF >> $UB_TOTAL_CONF
    rm  $UB_HOST_CONF
  fi


  if [ -f $UB_SRVMASQ_CONF ] ; then
    # UCI found link to dnsmasq
    cat $UB_SRVMASQ_CONF >> $UB_TOTAL_CONF
    rm  $UB_SRVMASQ_CONF
  fi


  if [ -f "$UB_DHCP_CONF" ] ; then
    {
      # Seed DHCP records because dhcp scripts trigger externally
      # Incremental Unbound restarts may drop unbound-control records
      echo "include: $UB_DHCP_CONF"
      echo
    } >> $UB_TOTAL_CONF
  fi


  if [ -z "$adb_files" ] || [  ! -x /usr/bin/adblock.sh ] \
  || [ ! -x /etc/init.d/adblock ] ; then
    adb_enabled=0

  elif /etc/init.d/adblock enabled ; then
    adb_enabled=1
    {
      # Pull in your selected openwrt/pacakges/net/adblock generated lists
      echo "include: $UB_VARDIR/adb_list.*"
      echo
    } >> $UB_TOTAL_CONF

  else
    adb_enabled=0
  fi


  if [ -f $UB_SRV_CONF ] ; then
    {
      # Pull your own "server:" options here
      echo "include: $UB_SRV_CONF"
      echo
    } >> $UB_TOTAL_CONF
  fi


  if [ -f "$UB_ZONE_CONF" ] ; then
    # UCI defined forward, stub, and auth zones
    cat $UB_ZONE_CONF >> $UB_TOTAL_CONF
    rm  $UB_ZONE_CONF
  fi


  if [ -f "$UB_CTRL_CONF" ] ; then
    # UCI defined control application connection
    cat $UB_CTRL_CONF >> $UB_TOTAL_CONF
    rm  $UB_CTRL_CONF
  fi


  if [ -f "$UB_EXTMASQ_CONF" ] ; then
    # UCI found link to dnsmasq
    cat $UB_EXTMASQ_CONF >> $UB_TOTAL_CONF
    rm  $UB_EXTMASQ_CONF
  fi


  if [ -f "$UB_EXT_CONF" ] ; then
    {
      # Pull your own extend feature clauses here
      echo "include: $UB_EXT_CONF"
      echo
    } >> $UB_TOTAL_CONF
  fi
}

##############################################################################

resolv_setup() {
  if [ "$UB_N_RX_PORT" != "53" ] ; then
    # unbound is not the default on target resolver
    echo "do nothing" >/dev/null

  elif [ -x /etc/init.d/dnsmasq ] \
    && /etc/init.d/dnsmasq enabled \
    && nslookup localhost 127.0.0.1#53 >/dev/null 2>&1 ; then
    # unbound is configured for port 53, but dnsmasq is enabled, and a resolver
    # is already listening on port 53. Let dnsmasq manage resolve.conf.
    # This also works to prevent clobbering while changing UCI.
    echo "do nothing" >/dev/null

  else
    # unbound listens on 127.0.0.1#53 so set resolver file to local.
    rm -f $UB_RESOLV_CONF

    {
      echo "# $UB_RESOLV_CONF generated by Unbound UCI $( date -Is )"
      echo "nameserver 127.0.0.1"
      echo "nameserver ::1"
      echo "search $UB_TXT_DOMAIN."
    } > $UB_RESOLV_CONF
  fi
}

##############################################################################

unbound_start() {
  # get interface subnets together
  config_load network
  config_foreach bundle_all_networks interface

  # read Unbound UCI but pick through it later
  config_load unbound
  config_foreach unbound_uci unbound
  unbound_mkdir


  if [ $UB_B_MAN_CONF -eq 0 ] ; then
    # iterate zones before we load other UCI
    # forward-zone: auth-zone: and stub-zone:
    config_foreach unbound_zone zone
    # associate potential DNS RR with interfaces
    config_load dhcp
    config_foreach bundle_dhcp_networks dhcp
    # server:
    unbound_conf
    unbound_hostname
    # control:
    unbound_control
    # dnsmasq
    dnsmasq_link
    # merge
    unbound_include
  fi


  resolv_setup
}

##############################################################################

