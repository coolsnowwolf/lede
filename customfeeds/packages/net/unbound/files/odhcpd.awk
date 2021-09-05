#!/usr/bin/awk
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
# Turn DHCP records into meaningful A, AAAA, and PTR records. Also lift a
# function from dnsmasq and use DHCPv4 MAC to find IPV6 SLAAC hosts.
#
# External Parameters
#   "conffile" = Unbound configuration left for a restart
#   "pipefile" = DNS entries for unbound-control standard input
#   "domain" = text domain suffix
#   "bslaac" = boolean, use DHCPv4 MAC to find GA and ULA IPV6 SLAAC
#   "bisolt" = boolean, format <host>.<network>.<domain>. so you can isolate
#   "bconf"  = boolean, write conf file with pipe records
#
##############################################################################

/^#/ {
  # We need to pick out DHCP v4 or v6 records
  net = $2 ; id = $3 ; cls = $4 ; hst = $5 ; adr = $9 ; adr2 = $10
  cdr = adr ;
  cdr2 = adr2 ;
  sub( /\/.*/, "", adr ) ;
  sub( /.*\//, "", cdr ) ;
  sub( /\/.*/, "", adr2 ) ;
  sub( /.*\//, "", cdr2 ) ;
  gsub( /_/, "-", hst ) ;


  if ( hst !~ /^[[:alnum:]]([-[:alnum:]]*[[:alnum:]])?$/ ) {
    # that is not a valid host name (RFC1123)
    # above replaced common error of "_" in host name with "-"
    hst = "-" ;
  }


  if ( bisolt == 1 ) {
    # TODO: this might be better with a substituion option,
    # or per DHCP pool do-not-DNS option, but its getting busy here.
    fqdn = net
    gsub( /\./, "-", fqdn ) ;
    fqdn = tolower( hst "." fqdn "." domain ) ;
  }

  else {
    fqdn = tolower( hst "." domain ) ;
  }


  if ((cls == "ipv4") && (hst != "-") && (cdr == 32) && (NF == 9)) {
    # IPV4 ; only for provided hostnames and full /32 assignments
    # NF=9 ; odhcpd errata in field format without host name
    ptr = adr ; qpr = "" ; split( ptr, ptr, "." ) ;
    slaac = slaac_eui64( id ) ;


    if ( bconf == 1 ) {
      x = ( "local-data: \"" fqdn ". 300 IN A " adr "\"" ) ;
      y = ( "local-data-ptr: \"" adr " 300 " fqdn "\"" ) ;
      print ( x "\n" y "\n" ) > conffile ;
    }


    # always create the pipe file
    for( i=1; i<=4; i++ ) { qpr = ( ptr[i] "." qpr) ; }
    x = ( fqdn ". 300 IN A " adr ) ;
    y = ( qpr "in-addr.arpa. 300 IN PTR " fqdn ) ;
    print ( x "\n" y ) > pipefile ;


    if (( bslaac == 1 ) && ( slaac != 0 )) {
      # UCI option to discover IPV6 routed SLAAC addresses
      # NOT TODO - ping probe take too long when added in awk-rule loop
      cmd = ( "ip -6 --oneline route show dev " net ) ;


      while ( ( cmd | getline adr ) > 0 ) {
        if (( substr( adr, 1, 5 ) <= "fdff:" ) \
        && ( index( adr, "::/" ) != 0 ) \
        && ( index( adr, "anycast" ) == 0 ) \
        && ( index( adr, "via" ) == 0 )) {
          # GA or ULA routed addresses only (not LL or MC)
          sub( /\/.*/, "", adr ) ;
          adr = ( adr slaac ) ;


          if ( split( adr, tmp0, ":" ) > 8 ) {
            sub( "::", ":", adr ) ;
          }


          if ( bconf == 1 ) {
            x = ( "local-data: \"" fqdn ". 300 IN AAAA " adr "\"" ) ;
            y = ( "local-data-ptr: \"" adr " 300 " fqdn "\"" ) ;
            print ( x "\n" y "\n" ) > conffile ;
          }


          # always create the pipe file
          qpr = ipv6_ptr( adr ) ;
          x = ( fqdn ". 300 IN AAAA " adr ) ;
          y = ( qpr ". 300 IN PTR " fqdn ) ;
          print ( x "\n" y ) > pipefile ;
        }
      }


      close( cmd ) ;
    }
  }

  else if ((cls != "ipv4") && (hst != "-") && (9 <= NF) && (NF <= 10)) {
    if (cdr == 128) {
      if ( bconf == 1 ) {
        x = ( "local-data: \"" fqdn ". 300 IN AAAA " adr "\"" ) ;
        y = ( "local-data-ptr: \"" adr " 300 " fqdn "\"" ) ;
        print ( x "\n" y "\n" ) > conffile ;
      }


      # only for provided hostnames and full /128 assignments
      qpr = ipv6_ptr( adr ) ;
      x = ( fqdn ". 300 IN AAAA " adr ) ;
      y = ( qpr ". 300 IN PTR " fqdn ) ;
      print ( x "\n" y ) > pipefile ;
    }

    if (cdr2 == 128) {
      if ( bconf == 1 ) {
        x = ( "local-data: \"" fqdn ". 300 IN AAAA " adr2 "\"" ) ;
        y = ( "local-data-ptr: \"" adr2 " 300 " fqdn "\"" ) ;
        print ( x "\n" y "\n" ) > conffile ;
      }


      # odhcp puts GA and ULA on the same line (position 9 and 10)
      qpr2 = ipv6_ptr( adr2 ) ;
      x = ( fqdn ". 300 IN AAAA " adr2 ) ;
      y = ( qpr2 ". 300 IN PTR " fqdn ) ;
      print ( x "\n" y ) > pipefile ;
    }
  }

  else {
    # dump non-conforming lease records
  }
}

##############################################################################

function ipv6_ptr( ipv6,    arpa, ary, end, i, j, new6, sz, start ) {
  # IPV6 colon flexibility is a challenge when creating [ptr].ip6.arpa.
  sz = split( ipv6, ary, ":" ) ; end = 9 - sz ;


  for( i=1; i<=sz; i++ ) {
    if( length(ary[i]) == 0 ) {
      for( j=1; j<=end; j++ ) { ary[i] = ( ary[i] "0000" ) ; }
    }

    else {
      ary[i] = substr( ( "0000" ary[i] ), length( ary[i] )+5-4 ) ;
    }
  }


  new6 = ary[1] ;
  for( i = 2; i <= sz; i++ ) { new6 = ( new6 ary[i] ) ; }
  start = length( new6 ) ;
  for( i=start; i>0; i-- ) { arpa = ( arpa substr( new6, i, 1 ) ) ; } ;
  gsub( /./, "&\.", arpa ) ; arpa = ( arpa "ip6.arpa" ) ;

  return arpa ;
}

##############################################################################

function slaac_eui64( mac,    ary, glbit, eui64 ) {
  if ( length(mac) >= 12 ) {
    # RFC2373 and use DHCPv4 registered MAC to find SLAAC addresses
    split( mac , ary , "" ) ;
    glbit = ( "0x" ary[2] ) ;
    glbit = sprintf( "%d", glbit ) ;
    glbit = or( glbit, 2 ) ;
    ary[2] = sprintf( "%x", glbit ) ;
    eui64 = ( ary[1] ary[2] ary[3] ary[4] ":" ary[5] ary[6] "ff:fe" ) ;
    eui64 = ( eui64 ary[7] ary[8] ":" ary[9] ary[10]  ary[11] ary[12] ) ;
  }

  else {
    eui64 = 0 ;
  }


  return eui64 ;
}

##############################################################################

