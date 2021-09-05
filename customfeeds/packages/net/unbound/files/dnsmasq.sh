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
# This crosses over to the dnsmasq UCI file "dhcp" and parses it for fields
# that will allow Unbound to request local host DNS of dnsmasq. We need to look
# at the interfaces in "dhcp" and get their subnets. The Unbound conf syntax
# makes this a little difficult. First in "server:" we need to create private
# zones for the domain and PTR records. Then we need to create numerous
# "forward:" clauses to forward those zones to dnsmasq.
#
##############################################################################

# while useful (sh)ellcheck is pedantic and noisy
# shellcheck disable=1091,2002,2004,2034,2039,2086,2094,2140,2154,2155

DM_D_WAN_FQDN=0

DM_LIST_KNOWN_ZONES="invalid"
DM_LIST_TRN_ZONES=""
DM_LIST_LOCAL_DATA=""
DM_LIST_LOCAL_PTR=""
DM_LIST_FWD_PORTS=""
DM_LIST_FWD_ZONES=""

##############################################################################

create_local_zone() {
  local target="$1"
  local partial domain found

  case $DM_LIST_TRN_ZONES in
    *"${target}"*)
      found=1
      ;;

    *)
      case $target in
        [A-Za-z0-9]*.[A-Za-z0-9]*)
          found=0
          ;;

        *) # no dots
          found=1
          ;;
      esac
  esac


  if [ $found -eq 0 ] ; then
    # New Zone! Bundle local-zones: by first two name tiers "abcd.tld."
    partial=$( echo "$target" | awk -F. '{ j=NF ; i=j-1; print $i"."$j }' )
    DM_LIST_TRN_ZONES="$DM_LIST_TRN_ZONES $partial"
    DM_LIST_KNOWN_ZONES="$DM_LIST_KNOWN_ZONES $partial"
  fi
}

##############################################################################

create_host_record() {
  local cfg="$1"
  local ip name debug_ip

  # basefiles dhcp "domain" clause which means host A, AAAA, and PRT record
  config_get ip   "$cfg" ip
  config_get name "$cfg" name


  if [ -n "$name" ] && [ -n "$ip" ] ; then
    create_local_zone "$name"


    case $ip in
      fe[89ab][0-9a-f]:*|169.254.*)
        debug_ip="$ip@$name"
        ;;

      [1-9a-f]*:*[0-9a-f])
        DM_LIST_LOCAL_DATA="$DM_LIST_LOCAL_DATA $name.@@300@@IN@@AAAA@@$ip"
        DM_LIST_LOCAL_PTR="$DM_LIST_LOCAL_PTR $ip@@300@@$name"
        ;;

      [1-9]*.*[0-9])
        DM_LIST_LOCAL_DATA="$DM_LIST_LOCAL_DATA $name.@@300@@IN@@A@@$ip"
        DM_LIST_LOCAL_PTR="$DM_LIST_LOCAL_PTR $ip@@300@@$name"
        ;;
    esac
  fi
}

##############################################################################

create_mx_record() {
  local cfg="$1"
  local domain relay pref record

  # Insert a static MX record
  config_get domain "$cfg" domain
  config_get relay  "$cfg" relay
  config_get pref   "$cfg" pref 10


  if [ -n "$domain" ] && [ -n "$relay" ] ; then
    create_local_zone "$domain"
    record="$domain.@@300@@IN@@MX@@$pref@@$relay."
    DM_LIST_LOCAL_DATA="$DM_LIST_LOCAL_DATA $record"
  fi
}

##############################################################################

create_srv_record() {
  local cfg="$1"
  local srv target port class weight record

  # Insert a static SRV record such as SIP server
  config_get srv    "$cfg" srv
  config_get target "$cfg" target
  config_get port   "$cfg" port
  config_get class  "$cfg" class 10
  config_get weight "$cfg" weight 10


  if [ -n "$srv" ] && [ -n "$target" ] && [ -n "$port" ] ; then
    create_local_zone "$srv"
    record="$srv.@@300@@IN@@SRV@@$class@@$weight@@$port@@$target."
    DM_LIST_LOCAL_DATA="$DM_LIST_LOCAL_DATA $record"
  fi
}

##############################################################################

create_cname_record() {
  local cfg="$1"
  local cname target record

  # Insert static CNAME record
  config_get cname  "$cfg" cname
  config_get target "$cfg" target


  if [ -n "$cname" ] && [ -n "$target" ] ; then
    create_local_zone "$cname"
    record="$cname.@@300@@IN@@CNAME@@$target."
    DM_LIST_LOCAL_DATA="$DM_LIST_LOCAL_DATA $record"
  fi
}

##############################################################################

dnsmasq_local_zone() {
  local cfg="$1"
  local fwd_port fwd_domain wan_fqdn

  # dnsmasq domain and interface assignment settings will control config
  config_get fwd_domain "$cfg" domain
  config_get fwd_port "$cfg" port
  config_get wan_fqdn "$cfg" add_wan_fqdn


  if [ -n "$wan_fqdn" ] ; then
    DM_D_WAN_FQDN=$wan_fqdn
  fi


  if [ -n "$fwd_domain" ] && [ -n "$fwd_port" ] \
  && [ ! ${fwd_port:-53} -eq 53 ] ; then
    # dnsmasq localhost listening ports (possible multiple instances)
    DM_LIST_FWD_PORTS="$DM_LIST_FWD_PORTS $fwd_port"
    DM_LIST_FWD_ZONES="$DM_LIST_FWD_ZONES $fwd_domain"
  fi
}

##############################################################################

dnsmasq_local_arpa() {
  local ifarpa ifsubnet


  if [ -n "$UB_LIST_NETW_LAN" ] ; then
    for ifsubnet in $UB_LIST_NETW_LAN ; do
      ifarpa=$( domain_ptr_any "${ifsubnet#*@}" )
      DM_LIST_FWD_ZONES="$DM_LIST_FWD_ZONES $ifarpa"
    done
  fi


  if [ -n "$UB_LIST_NETW_WAN" ] && [ $DM_D_WAN_FQDN -gt 0 ] ; then
    for ifsubnet in $UB_LIST_NETW_WAN ; do
      ifarpa=$( domain_ptr_any "${ifsubnet#*@}" )
      DM_LIST_FWD_ZONES="$DM_LIST_FWD_ZONES $ifarpa"
    done
  fi
}

##############################################################################

dnsmasq_inactive() {
  local record


  if [ $UB_D_EXTRA_DNS -gt 0 ] ; then
    # Parasite from the uci.dhcp.domain clauses
    DM_LIST_KNOWN_ZONES="$DM_LIST_KNOWN_ZONES $UB_TXT_DOMAIN"
    config_load dhcp
    config_foreach create_host_record domain


    if [ $UB_D_EXTRA_DNS -gt 1 ] ; then
      config_foreach create_srv_record srvhost
      config_foreach create_mx_record mxhost
    fi


    if [ $UB_D_EXTRA_DNS -gt 2 ] ; then
      config_foreach create_cname_record cname
    fi


    {
      echo "# $UB_SRVMASQ_CONF generated by UCI $( date -Is )"
      if [ -n "$DM_LIST_TRN_ZONES" ] ; then
        for record in $DM_LIST_TRN_ZONES ; do
          echo "  local-zone: $record transparent"
        done
        echo
      fi
      if [ -n "$DM_LIST_LOCAL_DATA" ] ; then
        for record in $DM_LIST_LOCAL_DATA ; do
          echo "  local-data: \"${record//@@/ }\""
        done
        echo
      fi
      if [ -n "$DM_LIST_LOCAL_PTR" ] ; then
        for record in $DM_LIST_LOCAL_PTR ; do
          echo "  local-data-ptr: \"${record//@@/ }\""
        done
        echo
      fi
    } > $UB_SRVMASQ_CONF
  fi
}

##############################################################################

dnsmasq_active() {
  # Look at dnsmasq settings
  config_load dhcp
  # Zone for DHCP / SLAAC-PING DOMAIN
  config_foreach dnsmasq_local_zone dnsmasq
  # Zone for DHCP / SLAAC-PING ARPA
  dnsmasq_local_arpa


  if [ -n "$DM_LIST_FWD_PORTS" ] && [ -n "$DM_LIST_FWD_ZONES" ] ; then
    if [ $UB_B_DNS_ASSIST -lt 1 ] ; then
      {
        # Forward to dnsmasq on same host for DHCP lease hosts
        echo "# $UB_SRVMASQ_CONF generated by UCI $( date -Is )"
        echo "  do-not-query-localhost: no"
        echo
      } > $UB_SRVMASQ_CONF

    else
      echo > $UB_SRVMASQ_CONF
    fi

    echo "# $UB_EXTMASQ_CONF generated by UCI $( date -Is )" > $UB_EXTMASQ_CONF


    for fwd_domain in $DM_LIST_FWD_ZONES ; do
      {
        # This creates a domain with local privledges
        echo "  domain-insecure: $fwd_domain"
        echo "  private-domain: $fwd_domain"
        echo "  local-zone: $fwd_domain transparent"
        echo
      } >> $UB_SRVMASQ_CONF

      {
        # This is derived from dnsmasq local domain and dhcp service subnets
        echo "forward-zone:"
        echo "  name: $fwd_domain"
        echo "  forward-first: no"
        for port in $DM_LIST_FWD_PORTS ; do
          echo "  forward-addr: 127.0.0.1@$port"
        done
        echo
      } >> $UB_EXTMASQ_CONF
    done
  fi
}

##############################################################################

dnsmasq_link() {
  if [ "$UB_D_DHCP_LINK" = "dnsmasq" ] ; then
    dnsmasq_active

  else
    dnsmasq_inactive
  fi
}

##############################################################################

