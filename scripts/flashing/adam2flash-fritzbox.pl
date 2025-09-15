#!/usr/bin/env perl
#
#   D-Link DSL-G6x4T flash utility
#
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

use IO::Socket::INET;
use IO::Select;
use Socket;
use strict;
use warnings;

sub usage() {
	print STDERR "Usage: $0 <ip> [firmware.bin]\n\n";
	exit 0;
}

my $ip = shift @ARGV;
$ip and $ip =~ /\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}/ or usage();

my $setip = unpack("N", inet_aton($ip));
$setip > 0 or usage();

my @packets;
foreach my $ver ([18, 1], [22, 2]) {
	push @packets, pack("vCCVNV", 0, @$ver, 1, $setip, 0);
}
print STDERR "Looking for device: ";
my $scanning;
my $box;

my $probe = IO::Socket::INET->new(Proto => 'udp',
                                  Broadcast => 1,
                                  LocalAddr => $ip,
                                  LocalPort => 5035) or die "socket: $!";
my $sel = IO::Select->new($probe);
my $packet = pack("vCCVNV", 0, 18, 1, 1, 0, 0);
my $broadcast = sockaddr_in(5035, INADDR_BROADCAST);

$probe->send($packet, 0, $broadcast);


scan_again:
print "Looking for Fritz!Box ";
my @boxes = ();
my $peer;
$scanning = 100;
print "o";
while($scanning) {
  my $reply;
  my @ready;

  if (@ready = $sel->can_read(0.2)) {
    $peer = $probe->recv($reply, 16);
    next if (length($reply) < 16);
    my ($port, $addr) = sockaddr_in($peer);
    my ($major, $minor1, $minor2, $code, $addr2) = unpack("vCCVV", $reply);
    $addr2 = pack("N", $addr2);
    if ($code == 2) {
      print "O";
      push @boxes, [$major, $minor1, $minor2, $addr, $addr2];
      $scanning = 2 if ($scanning > 2);
    }
  } else {
    $scanning--;
    if (scalar @boxes == 0) {
      $probe->send($packet, 0, $broadcast);
      print "o";
    } else {
      print ".";
    }
  }
}

if (scalar @boxes == 0) {
  print " none found, giving up.\n";
  exit 1;
} else {
  print " found!\n";
}

{
  package ADAM2FTP;
  use base qw(Net::FTP);
  # ADAM2 requires upper case commands, some brain dead firewall doesn't ;-)
  sub _USER { shift->command("USER",@_)->response() }
  sub _PASV { shift->command("P\@SW")->response() == Net::FTP::CMD_OK }
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
  sub _REBOOT { shift->command("REBOOT")->response() == Net::FTP::CMD_OK }
  sub reboot {
    my $ftp = shift;
    $ftp->_REBOOT;
    $ftp->close;
  }
  sub check {
    my $ftp = shift;
    
    delete ${*$ftp}{'net_ftp_port'};
    delete ${*$ftp}{'net_ftp_pasv'};

    my $data = $ftp->_data_cmd('CHECK' ,@_) or return undef;
    my $sum;
    if (${${*$ftp}{'net_cmd_resp'}}[0] =~ /^Flash check 0x([0-9A-F]{8})/) {
      $sum = hex($1);
    }
    $data->_close();
    return $sum;
  }
}

# passive mode geht mit Net::FTP nicht, connected zu spaet fuer ADAM2!
my $ftp = ADAM2FTP->new($ip, Passive => 0, Debug => 0, Timeout => 600)
  or die "can't FTP ADAM2";
$ftp->login("adam2", "adam2") or die "can't login adam2";
$ftp->binary();
my $pid   = $ftp->getenv('ProductID');
my $hwrev = $ftp->getenv('HWRevision');
my $fwrev = $ftp->getenv('firmware_info');
my $ulrev = $ftp->getenv('urlader-version');

print "Product ID: $pid\n";
print "Hardware Revision: $hwrev\n";
print "Urlader  Revision: $ulrev\n";
print "Firmware Revision: $fwrev\n";

$ftp->hash(\*STDOUT, 64 * 1024);

my $file = shift @ARGV;
$file || exit 0;

open FILE, "<$file" or die "can't open firmware file\n";

my $mtd0 = $ftp->getenv("mtd0");
my $mtd1 = $ftp->getenv("mtd1");
my ($ksize, $fssize);

$mtd1 =~ /^(0x\w+),(0x\w+)$/ and $ksize = hex($2) - hex($1);
$mtd0 =~ /^(0x\w+),(0x\w+)$/ and $fssize = hex($2) - hex($1);
$ksize and $fssize or die 'cannot read partition offsets';
printf STDERR "Available flash space: 0x%08x (0x%08x + 0x%08x)\n", $ksize + $fssize, $ksize, $fssize;

$ftp->command("MEDIA FLSH")->response();
$ftp->binary();
print STDERR "Writing to mtd1...\n";

my $dc = $ftp->stor("fs mtd1");
$dc or die "can't open data connection\n";
my $rbytes = 1;

while (($ksize > 0) and ($rbytes > 0)) {
	my $buffer;
	my $len = ($ksize > 1024 ? 1024 : $ksize);
	$rbytes = read FILE, $buffer, $len;
	$rbytes and $ksize -= $dc->write($buffer, $rbytes, 600);
}

$dc->close();
$rbytes or die "no more data left to write\n";

print STDERR "Writing to mtd0...\n";

$dc = $ftp->stor("fs mtd0");
$dc or die "can't open data connection\n";

while (($fssize > 0) and ($rbytes > 0)) {
	my $buffer;
	my $len = ($fssize > 1024 ? 1024 : $fssize);
	$rbytes = read FILE, $buffer, $len;
	$rbytes and $fssize -= $dc->write($buffer, $rbytes, 600);
}

$dc->close();
$ftp->reboot();
