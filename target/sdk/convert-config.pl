#!/usr/bin/env perl
use strict;

while (<>) {
	my $match;
	my $var;
	my $val;
	my $type;
	chomp;
	next if /^CONFIG_SIGNED_PACKAGES/;

	if (/^CONFIG_((BINARY)|(DOWNLOAD))_FOLDER=(.*)$/) {
		# We don't want to preserve the build setting of
		# BINARY_FOLDER and DOWNLOAD_FOLDER.
		$var = "$1_FOLDER";
		$val = '""';
		$type = "string";
	} elsif (/^CONFIG_([^=]+)=(.*)$/) {
		$var = $1;
		$val = $2;

		next if $var eq 'ALL';

		if ($val eq 'y') {
			$type = "bool";
		} elsif ($val eq 'm') {
			$type = "tristate";
		} elsif ($val =~ /^".*"$/) {
			$type = "string";
		} elsif ($val =~ /^\d+$/) {
			$type = "int";
		} else {
			warn "WARNING: no type found for symbol CONFIG_$var=$val\n";
			next;
		}
	} elsif (/^# CONFIG_BUSYBOX_(.*) is not set/) {
		$var = "BUSYBOX_$1";
		$val = 'n';
		$type = "bool";
	} else {
		# We don't want to preserve a record of deselecting
		# packages because we may want build them in the SDK.
		# non-package configs however may be important to preserve
		# the same compilation settings for packages that get
		# recompiled in the SDK.
		# Also we want avoid preserving image generation settings
		# because we set those while in ImageBuilder
		next if /^(# )?CONFIG_PACKAGE/;
		next if /^(# )?CONFIG_TARGET/;
		if (/^# CONFIG_(.*) is not set/) {
			$var = $1;
			$val = 'n';
			$type = "bool";
                }
	}

	if (($var ne '') && ($type ne '') && ($val ne '')) {
		print <<EOF;
config $var
	$type
	default $val

EOF
	}
}
