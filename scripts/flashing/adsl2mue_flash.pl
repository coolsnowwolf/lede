#!/usr/bin/env perl
#
#   Linksys ADSL2MUE Flash utility.
#
#   Copyright (C) 2008 Alexandre Lissy <alexandrelissy@free.fr>
#   based on D-Link DSL-G6x4T flash utility by Felix Fietkau <mailto@nbd.name>
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
use Socket;
use strict;
use warnings;

sub usage() {
	print STDERR "Usage: $0 <ip> [firmware.bin] [partition]\n\n";
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

$scanning = 10;
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
		my ($major, $minor1, $minor2, $code, $addr2) = unpack("vCCVN", $reply);
		$addr2 = pack("N", $addr2);
		if ($code == 1) {
			$scanning = 0;
			printf STDERR " found!\nADAM2 version $major.$minor1.$minor2 at %s (%s)\n", inet_ntoa($addr2), inet_ntoa($addr);
			$box = inet_ntoa($addr2);
		}
	}
}

$box or die " not found!\n";

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

my $file = shift @ARGV;
my $part = shift @ARGV;
$file || exit 0;
$part || exit 0;

open FILE, "<$file" or die "can't open firmware file\n";
my $ftp = ADAM2FTP->new($box, Debug => 0, Timeout => 600) or die "can't open control connection\n";
$ftp->login("adam2", "adam2") or die "can't login\n";

# my $mtd0 = $ftp->getenv("mtd0");
# my $mtd1 = $ftp->getenv("mtd1");
my $mtd4 = $ftp->getenv($part);
# my ($ksize, $fssize);
my ($ossize, $mtd_start, $mtd_end);

# $mtd1 =~ /^(0x\w+),(0x\w+)$/ and $ksize = hex($2) - hex($1);
# $mtd0 =~ /^(0x\w+),(0x\w+)$/ and $fssize = hex($2) - hex($1);
$mtd4 =~ /^(0x\w+),(0x\w+)$/;
$ossize = hex($2) - hex($1);
$mtd_start = hex($1);
$mtd_end = hex($2);
$ossize and $mtd_start and $mtd_end or die 'cannot read partition offsets';
printf STDERR "Available flash space: 0x%08x ($part: 0x%08x to 0x%08x)\n", $ossize, $mtd_start, $mtd_end;

$ftp->command("MEDIA FLSH")->response();
$ftp->binary();

print STDERR "Writing to $part ...\n";
my $dc = $ftp->stor("data $part");
$dc or die "can't open data connection\n";
my $rbytes = 1;

while (($ossize > 0) and ($rbytes > 0)) {
	my $buffer;
	my $len = ($ossize > 1024 ? 1024 : $ossize);
	$rbytes = read FILE, $buffer, $len;
	printf STDERR ".";
	$rbytes and $ossize -= $dc->write($buffer, $rbytes, 600);
}

printf STDERR "\nDone.\n";

$dc->close();
