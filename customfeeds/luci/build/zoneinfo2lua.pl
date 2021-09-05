#!/usr/bin/perl
# zoneinfo2lua.pl - Make Lua module from /usr/share/zoneinfo
# Execute from within root of Luci feed, usually feeds/luci
# $Id$

use strict;

my %TZ;

my $tzdin  = $ARGV[0] || "/usr/share/zoneinfo";
my $tzdout = $ARGV[1] || "./modules/luci-base/luasrc/sys/zoneinfo";

local $/ = "\012";
open( ZTAB, "< $tzdin/zone.tab" ) || die "open($tzdin/zone.tab): $!";

while( ! eof ZTAB ) {
	chomp( my $line = readline ZTAB );
	next if $line =~ /^#/ || $line =~ /^\s+$/;

	my ( undef, undef, $zone, @comment ) = split /\s+/, $line;

	printf STDERR "%-40s", $zone;

	if( open ZONE, "< $tzdin/$zone" ) {
		seek ZONE, -2, 2;

		while( tell(ZONE) > 0 ) {
			read ZONE, my $char, 1;
			( $char eq "\012" ) ? last : seek ZONE, -2, 1;
		}

		chomp( my $tz = readline ZONE );
		print STDERR ( $tz || "(no tzinfo found)" ), "\n";
		close ZONE;

		if( $tz ) {
			$zone =~ s/_/ /g;
			$TZ{$zone} = $tz;
		}
	}
	else
	{
		print STDERR "open($tzdin/$zone): $!\n";
	}
}

close ZTAB;

# Add Etc/GMT zones from manually as they are not in zone.tab
$TZ{"Etc/GMT"} = "GMT0";
$TZ{"Etc/GMT-1"} = "<+01>-1";
$TZ{"Etc/GMT-2"} = "<+02>-2";
$TZ{"Etc/GMT-3"} = "<+03>-3";
$TZ{"Etc/GMT-4"} = "<+04>-4";
$TZ{"Etc/GMT-5"} = "<+05>-5";
$TZ{"Etc/GMT-6"} = "<+06>-6";
$TZ{"Etc/GMT-7"} = "<+07>-7";
$TZ{"Etc/GMT-8"} = "<+08>-8";
$TZ{"Etc/GMT-9"} = "<+09>-9";
$TZ{"Etc/GMT-10"} = "<+10>-10";
$TZ{"Etc/GMT-11"} = "<+11>-11";
$TZ{"Etc/GMT-12"} = "<+12>-12";
$TZ{"Etc/GMT-13"} = "<+13>-13";
$TZ{"Etc/GMT-14"} = "<+14>-14";
$TZ{"Etc/GMT+1"} = "<-01>1";
$TZ{"Etc/GMT+2"} = "<-02>2";
$TZ{"Etc/GMT+3"} = "<-03>3";
$TZ{"Etc/GMT+4"} = "<-04>4";
$TZ{"Etc/GMT+5"} = "<-05>5";
$TZ{"Etc/GMT+6"} = "<-06>6";
$TZ{"Etc/GMT+7"} = "<-07>7";
$TZ{"Etc/GMT+8"} = "<-08>8";
$TZ{"Etc/GMT+9"} = "<-09>9";
$TZ{"Etc/GMT+10"} = "<-10>10";
$TZ{"Etc/GMT+11"} = "<-11>11";
$TZ{"Etc/GMT+12"} = "<-12>12";

open(O, "> $tzdout/tzdata.lua") || die "open($tzdout/tzdata.lua): $!\n";

print STDERR "Writing time zones to $tzdout/tzdata.lua ... ";
print O <<HEAD;
-- Licensed to the public under the Apache License 2.0.

module "luci.sys.zoneinfo.tzdata"

TZ = {
HEAD

foreach my $zone ( sort keys %TZ ) {
	printf O "\t{ '%s', '%s' },\n", $zone, $TZ{$zone}
}

print O "}\n";
close O;

print STDERR "done\n";


open (O, "> $tzdout/tzoffset.lua") || die "open($tzdout/tzoffset.lua): $!\n";

print STDERR "Writing time offsets to $tzdout/tzoffset.lua ... ";
print O <<HEAD;
-- Licensed to the public under the Apache License 2.0.

module "luci.sys.zoneinfo.tzoffset"

OFFSET = {
HEAD

my %seen;
foreach my $tz ( sort keys %TZ ) {
	my $zone = $TZ{$tz};

	if( $zone =~ /^
		([A-Z]+)
		(?:
			( -? \d+ (?: : \d+ )? )
			(?:
				([A-Z]+)
				( -? \d+ (?: : \d+ )? )? 
			)?
		)?
	\b /xo ) {
		my ( $offset, $s, $h, $m ) = ( 0, 1, 0, 0 );
		my ( $std, $soffset, $dst, $doffset ) = ( $1, $2, $3, $4 );

		next if $seen{$std}; # and ( !$dst or $seen{$dst} );

		if ( $soffset ) {
			( $s, $h, $m ) = $soffset =~ /^(-)?(\d+)(?::(\d+))?$/;

			$s = $s ? 1 : -1;
			$h ||= 0;
			$m ||= 0;

			$offset  = $s * $h * 60 * 60;
			$offset += $s * $m * 60;

			printf O "\t%-5s = %6d,\t-- %s\n",
				lc($std), $offset, $std;

			$seen{$std} = 1;

			if( $dst ) {
				if( $doffset ) {
					( $s, $h, $m ) = $doffset =~ /^(-)?(\d+)(?::(\d+))?$/;

					$s = $s ? 1 : -1;
					$h ||= 0;
					$m ||= 0;

					$offset  = $s * $h * 60 * 60;
					$offset += $s * $m * 60;
				} else  {
					$offset += 60 * 60;
				}

				printf O "\t%-5s = %6d,\t-- %s\n",
					lc($dst), $offset, $dst;
	
				$seen{$dst} = 1;
			}
		}
		else {
			printf O "\t%-5s = %6d,\t-- %s\n",
				lc($std), $offset, $std;

			$seen{$std} = 1;
		}

	}
}

print O "}\n";
close O;

print STDERR "done\n";
