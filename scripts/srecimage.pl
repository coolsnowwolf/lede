#!/usr/bin/env perl
#
# srecimage.pl - script to convert a binary image into srec
# Copyright (c) 2015 - Jo-Philipp Wich <jo@mein.io>
#
# This script is in the public domain.

use strict;

my ($input, $output, $offset) = @ARGV;

if (!defined($input) || !-f $input || !defined($output) ||
    !defined($offset) || $offset !~ /^(0x)?[a-fA-F0-9]+$/) {
	die "Usage: $0 <input file> <output file> <load address>\n";
}

sub srec
{
	my ($type, $addr, $data, $len) = @_;
	my @addrtypes = qw(%04X %04X %06X %08X %08X %04X %06X %08X %06X %04X);
	my $addrstr = sprintf $addrtypes[$type], $addr;

	$len = length($data) if ($len <= 0);
	$len += 1 + (length($addrstr) / 2);

	my $sum = $len;

	foreach my $byte (unpack('C*', pack('H*', $addrstr)), unpack('C*', $data))
	{
		$sum += $byte;
	}

	return sprintf "S%d%02X%s%s%02X\r\n",
	       $type, $len, $addrstr, uc(unpack('H*', $data)), ~($sum & 0xFF) & 0xFF;
}


open(IN, '<:raw', $input) || die "Unable to open $input: $!\n";
open(OUT, '>:raw', $output) || die "Unable to open $output: $!\n";

my ($basename) = $output =~ m!([^/]+)$!;

print OUT srec(0, 0, $basename, 0);

my $off = hex($offset);
my $len;

while (defined($len = read(IN, my $buf, 16)) && $len > 0)
{
	print OUT srec(3, $off, $buf, $len);
	$off += $len;
}

print OUT srec(7, hex($offset), "", 0);

close OUT;
close IN;
