#!/usr/bin/env perl

use strict;
use warnings;
use Cwd;

my (%targets, %architectures);

$ENV{'TOPDIR'} = Cwd::getcwd();


sub parse_targetinfo {
	my ($target_dir, $subtarget) = @_;

	if (open M, "make -C '$target_dir' --no-print-directory DUMP=1 TARGET_BUILD=1 SUBTARGET='$subtarget' |") {
		my ($target_name, $target_arch, @target_features);
		while (defined(my $line = readline M)) {
			chomp $line;

			if ($line =~ /^Target: (.+)$/) {
				$target_name = $1;
			}
			elsif ($line =~ /^Target-Arch-Packages: (.+)$/) {
				$target_arch = $1;
			}
			elsif ($line =~ /^Target-Features: (.+)$/) {
				@target_features = split /\s+/, $1;
			}
			elsif ($line =~ /^@\@$/) {
				if ($target_name && $target_arch &&
				    !grep { $_ eq 'broken' or $_ eq 'source-only' } @target_features) {
					$targets{$target_name} = $target_arch;
					$architectures{$target_arch} ||= [];
					push @{$architectures{$target_arch}}, $target_name;
				}

				undef $target_name;
				undef $target_arch;
				@target_features = ();
			}
		}
		close M;
	}
}

sub get_targetinfo {
	foreach my $target_makefile (glob "target/linux/*/Makefile") {
		my ($target_dir) = $target_makefile =~ m!^(.+)/Makefile$!;
		my @subtargets;

		if (open M, "make -C '$target_dir' --no-print-directory DUMP=1 TARGET_BUILD=1 val.FEATURES V=s 2>/dev/null |") {
			if (defined(my $line = readline M)) {
				chomp $line;
				if (grep { $_ eq 'broken' or $_ eq 'source-only' } split /\s+/, $line) {
					next;
				}
			}
		}

		if (open M, "make -C '$target_dir' --no-print-directory DUMP=1 TARGET_BUILD=1 val.SUBTARGETS V=s 2>/dev/null |") {
			if (defined(my $line = readline M)) {
				chomp $line;
				@subtargets = split /\s+/, $line;
			}
			close M;
		}

		push @subtargets, 'generic' if @subtargets == 0;

		foreach my $subtarget (@subtargets) {
			parse_targetinfo($target_dir, $subtarget);
		}
	}
}

if (@ARGV == 1 && $ARGV[0] eq 'targets') {
	get_targetinfo();
	foreach my $target_name (sort keys %targets) {
		printf "%s %s\n", $target_name, $targets{$target_name};
	}
}
elsif (@ARGV == 1 && $ARGV[0] eq 'architectures') {
	get_targetinfo();
	foreach my $target_arch (sort keys %architectures) {
		printf "%s %s\n", $target_arch, join ' ', @{$architectures{$target_arch}};
	}
}
else {
	print "Usage: $0 targets\n";
	print "Usage: $0 architectures\n";
}
