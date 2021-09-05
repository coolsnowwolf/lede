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
# This script facilitates alternate installation of Unbound+odhcpd and no
# need for dnsmasq. There are some limitations, but it works and is small.
# The lease file is parsed to make "zone-data:" and "local-data:" entries.
#
# config odhcpd 'odhcpd'
#   option leasetrigger '/usr/lib/unbound/odhcpd.sh'
#
##############################################################################

# while useful (sh)ellcheck is pedantic and noisy
# shellcheck disable=1091,2002,2004,2034,2039,2086,2094,2140,2154,2155

UB_ODHCPD_BLANK=

##############################################################################

odhcpd_zonedata() {
  . /lib/functions.sh
  . /usr/lib/unbound/defaults.sh

  local dhcp_link=$( uci_get unbound.@unbound[0].dhcp_link )
  local dhcp4_slaac6=$( uci_get unbound.@unbound[0].dhcp4_slaac6 )
  local dhcp_domain=$( uci_get unbound.@unbound[0].domain )
  local dhcp_origin=$( uci_get dhcp.@odhcpd[0].leasefile )


  if [ -f "$UB_TOTAL_CONF" ] && [ -f "$dhcp_origin" ] \
  && [ "$dhcp_link" = "odhcpd" ] && [ -n "$dhcp_domain" ] ; then
    local longconf dateconf dateoldf
    local dns_ls_add=$UB_VARDIR/dhcp_dns.add
    local dns_ls_del=$UB_VARDIR/dhcp_dns.del
    local dns_ls_new=$UB_VARDIR/dhcp_dns.new
    local dns_ls_old=$UB_VARDIR/dhcp_dns.old
    local dhcp_ls_new=$UB_VARDIR/dhcp_lease.new


    if [ ! -f $UB_DHCP_CONF ] || [ ! -f $dns_ls_old ] ; then
      # no old files laying around
      touch $dns_ls_old
      sort $dhcp_origin > $dhcp_ls_new
      longconf=freshstart

    else
      # incremental at high load or full refresh about each 5 minutes
      dateconf=$(( $( date +%s ) - $( date -r $UB_DHCP_CONF +%s ) ))
      dateoldf=$(( $( date +%s ) - $( date -r $dns_ls_old +%s ) ))


      if [ $dateconf -gt 300 ] ; then
        touch $dns_ls_old
        sort $dhcp_origin > $dhcp_ls_new
        longconf=longtime

      elif [ $dateoldf -gt 1 ] ; then
        touch $dns_ls_old
        sort $dhcp_origin > $dhcp_ls_new
        longconf=increment

      else
        # odhcpd is rapidly updating leases a race condition could occur
        longconf=skip
      fi
    fi


    case $longconf in
    freshstart)
      awk -v conffile=$UB_DHCP_CONF -v pipefile=$dns_ls_new \
          -v domain=$dhcp_domain -v bslaac=$dhcp4_slaac6 \
          -v bisolt=0 -v bconf=1 \
          -f /usr/lib/unbound/odhcpd.awk $dhcp_ls_new

      cp $dns_ls_new $dns_ls_add
      cp $dns_ls_new $dns_ls_old
      cat $dns_ls_add | $UB_CONTROL_CFG local_datas
      rm -f $dns_ls_new $dns_ls_del $dns_ls_add $dhcp_ls_new
      ;;

    longtime)
      awk -v conffile=$UB_DHCP_CONF -v pipefile=$dns_ls_new \
          -v domain=$dhcp_domain -v bslaac=$dhcp4_slaac6 \
          -v bisolt=0 -v bconf=1 \
          -f /usr/lib/unbound/odhcpd.awk $dhcp_ls_new

      awk '{ print $1 }' $dns_ls_old | sort | uniq > $dns_ls_del
      cp $dns_ls_new $dns_ls_add
      cp $dns_ls_new $dns_ls_old
      cat $dns_ls_del | $UB_CONTROL_CFG local_datas_remove
      cat $dns_ls_add | $UB_CONTROL_CFG local_datas
      rm -f $dns_ls_new $dns_ls_del $dns_ls_add $dhcp_ls_new
      ;;

    increment)
      # incremental add and prepare the old list for delete later
      # unbound-control can be slow so high DHCP rates cannot run a full list
      awk -v conffile=$UB_DHCP_CONF -v pipefile=$dns_ls_new \
          -v domain=$dhcp_domain -v bslaac=$dhcp4_slaac6 \
          -v bisolt=0 -v bconf=0 \
          -f /usr/lib/unbound/odhcpd.awk $dhcp_ls_new

      sort $dns_ls_new $dns_ls_old $dns_ls_old | uniq -u > $dns_ls_add
      sort $dns_ls_new $dns_ls_old | uniq > $dns_ls_old
      cat $dns_ls_add | $UB_CONTROL_CFG local_datas
      rm -f $dns_ls_new $dns_ls_del $dns_ls_add $dhcp_ls_new
      ;;

    *)
      echo "do nothing" >/dev/null
      ;;
    esac
  fi
}

##############################################################################

UB_ODHPCD_LOCK=/tmp/unbound_odhcpd.lock

if [ ! -f $UB_ODHPCD_LOCK ] ; then
  # imperfect but it should avoid collisions
  touch $UB_ODHPCD_LOCK
  odhcpd_zonedata
  rm -f $UB_ODHPCD_LOCK

else
  UB_ODHCPD_LOCK_AGE=$(( $( date +%s ) - $( date -r $UB_ODHPCD_LOCK +%s ) ))

  if [ $UB_ODHCPD_LOCK_AGE -gt 100 ] ; then
    # unlock because something likely broke but do not write this time through
    rm -f $UB_ODHPCD_LOCK
  fi
fi

##############################################################################

