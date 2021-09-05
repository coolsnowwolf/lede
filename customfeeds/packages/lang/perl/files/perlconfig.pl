#!/usr/bin/perl

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# 
# Copyright 2015 Marcel Denia

=head1 NAME

perlconfig.pl

=head1 SYNOPSIS

B<perlconfig.pl> [B<-Dsymbol>=I<value>, ...] [B<-dsymbol>=I<value>, ...] I<[files]>

Generate a configuration file suitable for (cross-)compiling perl 5.

=head1 OPTIONS

=over

=item B<-Dsymbol=value>

The symbol identified by I<name> will have the literal value I<value>.
When generating the configuration file, it's value will be printed enclosed by
single quotation marks(').

=item B<-dsymbol=value>

The symbol identified by I<name> will have the literal value I<value>.

=back

=head1 DESCRIPTION

B<perlconfig.pl> is a program to compile and generate configuration files ready
to be used as a "config.sh" file for compiling perl 5. It does so by processing
specially-made configuration files(usually called *.config), typically augmented
by command-line definitions.

B<perlconfig.pl>'s intent is to be used in place of perl 5's own Configure
script in situations where it can not be run, like cross-compiling.

=head2 File syntax

B<perlconfig.pl>'s configuration files a consist of symbol definitions in
different variations, each one assigning a specific symbol identified by a name
some value, as well as conditional blocks in order to allow for some
flexibility.

=head3 Symbol names

A symbol name has to consist entirely of alphanumeric characters as well as
the underscore(_) character. In addition, symbol names may be prefixed with an
all-lowercase string, separated by a colon(:):

  my:name=value

Having a zero-length prefix string is also valid:

  :name=value

Symbols prefixed that way are called private symbols. They act exactly like
regular symbols, with the only difference being that they will B<not> be written
to the final configuration file.

=head3 Symbol definitions

A symbol definition is in the form of a simple name/value pair, separated by
an equals sign(=):

  name=value

I<value> can be anything really. However, there are 3 notations, with
differences in quoting and interpolation:

=over

=item name=foo

The symbol identified by I<name> will have the literal value I<foo>.

=item name='foo'

The symbol identified by I<name> will have the literal value I<foo>.
When generating the configuration file, it's value will be printed enclosed by
single quotation marks(').

=item name="foo"

The symbol identified by I<name> will have the value of I<foo>
S<B<after interpolation>>(as described in L</Interpolation>).
When generating the configuration file, it's value will be printed enclosed by
single quotation marks(').

=back

=head3 Conditional blocks

A conditional block is of the form

  (condition) {
      ...
  }

B<perlconfig.pl> will execute everything enclosed in the curly braces({ and }),
or inside the BLOCK in Perl 5 terms, if I<condition> evaluates to any true
value. I<condition> will go through interpolation as described in
L</Interpolation>. It may contain any valid Perl 5 expression. Some common
examples are:

=over

=item $name eq 'foo'

Evaluates to true if configuration symbol I<name> is literally equal to I<foo>.

=item $name ne 'foo'

Evaluates to true if configuration symbol I<name> is B<not> literally equal to
I<foo>.

=item defined($name)

Evaluates to true if configuration symbol I<name> is defined(has any usable
value, see L<perlfunc/defined>).

=back

Conditional blocks may be nested inside conditional blocks. Note that the
opening curl brace({) has to be on the same line as your condition.

=head3 Comments

All lines starting with nothing or any number of whitespaces, followed by a
hash sign(#), are considered comments and will be completely ignored by
B<perlconfig.pl>.

=head3 Interpolation

In certain situations(see above), B<perlconfig.pl> will interpolate strings or
constructs in order to allow you to refer to configuration symbols or embed
code.

Interpolated strings are subject to the following rules:

=over

=item You may not include any single(') or double(") quotation marks.

You can use \qq in order to include double quotation marks(") in your string.

=item $name and ${name} reference configuration symbols

You can easily refer to existing configuration symbols using the commmon $name
or ${name} syntax. In case you want to refer to the perl variable named $name,
write \$name. This is useful for embedding code.

=item Perl 5 interpolation rules apply

Aside from the above, you may include anything that is also valid for an
interpolated(qq//) string in Perl 5. For instance, it's perfectly valid to
execute code using the @{[]} construct.

=back

=head1 EXAMPLES

As a simple example, consider the following configuration file, named
"example.config":

  recommendation='The Perl you want is'
  ($:maturity eq 'stable') {
      recommendation="$recommendation Perl 5"
  }
  ($:maturity eq 'experimental') {
      recommendation="$recommendation Perl 6(try Rakudo!)"
  }

Executing it using these command-lines will yield the following results:

=over

=item $ perlconfig.pl -D:maturity=stable example.config

  recommendation='The Perl you want is Perl 5'

=item $ perlconfig.pl -D:maturity=experimental example.config

  recommendation='The Perl you want is Perl 6(try Rakudo!)'

=back

=head1 AUTHOR

Marcel Denia <naoir@gmx.net>

=head1 COPYRIGHT AND LICENSE

Copyright 2015 Marcel Denia

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

=cut

use strict;
use warnings;
use List::Util qw/all/;
my $symbol_name_prefix_regex = '(?:[a-z]*:)';
my $symbol_name_regex = "($symbol_name_prefix_regex?(?:[a-zA-Z0-9_]+))";

my %config;

sub interpolate {
	my $string = shift;
	my %options = @_;
	
	# First, convert $foo into ${foo}
	$string =~ s/(?<!\\)\$$symbol_name_regex/\${$1}/gs;
	
	# Make ${foo} into $config{'foo'}->{value}
	$string =~ s/\$\{$symbol_name_regex\}/\$config{\'$1\'}->{value}/g;
	
	# Un-escape \$foo
	$string =~ s/\\\$/\$/g;
	
	# Turn \qq into "
	$string =~ s/\\qq/\\"/g;
	
	return $string;
}

# Parse command-line symbol definitions
while ($ARGV[0]) {
	if ($ARGV[0] =~ /^-([D|d])$symbol_name_regex=(.*)$/) {
		$config{$2} = { value => $3, quoted => $1 eq 'D' };
		shift(@ARGV);
	}
	else {
		last;
	}
}

# Process configuration files
my @condition_stack = ( 1 );
for my $file (@ARGV) {
	open(my $fh, '<', $file) or die "Can't open $file: $!\n";
	while (my $line = <$fh>) {
		chomp($line);
		
		if ($line =~ /^\s*$symbol_name_regex=(.*)$/) { # A symbol definition
			if (all {$_ == 1} @condition_stack) {
				my $symbol = $1;
				(my $quote_begin, my $value, my $quote_end) = $2 =~ /^(['|"])?([^'"]*)(['|"])?$/;
				
				$quote_begin = '' unless defined $quote_begin;
				$quote_end = '' unless defined $quote_end;
				die "$file:$.: Unmatched quotes in \"$line\"\n" unless $quote_begin eq $quote_end;
				
				if ($quote_begin eq '"') {
					$config{$symbol} = { value => eval('"' . interpolate($2) . '"'), quoted => 1 };
				}
				else {
					$config{$symbol} = { value => $2, quoted => $quote_begin eq '\'' };
				}
			}
		}
		elsif ($line =~ /^\s*\((.*)\)\s?{$/) { # A conditional block
			if (eval(interpolate($1))) {
				push(@condition_stack, 1);
			}
			else {
				push(@condition_stack, 0);
			}
		}
		elsif ($line =~ /^\s*}$/) { # Closing a conditional block
			pop(@condition_stack);
			die "$file:$.: Closing non-existent block\n" unless @condition_stack;
		}
		elsif ($line =~ (/^\s*$/) || ($line =~ /^\s*#/)) { # An empty line or comment
		}
		else {
			die "$file:$.: Malformed line: \"$line\"\n";
		}
	}
}

# Output
for (sort(keys(%config))) {
	my $quote = $config{$_}->{quoted} ? '\'' : '';
	print("$_=$quote$config{$_}->{value}$quote\n") unless $_ =~ /^$symbol_name_prefix_regex/;
}