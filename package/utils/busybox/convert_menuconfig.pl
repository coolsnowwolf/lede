#!/usr/bin/perl
# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

use strict;
my $PATH = $ARGV[0];
($PATH and -d $PATH) or die 'invalid path';

my %config;

open FIND, "find \"$PATH\" -name Config.in |";
while (<FIND>) {
	chomp;
	my $input = $_;
	my $output = $input;
	my $replace = quotemeta($PATH);
	$output =~ s/^$replace\///g;
	$output =~ s/sysdeps\/linux\///g;
	print STDERR "$input => $output\n";
	$output =~ /^(.+)\/[^\/]+$/ and system("mkdir -p $1");

	open INPUT, $input;
	open OUTPUT, ">$output";
	my ($cur, $default_set, $line);
	while ($line = <INPUT>) {
		next if $line =~ /^\s*mainmenu/;

		# FIXME: make this dynamic
		$line =~ s/default FEATURE_BUFFERS_USE_MALLOC/default FEATURE_BUFFERS_GO_ON_STACK/;
		$line =~ s/default FEATURE_SH_IS_NONE/default FEATURE_SH_IS_ASH/;

		if ($line =~ /^\s*config\s*([\w_]+)/) {
			$cur = $1;
			undef $default_set;
		}
		if ($line =~ /^\s*(menu|choice|end|source)/) {
			undef $cur;
			undef $default_set;
		}
		$line =~ s/^(\s*source\s+)([^\/]+\/)*([^\/]+\/[^\/]+)$/$1$3/;
		if ($line =~ /^(\s*range\s*)(\w+)(\s+)(\w+)\s*$/) {
			my $prefix = $1;
			my $r1 = $2;
			my $r2 = $4;
			$r1 =~ s/^([a-zA-Z]+)/BUSYBOX_CONFIG_$1/;
			$r2 =~ s/^([a-zA-Z]+)/BUSYBOX_CONFIG_$1/;
			$line = "$prefix$r1 $r2\n";
		}

		$line =~ s/^(\s*(prompt "[^"]+" if|config|depends|depends on|select|default|default \w if)\s+\!?)([A-Z_])/$1BUSYBOX_CONFIG_$3/g;
		$line =~ s/(( \|\| | \&\& | \( )!?)([A-Z_])/$1BUSYBOX_CONFIG_$3/g;
		$line =~ s/(\( ?!?)([A-Z_]+ (\|\||&&))/$1BUSYBOX_CONFIG_$2/g;

		if ($cur) {
			($cur eq 'LFS') and do {
				$line =~ s/^(\s*(bool|tristate|string))\s*".+"$/$1/;
			};
			if ($line =~ /^\s*default/) {
				my $c;
				$default_set = 1;
				$c = "BUSYBOX_DEFAULT_$cur";

				$line =~ s/^(\s*default\s*)(\w+|"[^"]*")(.*)/$1$c$3/;
			}
		}

		print OUTPUT $line;
	}
	close OUTPUT;
	close INPUT;
}
close FIND;
