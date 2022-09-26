#!/usr/bin/env perl
# 
# Copyright (C) 2006 OpenWrt.org
# Copyright (C) 2016 LEDE project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

use strict;
use warnings;
use File::Basename;
use File::Copy;
use Text::ParseWords;

@ARGV > 2 or die "Syntax: $0 <target dir> <filename> <hash> <url filename> [<mirror> ...]\n";

my $url_filename;
my $target = glob(shift @ARGV);
my $filename = shift @ARGV;
my $file_hash = shift @ARGV;
$url_filename = shift @ARGV unless $ARGV[0] =~ /:\/\//;
my $scriptdir = dirname($0);
my @mirrors;
my $ok;

my $check_certificate = $ENV{DOWNLOAD_CHECK_CERTIFICATE} eq "y";

$url_filename or $url_filename = $filename;

sub localmirrors {
	my @mlist;
	open LM, "$scriptdir/localmirrors" and do {
	    while (<LM>) {
			chomp $_;
			push @mlist, $_ if $_;
		}
		close LM;
	};
	open CONFIG, "<".$ENV{'TOPDIR'}."/.config" and do {
		while (<CONFIG>) {
			/^CONFIG_LOCALMIRROR="(.+)"/ and do {
				chomp;
				my @local_mirrors = split(/;/, $1);
				push @mlist, @local_mirrors;
			};
		}
		close CONFIG;
	};

	my $mirror = $ENV{'DOWNLOAD_MIRROR'};
	$mirror and push @mlist, split(/;/, $mirror);

	return @mlist;
}

sub which($) {
	my $prog = shift;
	my $res = `which $prog`;
	$res or return undef;
	$res =~ /^no / and return undef;
	$res =~ /not found/ and return undef;
	return $res;
}

sub hash_cmd() {
	my $len = length($file_hash);
	my $cmd;

	$len == 64 and return "$ENV{'MKHASH'} sha256";
	$len == 32 and return "$ENV{'MKHASH'} md5";
	return undef;
}

sub download_cmd {
	my $url = shift;
	my $have_curl = 0;
	my $have_aria2c = 0;
	my $filename = shift;
	my $additional_mirrors = join(" ", map "$_/$filename", @_);

	my @chArray = ('a'..'z', 'A'..'Z', 0..9);
	my $rfn = join '', "${filename}_", map{ $chArray[int rand @chArray] } 0..9;
	if (open CURL, '-|', 'curl', '--version') {
		if (defined(my $line = readline CURL)) {
			$have_curl = 1 if $line =~ /^curl /;
		}
		close CURL;
	}
	if (open ARIA2C, '-|', 'aria2c', '--version') {
		if (defined(my $line = readline ARIA2C)) {
			$have_aria2c = 1 if $line =~ /^aria2 /;
		}
		close ARIA2C;
	}

	if ($have_aria2c) {
		@mirrors=();
		return join(" ", "[ -d $ENV{'TMPDIR'}/aria2c ] || mkdir $ENV{'TMPDIR'}/aria2c;",
			"touch $ENV{'TMPDIR'}/aria2c/${rfn}_spp;",
			qw(aria2c --stderr -c -x2 -s10 -j10 -k1M), $url, $additional_mirrors,
			$check_certificate ? () : '--check-certificate=false',
			"--server-stat-of=$ENV{'TMPDIR'}/aria2c/${rfn}_spp",
			"--server-stat-if=$ENV{'TMPDIR'}/aria2c/${rfn}_spp",
			"-d $ENV{'TMPDIR'}/aria2c -o $rfn;",
			"cat $ENV{'TMPDIR'}/aria2c/$rfn;",
			"rm $ENV{'TMPDIR'}/aria2c/$rfn $ENV{'TMPDIR'}/aria2c/${rfn}_spp");
	} elsif ($have_curl) {
		return (qw(curl -f --connect-timeout 20 --retry 5 --location),
			$check_certificate ? () : '--insecure',
			shellwords($ENV{CURL_OPTIONS} || ''),
			$url);
	} else {
		return (qw(wget --tries=5 --timeout=20 --output-document=-),
			$check_certificate ? () : '--no-check-certificate',
			shellwords($ENV{WGET_OPTIONS} || ''),
			$url);
	}
}

my $hash_cmd = hash_cmd();
$hash_cmd or ($file_hash eq "skip") or die "Cannot find appropriate hash command, ensure the provided hash is either a MD5 or SHA256 checksum.\n";

sub download
{
	my $mirror = shift;
	my $download_filename = shift;
	my @additional_mirrors = @_;

	$mirror =~ s!/$!!;

	if ($mirror =~ s!^file://!!) {
		if (! -d "$mirror") {
			print STDERR "Wrong local cache directory -$mirror-.\n";
			cleanup();
			return;
		}

		if (! -d "$target") {
			system("mkdir", "-p", "$target/");
		}

		if (! open TMPDLS, "find $mirror -follow -name $filename 2>/dev/null |") {
			print("Failed to search for $filename in $mirror\n");
			return;
		}

		my $link;

		while (defined(my $line = readline TMPDLS)) {
			chomp ($link = $line);
			if ($. > 1) {
				print("$. or more instances of $filename in $mirror found . Only one instance allowed.\n");
				return;
			}
		}

		close TMPDLS;

		if (! $link) {
			print("No instances of $filename found in $mirror.\n");
			return;
		}

		print("Copying $filename from $link\n");
		copy($link, "$target/$filename.dl");

		$hash_cmd and do {
			if (system("cat '$target/$filename.dl' | $hash_cmd > '$target/$filename.hash'")) {
				print("Failed to generate hash for $filename\n");
				return;
			}
		};
	} else {
		my @cmd = download_cmd("$mirror/$download_filename", $download_filename, @additional_mirrors);
		print STDERR "+ ".join(" ",@cmd)."\n";
		open(FETCH_FD, '-|', @cmd) or die "Cannot launch aria2c, curl or wget.\n";
		$hash_cmd and do {
			open MD5SUM, "| $hash_cmd > '$target/$filename.hash'" or die "Cannot launch $hash_cmd.\n";
		};
		open OUTPUT, "> $target/$filename.dl" or die "Cannot create file $target/$filename.dl: $!\n";
		my $buffer;
		while (read FETCH_FD, $buffer, 1048576) {
			$hash_cmd and print MD5SUM $buffer;
			print OUTPUT $buffer;
		}
		$hash_cmd and close MD5SUM;
		close FETCH_FD;
		close OUTPUT;

		if ($? >> 8) {
			print STDERR "Download failed.\n";
			cleanup();
			return;
		}
	}

	$hash_cmd and do {
		my $sum = `cat "$target/$filename.hash"`;
		$sum =~ /^(\w+)\s*/ or die "Could not generate file hash\n";
		$sum = $1;

		if ($sum ne $file_hash) {
			print STDERR "Hash of the downloaded file does not match (file: $sum, requested: $file_hash) - deleting download.\n";
			cleanup();
			return;
		}
	};

	unlink "$target/$filename";
	system("mv", "$target/$filename.dl", "$target/$filename");
	cleanup();
}

sub cleanup
{
	unlink "$target/$filename.dl";
	unlink "$target/$filename.hash";
}

@mirrors = localmirrors();

foreach my $mirror (@ARGV) {
	if ($mirror =~ /^\@SF\/(.+)$/) {
		# give sourceforge a few more tries, because it redirects to different mirrors
		for (1 .. 5) {
			push @mirrors, "https://netix.dl.sourceforge.net/$1";
			push @mirrors, "https://freefr.dl.sourceforge.net/$1";
			push @mirrors, "https://downloads.sourceforge.net/$1";
		}
	} elsif ($mirror =~ /^\@OPENWRT$/) {
		# use OpenWrt source server directly
	} elsif ($mirror =~ /^\@DEBIAN\/(.+)$/) {
		push @mirrors, "https://mirrors.tencent.com/debian/$1";
		push @mirrors, "https://ftp.debian.org/debian/$1";
		push @mirrors, "https://mirror.leaseweb.com/debian/$1";
		push @mirrors, "https://mirror.netcologne.de/debian/$1";
	} elsif ($mirror =~ /^\@APACHE\/(.+)$/) {
		push @mirrors, "https://mirrors.cloud.tencent.com/apache/$1";
		push @mirrors, "https://mirror.netcologne.de/apache.org/$1";
		push @mirrors, "https://mirror.aarnet.edu.au/pub/apache/$1";
		push @mirrors, "https://mirror.csclub.uwaterloo.ca/apache/$1";
		push @mirrors, "https://archive.apache.org/dist/$1";
		push @mirrors, "http://mirror.cogentco.com/pub/apache/$1";
		push @mirrors, "http://mirror.navercorp.com/apache/$1";
		push @mirrors, "http://ftp.jaist.ac.jp/pub/apache/$1";
		push @mirrors, "ftp://apache.cs.utah.edu/apache.org/$1";
		push @mirrors, "ftp://apache.mirrors.ovh.net/ftp.apache.org/dist/$1";
	} elsif ($mirror =~ /^\@GITHUB\/(.+)$/) {
		my $dir = $1;
		my $i = 0;
		# replace the 2nd '/' with '@' for jsDelivr mirror
		push @mirrors, "https://cdn.jsdelivr.net/gh/". $dir =~ s{\/}{++$i == 2 ? '@' : $&}ger;
		# give github a few more tries (different mirrors)
		for (1 .. 5) {
			push @mirrors, "https://raw.githubusercontent.com/$dir";
		}
	} elsif ($mirror =~ /^\@GNU\/(.+)$/) {
		push @mirrors, "https://mirrors.cloud.tencent.com/gnu/$1";
		push @mirrors, "https://mirror.csclub.uwaterloo.ca/gnu/$1";
		push @mirrors, "https://mirror.netcologne.de/gnu/$1";
		push @mirrors, "http://ftp.kddilabs.jp/GNU/gnu/$1";
		push @mirrors, "http://www.nic.funet.fi/pub/gnu/gnu/$1";
		push @mirrors, "http://mirror.internode.on.net/pub/gnu/$1";
		push @mirrors, "http://mirror.navercorp.com/gnu/$1";
		push @mirrors, "ftp://mirrors.rit.edu/gnu/$1";
		push @mirrors, "ftp://download.xs4all.nl/pub/gnu/$1";
		push @mirrors, "https://ftp.gnu.org/gnu/$1";
	} elsif ($mirror =~ /^\@SAVANNAH\/(.+)$/) {
		push @mirrors, "https://mirror.netcologne.de/savannah/$1";
		push @mirrors, "https://mirror.csclub.uwaterloo.ca/nongnu/$1";
		push @mirrors, "http://ftp.acc.umu.se/mirror/gnu.org/savannah/$1";
		push @mirrors, "http://nongnu.uib.no/$1";
		push @mirrors, "http://ftp.igh.cnrs.fr/pub/nongnu/$1";
		push @mirrors, "ftp://cdimage.debian.org/mirror/gnu.org/savannah/$1";
		push @mirrors, "ftp://ftp.acc.umu.se/mirror/gnu.org/savannah/$1";
	} elsif ($mirror =~ /^\@KERNEL\/(.+)$/) {
		my @extra = ( $1 );
		if ($filename =~ /linux-\d+\.\d+(?:\.\d+)?-rc/) {
			push @extra, "$extra[0]/testing";
		} elsif ($filename =~ /linux-(\d+\.\d+(?:\.\d+)?)/) {
			push @extra, "$extra[0]/longterm/v$1";
		}
		foreach my $dir (@extra) {
			push @mirrors, "https://mirrors.ustc.edu.cn/kernel.org/$dir";
			push @mirrors, "https://cdn.kernel.org/pub/$dir";
			push @mirrors, "https://download.xs4all.nl/ftp.kernel.org/pub/$dir";
			push @mirrors, "https://mirrors.mit.edu/kernel/$dir";
			push @mirrors, "http://ftp.nara.wide.ad.jp/pub/kernel.org/$dir";
			push @mirrors, "http://www.ring.gr.jp/archives/linux/kernel.org/$dir";
			push @mirrors, "ftp://ftp.riken.jp/Linux/kernel.org/$dir";
			push @mirrors, "ftp://www.mirrorservice.org/sites/ftp.kernel.org/pub/$dir";
		}
	} elsif ($mirror =~ /^\@GNOME\/(.+)$/) {
		push @mirrors, "https://download.gnome.org/sources/$1";
		push @mirrors, "https://mirrors.ustc.edu.cn/gnome/sources/$1";
		push @mirrors, "https://mirror.csclub.uwaterloo.ca/gnome/sources/$1";
		push @mirrors, "http://ftp.acc.umu.se/pub/GNOME/sources/$1";
		push @mirrors, "http://ftp.kaist.ac.kr/gnome/sources/$1";
		push @mirrors, "http://www.mirrorservice.org/sites/ftp.gnome.org/pub/GNOME/sources/$1";
		push @mirrors, "http://mirror.internode.on.net/pub/gnome/sources/$1";
		push @mirrors, "http://ftp.belnet.be/ftp.gnome.org/sources/$1";
		push @mirrors, "ftp://ftp.cse.buffalo.edu/pub/Gnome/sources/$1";
		push @mirrors, "ftp://ftp.nara.wide.ad.jp/pub/X11/GNOME/sources/$1";
	} else {
		push @mirrors, $mirror;
	}
}

push @mirrors, 'https://sources.cdn.openwrt.org';
push @mirrors, 'https://sources.openwrt.org';
push @mirrors, 'https://mirror2.openwrt.org/sources';

if (-f "$target/$filename") {
	$hash_cmd and do {
		if (system("cat '$target/$filename' | $hash_cmd > '$target/$filename.hash'")) {
			die "Failed to generate hash for $filename\n";
		}

		my $sum = `cat "$target/$filename.hash"`;
		$sum =~ /^(\w+)\s*/ or die "Could not generate file hash\n";
		$sum = $1;

		cleanup();
		exit 0 if $sum eq $file_hash;

		die "Hash of the local file $filename does not match (file: $sum, requested: $file_hash) - deleting download.\n";
		unlink "$target/$filename";
	};
}

while (!-f "$target/$filename") {
	my $mirror = shift @mirrors;
	$mirror or die "No more mirrors to try - giving up.\n";

	download($mirror, $url_filename, @mirrors);
	if (!-f "$target/$filename" && $url_filename ne $filename) {
		download($mirror, $filename, @mirrors);
	}
}

$SIG{INT} = \&cleanup;
