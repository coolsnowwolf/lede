#!/usr/bin/env perl
#
# Script for generating redboot configs, based on brcmImage.pl
#
# Copyright (C) 2015 Álvaro Fernández Rojas <noltari@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

use strict;
use Getopt::Std;
use File::stat;

my $version = "0.1";
my %arg = (
	o => 'redboot.script',
	s => 0x1000,
	f => 0xbe430000,
	a => 0x80010000,
	l => 0x7c0000,
	t => 20,
);
my $prog = $0;
$prog =~ s/^.*\///;
getopts("r:k:o:s:f:a:l:t:vh", \%arg);

die "usage: $prog ~opts~

  -r <file>	: input rootfs file
  -k <file>	: input kernel file
  -o <file>	: output image file, default $arg{o}
  -s <size_kb>	: redboot script size, default ".sprintf('%d', parse_num($arg{s}))."
  -f <baseaddr>	: flash base, default ".sprintf('0x%x', parse_num($arg{f}))."
  -a <loadaddr>	: Kernel load address, default ".sprintf('0x%x', parse_num($arg{a}))."
  -l <linux_kb>	: linux partition size, default ".sprintf('0x%x', parse_num($arg{l}))."
  -t <timeout> 	: redboot script timeout, default ".sprintf('%d', parse_num($arg{t}))."
  -v		: be more verbose
  -h		: help, version $version

EXAMPLES:
    $prog -k kern -r rootfs
" if $arg{h} || !$arg{k} || !$arg{r};

sub parse_num
{
	my $num = @_[0];
	if (index(lc($num), lc("0x")) == 0) {
		return hex($num);
	} else {
		return $num + 0;
	}
}

sub gen_script
{
	my $kernel_off = parse_num($arg{s});
	my $kernel_addr = parse_num($arg{f});
	my $kernel_len = stat($arg{k})->size;

	my $rootfs_off = $kernel_off + $kernel_len;
	my $rootfs_addr = $kernel_addr + $kernel_len;
	my $rootfs_len = parse_num($arg{l}) - $kernel_len;
	my $rootfs_size = stat($arg{r})->size;

	my $load_addr = parse_num($arg{a});

	my $timeout = parse_num($arg{t});

	if ($arg{v}) {
		printf "kernel_off: 0x%x(%u)\n", $kernel_off, $kernel_off;
		printf "kernel_addr: 0x%x(%u)\n", $kernel_addr, $kernel_addr;
		printf "kernel_len: 0x%x(%u)\n", $kernel_len, $kernel_len;

		printf "rootfs_off: 0x%x(%u)\n", $rootfs_off, $rootfs_off;
		printf "rootfs_addr: 0x%x(%u)\n", $rootfs_addr, $rootfs_addr;
		printf "rootfs_len: 0x%x(%u)\n", $rootfs_len, $rootfs_len;
		printf "rootfs_size: 0x%x(%u)\n", $rootfs_size, $rootfs_size;
	}

	open(FO, ">$arg{o}");
	printf FO "fis init -f\n";
	printf FO "\n";
	printf FO "fconfig boot_script true\n";
	printf FO "fconfig boot_script_data\n";
	printf FO "fis load -b 0x%x -d kernel\n", $load_addr;
	printf FO "exec -c \"noinitrd\" 0x%x\n", $load_addr;
	printf FO "\n";
	printf FO "fconfig boot_script_timeout %d\n", $timeout;
	printf FO "\n";
	printf FO "fis create -o 0x%x -f 0x%x -l 0x%x kernel\n", $kernel_off, $kernel_addr, $kernel_len;
	printf FO "\n";
	printf FO "fis create -o 0x%x -s 0x%x -f 0x%x -l 0x%x rootfs\n", $rootfs_off, $rootfs_size, $rootfs_addr, $rootfs_len;
	printf FO "\n";
	printf FO "reset\n";
	close FO;
}

# MAIN
gen_script();
