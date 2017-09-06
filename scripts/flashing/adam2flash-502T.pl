#!/usr/bin/env perl
#
#   D-Link DSL-502T flash utility
#
#   Copyright (c) 2007 Oliver Jowett <oliver@opencloud.com>
#
#   Based on adam2flash.pl for the D-Link DSL-G6x4T, which is:
#   Copyright (C) 2005 Felix Fietkau <mailto@nbd.name>
#   based on fbox recovery util by Enrik Berkhan
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

# The default DSL-502T mtd map looks like this:
#
# mtd0 0x90091000,0x903f0000    # filesystem
# mtd1 0x90010090,0x90091000    # kernel
# mtd2 0x90000000,0x90010000    # bootloader - DO NOT MODIFY
# mtd3 0x903f0000,0x90400000    # config space - DO NOT MODIFY
# mtd4 0x90010000,0x903f0000    # firmware signature + kernel + filesystem, used to flash new firmware
#
# i.e. the flash layout is:
#
# 90000000-9000FFFF mtd2 bootloader
# 90010000-9001008F ---- firmware signature )
# 90010090-90090FFF mtd1 kernel             ) mtd4 spans these three regions
# 90091000-903EFFFF mtd0 filesystem         )
# 903F0000-903FFFFF mtd3 config space
#
# The ADAM2 bootloader uses the mtd1 settings to find the start of the image to boot.
# The image to load contains information about the loadable size of the image. If ADAM2 sees
# that the image appears to extend beyond the end of mtd1, it will refuse to load it. On
# the DSL-502T, this manifests as the USB light blinking rapidly on boot.
#
# The OpenWRT kernel does not follow quite the same layout:
#  (a) it does not have a 0x90-byte firmware signature prefix
#  (b) it is larger than the default mtd1 size
#
# (a) would be avoidable (build a custom image with a 0x90-byte prefix) but (b) is unavoidable.
# So we *have* to change mtd1. The simplest thing to do seems to make it span all of
# the flashable area, producing this layout:
#
# mtd0 0x90091000,0x903f0000    # filesystem
# mtd1 0x90010000,0x903f0000    # kernel (CHANGED)
# mtd2 0x90000000,0x90010000    # bootloader - DO NOT MODIFY
# mtd3 0x903f0000,0x90400000    # config space - DO NOT MODIFY
# mtd4 0x90010000,0x903f0000    # kernel + filesystem, used to flash new firmware
#
# *** NOTE NOTE NOTE NOTE ***
#
# /dev/mtd0 .. /dev/mtd4 when using OpenWRT do **NOT** correspond to the ADAM2 mtd0-4 settings!
# Instead, OpenWRT scans the MTD itself and determines its own boundaries which are arranged
# quite differently to ADAM2. It will look something like this, see dmsg on boot:
#
# (/dev/mtd0) 0x00000000-0x00010000 : "loader"        # Bootloader, read-only
# (/dev/mtd1) 0x003f0000-0x00400000 : "config"        # Config space
# (/dev/mtd2) 0x00010000-0x003f0000 : "linux"         # Firmware area (kernel + root fs + JFFS area)
# (/dev/mtd3) 0x000d0d58-0x003f0000 : "rootfs"        # Root FS, starts immediately after kernel
# (/dev/mtd4) 0x00280000-0x003f0000 : "rootfs_data"   # If rootfs is squashfs, start of JFFS area.
#
# All of those boundaries are autodetected by examining the data in flash.
#
# *** NOTE NOTE NOTE NOTE ***

use IO::Socket::INET;
use Socket;
use strict;
use warnings;

sub usage() {
	print STDERR "Usage: $0 <ip> [-setmtd1] [-noflash] [firmware.bin]\n\n";
	print STDERR "Acquires the ADAM2 bootloader of a D-Link DSL-504T at <ip>\n";
	print STDERR "Power off the device, start this script, then power it on.\n";
	print STDERR "<ip> may be any spare address on the local subnet.\n\n";
	print STDERR "If a firmware file is specified, MTD settings are verified and\n";
	print STDERR "then the firmware is written to the router's flash.\n";
	print STDERR "The firmware type (D-Link or OpenWRT) is automatically detected.\n\n";
	print STDERR "  -setmtd1  update mtd1 if it is not the appropriate value for this firmware\n";
	print STDERR "  -noflash  does normal checks, updates mtd1 if requested, but does not actually write firmware\n\n";
	exit 0;
}

my $ip = shift @ARGV;
$ip and $ip =~ /\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}/ or usage();

my $probe = IO::Socket::INET->new(Proto => 'udp',
                                  Broadcast => 1,
                                  LocalPort => 5035) or die "socket: $!";
my $setip = unpack("N", inet_aton($ip));
$setip > 0 or usage();

my @packets;
foreach my $ver ([18, 1], [22, 2]) {
	push @packets, pack("vCCVNV", 0, @$ver, 1, $setip, 0);
}
print STDERR "Looking for device: ";
my $broadcast = sockaddr_in(5035, INADDR_BROADCAST);
my $scanning;
my $box;

$SIG{"ALRM"} = sub {
	return if --$scanning <= 0;
	foreach my $packet (@packets) {
		$probe->send($packet, 0, $broadcast);
	}
	print STDERR ".";
};

$scanning = 15;
foreach my $packet (@packets) {
	$probe->send($packet, 0, $broadcast);
}
print STDERR ".";

while($scanning) {
	my $reply;

	alarm(1);
	if (my $peer = $probe->recv($reply, 16)) {
		next if (length($reply) < 16);
		my ($port, $addr) = sockaddr_in($peer);
		my ($major, $minor1, $minor2, $code, $addr2) = unpack("vCCVV", $reply);
		$addr2 = pack("N", $addr2);
		if ($code == 2) {
			$scanning = 0;
			printf STDERR " found!\nADAM2 version $major.$minor1.$minor2 at %s (%s)\n", inet_ntoa($addr), inet_ntoa($addr2);
			$box = inet_ntoa($addr);
		}
	}
}

$box or die " not found!\n";

alarm(0);

{
	package ADAM2FTP;
	use base qw(Net::FTP);
	
	# ADAM2 requires upper case commands, some brain dead firewall doesn't ;-)
	sub _USER {
		shift->command("USER",@_)->response()
	}
	
	sub _GETENV {
		my $ftp = shift;
		my ($ok, $name, $value);
		
		$ftp->command("GETENV",@_);
			while(length($ok = $ftp->response()) < 1) {
			my $line = $ftp->getline();
			unless (defined($value)) {
				chomp($line);
				($name, $value) = split(/\s+/, $line, 2);
			}
		}
		$ftp->debug_print(0, "getenv: $value\n")
		if $ftp->debug();
		return $value;
	}
	
	sub getenv {
		my $ftp = shift;
		my $name = shift;
		return $ftp->_GETENV($name);
	}
	
	sub _REBOOT {
		shift->command("REBOOT")->response() == Net::FTP::CMD_OK
	}
	
	sub reboot {
		my $ftp = shift;
		$ftp->_REBOOT;
		$ftp->close;
	}
}

my $file;
my $arg;
my $noflash = 0;
my $setmtd1 = 0;
while ($arg = shift @ARGV) {
  if ($arg eq "-noflash") { $noflash = 1; }
  elsif ($arg eq "-setmtd1") { $setmtd1 = 1; }
  else { $file = $arg; }
}

if (!$file) {
  print STDERR "No firmware file specified, exiting.\n";
  exit 0;
}

#
# Firmware checks
#

open FILE, "<$file" or die "can't open firmware file\n";

# D-Link firmware starts with "MTD4" little-endian, then has an image header at 0x90
# OpenWRT firmware just starts with an image header at 0x00

my $signature;
my $sbytes = read FILE, $signature, 4;
($sbytes == 4) or die "can't read firmware signature: $!";

my $expectedmtd4 = "0x90010000,0x903f0000";
my $fwtype;
my $expectedmtd1;

if ($signature eq "4DTM") {
  seek FILE, 0x90, 0 or die "can't read firmware signature: $!";
  $sbytes = read FILE, $signature, 4;
  ($sbytes == 4) or die "can't read firmware signature: $!";
  if ($signature eq "\x42\xfa\xed\xfe") {
    $fwtype = "D-Link (little-endian)";
    $expectedmtd1 = "0x90010090,0x90091000";
  } elsif ($signature eq "\xde\xad\xbe\x42") {
    $fwtype = "D-Link (big-endian)";
    $expectedmtd1 = "0x90010090,0x90091000";
  }
} elsif ($signature eq "\x42\xfa\xed\xfe") {
  $fwtype = "OpenWRT (little-endian)";
  $expectedmtd1 = "0x90010000,0x903f0000";
} elsif ($signature eq "\xde\xad\xbe\x42") {
  $fwtype = "OpenWRT (big-endian)";
  $expectedmtd1 = "0x90010000,0x903f0000";
}

$fwtype or die "Unknown firmware signature (are you sure that's the right firmware?)";
print STDERR "Firmware type: $fwtype\n";

#
# Bootloader login
#

print STDERR "logging into ADAM2 bootloader.. ";
my $ftp = ADAM2FTP->new($box, Debug => 0, Timeout => 600) or die "can't open control connection\n";
$ftp->login("adam2", "adam2") or die "can't login\n";
print STDERR "ok.\n";

#
# Hardware checks
#

print STDERR "checking hardware.. ";
my $prd = $ftp->getenv("ProductID");
my $usb = $ftp->getenv("usb_prod");
print STDERR "$prd / $usb.\n";
($prd eq "AR7RD" || $prd eq "AR7DB") or die "doesn't look like a DSL-502T?";
($usb eq "DSL-502T") or die "doesn't look like a DSL-502T?";

#
# MTD checks and update
#

print STDERR "checking MTD settings.. ";

my $mtd4 = $ftp->getenv("mtd4");
($mtd4 eq $expectedmtd4) or die "MTD4 was not as expected (should be '$expectedmtd4', was '$mtd4'). Cowardly refusing to do anything about it!";

# check MTD1 setting and update if needed
my $mtd1 = $ftp->getenv("mtd1");
if ($mtd1 ne $expectedmtd1) {
  die "MTD1 was not as expected (should be '$expectedmtd1', was '$mtd1'). Run with -setmtd1 to reset mtd1" unless ($setmtd1);
  print STDERR "Setting mtd1.. ";
  ($ftp->command("SETENV","mtd1,$expectedmtd1")->response() == Net::FTP::CMD_OK) or die "can't set mtd1";
  $file = shift @ARGV;
}

print STDERR "ok.\n";

#
# Firmware size check
#

my $fwsize = (stat(FILE))[7];
printf STDERR "Firmware size: 0x%08x\n", $fwsize;
my $flashsize;
$mtd4 =~ /^(0x\w+),(0x\w+)$/ and $flashsize = hex($2) - hex($1);
printf STDERR "Available flash space: 0x%08x\n", $flashsize;
die "firmware is too large" if ($flashsize < $fwsize);

#
# Flash it!
#

if ($noflash) {
  print STDERR "Not flashing firmware as -noflash was specified.\n";
  exit 0;
}

seek FILE, 0, 0, or die "can't seek in firmware: $!";

print STDERR "Preparing to flash.. ";
($ftp->command("MEDIA FLSH")->response() == Net::FTP::CMD_OK) or die "can't set MEDIA FLSH";
$ftp->binary() or die "can't set binary mode";
print STDERR "ok.\n";
print STDERR "Erasing flash and establishing data connection (this may take a while): ";

my $dc = $ftp->stor("fs mtd4");
$dc or die "can't open data connection: $!\n";
print STDERR "ok.\n";

print STDERR "Writing firmware: ";
while ($fwsize > 0) {
	my $buffer;
	my $len = ($fwsize > 1024 ? 1024 : $fwsize);

	my $rbytes = read FILE, $buffer, $len;
	($rbytes < 0) and die "read error on firmware file: $!";
	($rbytes == $len) or die "short read on firmware file ($rbytes < $len)";

	my $wbytes = $dc->write($buffer, $len, 600);
	($wbytes < 0) and die "write error on FTP data connection: $!";
	($rbytes == $wbytes) or die "short write on FTP data connection ($wbytes < $rbytes)";

	$fwsize -= $len;
	print STDERR ".";
}

$dc->close();
print STDERR " done.\n";

#
# Reboot
#

print STDERR "Rebooting device.\n";
$ftp->reboot();
