#!/usr/bin/env perl
#
# xxdi.pl - perl implementation of 'xxd -i' mode
#
# Copyright 2013 Greg Kroah-Hartman <gregkh@linuxfoundation.org>
# Copyright 2013 Linux Foundation
#
# Released under the GPLv2.
#
# Implements the "basic" functionality of 'xxd -i' in perl to keep build
# systems from having to build/install/rely on vim-core, which not all
# distros want to do.  But everyone has perl, so use it instead.
#

use strict;
use warnings;

my $indata;

{
	local $/;
	my $fh;

	if (@ARGV) {
		open($fh, '<:raw', $ARGV[0]) || die("Unable to open $ARGV[0]: $!\n");
	} else {
		$fh = \*STDIN;
	}

	$indata = readline $fh;

	close $fh;
}

my $len_data = length($indata);
my $num_digits_per_line = 12;
my $var_name;
my $outdata;

# Use the variable name of the file we read from, converting '/' and '.
# to '_', or, if this is stdin, just use "stdin" as the name.
if (@ARGV) {
	$var_name = $ARGV[0];
	$var_name =~ s/\//_/g;
	$var_name =~ s/\./_/g;
} else {
	$var_name = "stdin";
}

$outdata .= "unsigned char $var_name\[] = {";

# trailing ',' is acceptable, so instead of duplicating the logic for
# just the last character, live with the extra ','.
for (my $key= 0; $key < $len_data; $key++) {
	if ($key % $num_digits_per_line == 0) {
		$outdata .= "\n\t";
	}
	$outdata .= sprintf("0x%.2x, ", ord(substr($indata, $key, 1)));
}

$outdata .= "\n};\nunsigned int $var_name\_len = $len_data;\n";

binmode STDOUT;
print {*STDOUT} $outdata;

