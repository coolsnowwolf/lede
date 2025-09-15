#!/usr/bin/env perl

use strict;
use warnings;
use Config;

if (@ARGV < 2) {
	die "Usage: $0 <prefix> <command...>\n";
}

sub gettime {
	my ($sec, $usec);

	eval {
		require Time::HiRes;
		($sec, $usec) = Time::HiRes::gettimeofday();
	};

	unless (defined($sec) && defined($usec)) {
		my $tv_t = ($Config{'longsize'} == 8) ? 'qq' : 'll';
		my $tv = pack $tv_t, 0, 0;

		eval {
			require 'syscall.ph';
			syscall(SYS_gettimeofday(), $tv, 0);
		};

		($sec, $usec) = unpack $tv_t, $tv;
	}

	return ($sec, $usec);
}

my ($prefix, @cmd) = @ARGV;
my ($sec, $usec) = gettime();
my $pid = fork();

if (!defined($pid)) {
	die "$0: Failure to fork(): $!\n";
}
elsif ($pid == 0) {
	exec(@cmd);
	die "$0: Failure to exec(): $!\n";
}
else {
	$SIG{'INT'} = 'IGNORE';
	$SIG{'QUIT'} = 'IGNORE';

	if (waitpid($pid, 0) == -1) {
		die "$0: Failure to waitpid(): $!\n";
	}

	my $exitcode = $? >> 8;
	my ($sec2, $usec2) = gettime();
	my (undef, undef, $cuser, $csystem) = times();

	printf STDOUT "%s#%.2f#%.2f#%.2f\n",
		$prefix, $cuser, $csystem,
		($sec2 - $sec) + ($usec2 - $usec) / 1000000;

	$SIG{'INT'} = 'DEFAULT';
	$SIG{'QUIT'} = 'DEFAULT';

	exit $exitcode;
}
