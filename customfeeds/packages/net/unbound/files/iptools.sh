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
# These are iptools that might be useful in a larger package, if provided
# elsewhere for common use. One example that many may find useful is turning
# flexible IPV6 colon dividers into PTR. Otherwise these are incomplete and
# would need robustness improvements for more generic applications.
#
##############################################################################

# while useful (sh)ellcheck is pedantic and noisy
# shellcheck disable=1091,2002,2004,2034,2039,2086,2094,2140,2154,2155

UB_IPTOOLS_BLANK=

##############################################################################

domain_ptr_ip6() {
  # Get the nibble rounded /CIDR ...ip6.arpa.
  echo "$1" | awk -F: \
  'BEGIN { OFS = "" ; }
  { CIDR = $0 ;
  sub(/.*\//,"",CIDR) ;
  CIDR = (CIDR / 4) ;
  sub(/\/[0-9]+/,"",$0) ;
  ct_stop = 9 - NF ;
  for(i=1; i<=NF; i++) {
    if(length($i) == 0) {
      for(j=1; j<=ct_stop; j++) { $i = ($i "0000") ; } }
    else { $i = substr(("0000" $i), length($i)+5-4) ; } } ;
  y = $0 ;
  ct_start = length(y) - 32 + CIDR ;
  for(i=ct_start; i>0; i--) { x = (x substr(y,i,1)) ; } ;
  gsub(/./,"&\.",x) ;
  x = (x "ip6.arpa") ;
  print x }'
}

##############################################################################

host_ptr_ip6() {
  # Get complete host ...ip6.arpa.
  echo "$1" | awk -F: \
  'BEGIN { OFS = "" ; }
  { sub(/\/[0-9]+/,"",$0) ;
  ct_stop = 9 - NF ;
  for(i=1; i<=NF; i++) {
    if(length($i) == 0) {
      for(j=1; j<=ct_stop; j++) { $i = ($i "0000") ; } }
    else { $i = substr(("0000" $i), length($i)+5-4) ; } } ;
  y = $0 ;
  ct_start = length(y);
  for(i=ct_start; i>0; i--) { x = (x substr(y,i,1)) ; } ;
  sub(/[0-9]+\//,"",x) ;
  gsub(/./,"&\.",x) ;
  x = (x "ip6.arpa") ;
  print x }'
}

##############################################################################

domain_ptr_ip4() {
  # Get the byte rounded /CIDR ...in-addr.arpa.
  echo "$1" | awk \
  '{ CIDR = $0 ;
  sub(/.*\//,"",CIDR) ;
  CIDR = (CIDR / 8) ;
  dtxt = $0 ;
  sub(/\/.*/,"",dtxt) ;
  split(dtxt, dtxt, ".") ;
  for(i=1; i<=CIDR; i++) { x = (dtxt[i] "." x) ; }
  x = (x "in-addr.arpa") ;
  print x }'
}

##############################################################################

host_ptr_ip4() {
  # Get complete host ...in-addr.arpa.
  echo "$1" | awk -F. \
  '{ x = ( $4"."$3"."$2"."$1".in-addr.arpa" ) ;
  sub(/\/[0-9]+/,"",x) ;
  print x }'
}

##############################################################################

valid_subnet6() {
  case "$1" in
    # GA
    [1-9][0-9a-f][0-9a-f][0-9a-f]":"*) echo "ok" ;;
    # ULA
    f[cd][0-9a-f][0-9a-f]":"*) echo "ok" ;;
    # fe80::, ::1, and such
    *) echo "not" ;;
  esac
}

##############################################################################

valid_subnet4() {
  case "$1" in
    # Link, Local, and Such
    169"."254"."*) echo "not" ;;
    127"."*) echo "not" ;;
    0"."*) echo "not" ;;
    255"."*) echo "not" ;;
    # Other Normal
    25[0-4]"."[0-9]*) echo "ok" ;;
    2[0-4][0-9]"."[0-9]*) echo "ok" ;;
    1[0-9][0-9]"."[0-9]*) echo "ok" ;;
    [0-9][0-9]"."[0-9]*) echo "ok" ;;
    [0-9]"."[0-9]*) echo "ok" ;;
    # Not Right
    *) echo "not";;
  esac
}

##############################################################################

valid_subnet_any() {
  local subnet=$1
  local validip4=$( valid_subnet4 $subnet )
  local validip6=$( valid_subnet6 $subnet )


  if [ "$validip4" = "ok" ] || [ "$validip6" = "ok" ] ; then
    echo "ok"
  else
    echo "not"
  fi
}

##############################################################################

private_subnet() {
  case "$1" in
    10"."*) echo "ok" ;;
    172"."1[6-9]"."*) echo "ok" ;;
    172"."2[0-9]"."*) echo "ok" ;;
    172"."3[0-1]"."*) echo "ok" ;;
    192"."168"."*) echo "ok" ;;
    f[cd][0-9a-f][0-9a-f]":"*) echo "ok" ;;
    *) echo "not" ;;
  esac
}

##############################################################################

local_subnet() {
  # local subnet 2nd place is limited to one digit to improve the filter
  case "$1" in
    127"."[0-9]"."[0-9]*) echo "ok" ;;
    ::1) echo "ok" ;;
    *) echo "not" ;;
  esac
}

##############################################################################

domain_ptr_any() {
  local subnet=$1
  local arpa validip4 validip6

  validip4=$( valid_subnet4 $subnet )
  validip6=$( valid_subnet6 $subnet )


  if [ "$validip4" = "ok" ] ; then
    arpa=$( domain_ptr_ip4 "$subnet" )
  elif [ "$validip6" = "ok" ] ; then
    arpa=$( domain_ptr_ip6 "$subnet" )
  fi


  if [ -n "$arpa" ] ; then
    echo $arpa
  fi
}

##############################################################################

host_ptr_any() {
  local subnet=$1
  local arpa validip4 validip6

  validip4=$( valid_subnet4 $subnet )
  validip6=$( valid_subnet6 $subnet )


  if [ "$validip4" = "ok" ] ; then
    arpa=$( host_ptr_ip4 "$subnet" )
  elif [ "$validip6" = "ok" ] ; then
    arpa=$( host_ptr_ip6 "$subnet" )
  fi


  if [ -n "$arpa" ] ; then
    echo $arpa
  fi
}

##############################################################################

