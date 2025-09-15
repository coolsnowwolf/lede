#!/usr/bin/env perl
#
#    Copyright (C) 2009	Henk Vergonet <Henk.Vergonet@gmail.com>
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

# Description:
#   Replacement for brcmImagebuilder
#
# Disclaimer:
#   Use this software at your own risk.
#
# Changelog:
#   2009-01-01	Henk.Vergonet at gmail.com
#
use strict;
use Getopt::Std;
use Compress::Zlib;

my $version = "0.1";
my %arg = (
	o => 'bcm963xx_fs_kernel',
	b => 'OpenWrt',
	c => '6348',
	s => 64,
	f => 0xbfc00000,
	x => 0x00010000,
	a => 0x80010000,
	e => 0x80010000,
	i => 2,
);
my $prog = $0;
$prog =~ s/^.*\///;
getopts("r:k:o:lc:b:s:f:i:a:e:tpvh", \%arg);

die "usage: $prog ~opts~

  -r <file>	: input rootfs file
  -k <file>	: input kernel file
  -o <file>	: output image file, default $arg{o}
  -l		: littleendian system, default ".($arg{l} ? 'yes' : 'no')."
  -c <chipid>	: default $arg{c} 
  -b <boardid>	: default $arg{b} 
  -s <size_kb>	: erase sise flash, default $arg{s} 
  -f <baseaddr>	: flash base, default ".sprintf('0x%x', $arg{f})."
  -x <cfelen>	: length of cfe, default ".sprintf('0x%x', $arg{x})."
  -i		: 2=dual image, default $arg{i}

  -a <loadaddr>	: Kernel load address, default ".sprintf('0x%x', $arg{a})."
  -e <entryaddr>: Kernel entry address, default ".sprintf('0x%x', $arg{e})."
  -t		: Prefix kernel with load,entry,size

  -p		: Add a 'gOtO' partition 

  -v		: be more verbose
  -h		: help, version $version

EXAMPLES:
    $prog -k kern -r rootfs
" if $arg{h} || !$arg{k} || !$arg{r};

sub Read_Image
{
	open my $fh, $_[0] or die "open $_[0]: $!";
	local $/;	# Set input to "slurp" mode.
	my $buf = <$fh>;
	close $fh;
	return $buf;
}

sub Padlen
{
	my $p = $_[0] % $_[1];
	return ($p ? $_[1] - $p : 0);
}

sub Pad
{
	my ($buf, $off, $bs) = @_[0..2];
	$buf .= chr(255) x Padlen(length($buf) + $off, $bs);
	return $buf;
}

sub bcmImage
{
	my ($k, $f) = @_[0..1];
	my $tmp = $arg{x} + 0x100 + $arg{f};
	
	# regular: rootfs+kernel
	my ($img, $fa, $ka) = ( $f.$k, $tmp, $tmp + length($f) );

	# test: kernel+rootfs
#	my ($img, $fa, $ka) = ( $k.$f, $tmp + length($k), $tmp );

	$fa = 0 unless length($f);

	my $hdr = pack("a4a20a14a6a16a2a10a12a10a12a10a12a10a2a2a74Na16",
		'6',
		'LinuxInside', 
		'ver. 2.0', 
		$arg{c},
		$arg{b},
		($arg{l} ? '0' : '1'),
		length($img),
		'0',
		'0',
		$fa,
		length($f),
		$ka,
		length($k),
		($arg{i}==2 ? '1' : '0'),
		'',		# if 1, the image is INACTIVE; if 0, active
		'',
		~crc32($k, crc32($f)),
		'');
	$hdr .= pack('Na16', ~crc32($hdr), '');

	printf "kernel at 0x%x length 0x%x(%u)\n", $ka, length($k), length($k)
		if $arg{v};
	printf "rootfs at 0x%x length 0x%x(%u)\n", $fa, length($f), length($f)
		if $arg{v};

	open(FO, ">$arg{o}");
	print FO $hdr;
	print FO $img;
	close FO;
}

# MAIN

my $kern = Read_Image $arg{k};
my $root = Read_Image $arg{r};

$kern = pack('NNN', $arg{a}, $arg{e}, length($kern)).$kern if $arg{t};

# specific fixup for the CFE that expects rootfs-kernel order
if ($arg{p}) {
	$kern = Pad($kern, 0x10c, $arg{s} * 1024);
	my $dummy_root = pack('a4NN',
			'gOtO',
			length($kern)+12,
			length($root)+Padlen(length($root), $arg{s} * 1024)
	);
	$kern .= $root;
	$root = $dummy_root;
}

bcmImage($kern, $root);

