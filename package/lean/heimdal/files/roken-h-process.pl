#!/usr/bin/perl

use Getopt::Std;

my $debug = 0;

getopts('dc:p:o:') || die "foo";

if ($opt_d) {
    $debug = 1;
}

die "missing arg" if (!defined $opt_c || !defined $opt_p || !defined $opt_o);

my %defines;
my $IN;
my $OUT;

print "parse config.h\n" if ($debug);

open IN, $opt_c || die "failed open ${opt_c}";

my @nesting;

push @nesting, 1;

while (<IN>) {
    if (m/\s*#ifdef\s+(.*)/) {
	my $var = $1;
	if (defined $defines{$var}) {
	    push @nesting, 1;
	} else {
	    push @nesting, 0;
	}
	next;
    } elsif (m/\s*#ifndef\s+(.*)/) {
	my $var = $1;
	if (defined $defines{$var}) {
	    push @nesting, 0;
	} else {
	    push @nesting, 1;
	}
	next;
    } elsif (m/\s*#else/) {
	my $var = pop @nesting;
	$var = !$var;
	push @nesting, $var;
	next;
    } elsif ($nesting[$#nesting] and m/\s*#define\s+(\w+)\s+(\S+)/) {
	my $res = $2;
	$res = 1 if (!defined $res);
	$defines{$1} = $res;
    }
}

close IN;

if ($debug) {
    foreach my $i (keys %defines) {
	print "k: $i v: $defines{$i}\n";
    }
}

open IN, "$opt_p" || die "failed open ${opt_p}";
open OUT, ">$opt_o" || die "failed open ${opt_o}";

print "parse roken.h.in\n" if ($debug);

print OUT "/* This is an OS dependent, generated file */\n";
print OUT "\n";
print OUT "\n";
print OUT "#ifndef __ROKEN_H__\n";
print OUT "#define __ROKEN_H__\n";
print OUT "\n";

@nesting = (1);

while (<IN>) {
    if (m/\s*#ifdef\s+(.*)/) {
	my $var = $1;
	if (defined $defines{$var}) {
	    push @nesting, 1;
	} else {
	    push @nesting, 0;
	}
	next;
    } elsif (m/\s*#ifndef\s+(.*)/) {
	my $var = $1;
	if (defined $defines{$var}) {
	    push @nesting, 0;
	} else {
	    push @nesting, 1;
	}
	next;
    } elsif (m/\s*#if\s+(.*)/) {
	my $res = parse_if($1);
	print "line = $res: $1\n" if ($debug);
	push @nesting, $res;
	next;
    } elsif (m/\s*#elif\s+(.*)/) {
	my $res = pop @nesting;
	if ($res gt 0) {
	    $res = -1;
	} else {
	    my $res = parse_if($1);
	}
	push @nesting, $res;
	next;
    } elsif (m/\s*#else/) {
	my $var = pop @nesting;
	$var = !$var;
	push @nesting, $var;
	next;
    } elsif (m/\s*#endif/) {
	pop @nesting;
	next;
    }
    print "line: $_\n"  if ($debug);
    print "nesting dep $#{nesting}\n"  if ($debug);
    my $i = 0, $t = 1;
    while ($i le $#nesting) {
	$t = 0 if ($nesting[$i] le 0);
	print "nesting $i val $nesting[$i] -> $t\n" if ($debug);
	$i++;
    }
    if ($t) {
	print OUT;
    }
}

print OUT "\n";
print OUT "#endif /* __ROKEN_H__ */\n";


close IN;

exit 0;

sub parse_if
{
    my ($neg, $var);

    $_ = shift;

    if (m/^\s*$/) {
	print "end $_\n" if ($debug);
	return 1;
    } elsif (m/^\(([^&]+)\&\&(.*)\)\s*\|\|\s*\(([^&]+)\&\&(.*)\)$/) {
	print "($1 and $2) or ($3 and $4)\n" if ($debug);
	return ((parse_if($1) and parse_if($2)) or (parse_if($3) and parse_if($4)));
    } elsif (m/^([^&]+)\&\&(.*)$/) {
	print "$1 and $2\n" if ($debug);
	return parse_if($1) and parse_if($2);
    } elsif (m/^([^\|]+)\|\|(.*)$/) {
	print "$1 or $2\n" if ($debug);
	return (parse_if($1) or parse_if($2));
    } elsif (m/^\s*(\!)?\s*defined\((\w+)\)/) {
	($neg, $var) = ($1, $2);
	print "def: ${neg}-defined(${var})\n" if ($debug);
	my $res = defined $defines{$var};
	if ($neg eq "!") {
	    if ($res) {
		$res = 0;
	    } else {
		$res = 1;
	    }
	}
	print "res: $res\n" if ($debug);
	return $res;
    } elsif (m/^\s*(\!)?(\w+)/) {
	($neg, $var) = ($1, $2);
	print "var: $neg $var\n" if ($debug);
	my $res;
	if (defined $defines{$var}) {
	    $res = $defines{$var};
	} else {
	    $res = 0;
	}
	$res = ! $res if ($neg =~ m/!/);
	print "res: $res\n" if ($debug);
	return $res;
    }
    die "failed parse: $_\n";
}
