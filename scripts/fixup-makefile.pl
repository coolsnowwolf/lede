#!/usr/bin/env perl
use strict;

my $error;
my %state;

sub usage() {
die <<EOF;
Usage: $0 <file> <command> [<arguments>]

Commands:
add-hash <variable> <value>
fix-hash <variable> <value>
rename-var <variable> <name>

EOF
}

sub set_var($) {
	my $var = shift;

	$state{var} = $var;
	if ($var =~ /(.*):(.*)/) {
		$state{template} = $1;
		$state{var} = $2;
		$state{related_var} = "URL";
	} else {
		$state{context} = 1;
		$state{related_var} = "PKG_SOURCE";
	}
}

my %check_command = (
	"add-hash" => sub {
		set_var($ARGV[0]);

		$state{value} = $ARGV[1];
		length($ARGV[1]) == 64 or die "Invalid hash value\n";
	},
	"fix-hash" => sub {
		set_var($ARGV[0]);

		$state{value} = $ARGV[1];
		$state{prev_value} = $ARGV[2];

		length($ARGV[1]) == 64 or die "Invalid hash value\n";
	},
	"rename-var" => sub {
		set_var($ARGV[0]);
		$state{new_var} = $ARGV[1];
		$state{new_var} =~ s/.*://g;
	},
);

sub check_context($) {
	my $line = shift;
	return unless $state{template};

	$state{next} and do {
		$state{context} = 1;
		undef $state{next};
		return;
	};

	if (not $state{context}) {
		$line =~ /^\s*define\s+$state{template}/ and $state{next} = 1;
	} else {
		$line =~ /^\s*endef/ and do {
			$state{done} = 1;
			undef $state{context};
		}
	}
}

my %commands = (
	"add-hash" => sub {
		my $line = shift;
		check_context($line);
		return $line unless $state{context};

		# skip existing hash variable
		return "" if $line =~ /^(\s*)$state{var}(\s*):?=(\s*)(.*)\n/;

		# insert md5sum after related variable
		return $line unless $line =~ /^(\s*)$state{related_var}(\s*):?=(\s*)(.*)\n/;
		return "$line$1$state{var}$2:=$3$state{value}\n";
	},
	"fix-hash" => sub {
		my $line = shift;
		check_context($line);
		return $line unless $state{context};
		return $line unless $line =~ /^(\s*)$state{var}(\s*):?=(\s*)(.*)$state{prev_value}(.*)\n/;
		$state{done} = 1;
		$4 =~ /\$/ and do {
			warn "$state{var} contains a reference to another variable, can't fix automatically\n";
			return $line;
		};
		return "$1$state{var}$2:=$3$state{value}\n";
	},
	"rename-var" => sub {
		my $line = shift;
		check_context($line);
		return $line unless $state{context};
		return $line unless $line =~ /^(\s*)$state{var}(\s*:?=.*)\n/;
		return "$1$state{new_var}$2\n";
	},
);

my $file = shift @ARGV;
my $command = shift @ARGV;

($file and $command and $check_command{$command}) or usage;
&{$check_command{$command}}();

-f $file or die "File $file not found\n";

open IN, "<${file}" or die "Cannot open input file\n";
open OUT, ">${file}.new" or die "Cannot open output file\n";

my $cmd = $commands{$command};
while (my $line = <IN>) {
	$line = &$cmd($line) unless $state{done};
	print OUT $line;
	last if $error;
}

close OUT;
close IN;

$error and do {
	unlink "${file}.new";
	exit 1;
};

rename "${file}.new", "$file";
