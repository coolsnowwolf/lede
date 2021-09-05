#!/usr/bin/perl

@ARGV <= 2 || die "Usage: $0 [<po directory>] [<file pattern>]\n";

my $source  = shift @ARGV;
my $pattern = shift @ARGV || '*.po';

sub read_header
{
	my $file = shift || return;
	local $/;

	open P, "< $file" || die "open(): $!";
	my $data = readline P;
	close P;

	$data =~ /
		^ (
		msgid \s "" \n
		msgstr \s "" \n
		(?: " [^\n]+ " \n )+
		\n )
	/mx;

	return $1;
}

sub write_header
{
	my $file = shift || return;
	my $head = shift || return;
	local $/;

	open P, "< $file" || die "open(): $!";
	my $data = readline P;
	close P;

	$data =~ s/
		^ (
		msgid \s "" \n
		msgstr \s "" \n
		(?: " [^\n]+ " \n )+
		\n )
	/$head/mx;

	open P, "> $file" || die "open(): $!";
	print P $data;
	close P;
}

my @dirs;

if( ! $source )
{
	@dirs = glob("./*/*/po/");
}
else
{
	@dirs = ( $source );
}

foreach my $dir (@dirs)
{
	if( open F, "find $dir -type f -name '$pattern' |" )
	{
		while( chomp( my $file = readline F ) )
		{
			my ( $basename ) = $file =~ m{.+/([^/]+)\.po$};
		
			if( -f "$dir/templates/$basename.pot" )
			{
				my $head = read_header($file);

				printf "Updating %-40s", $file;
				system("msgmerge", "-U", "-N", $file, "$dir/templates/$basename.pot");

				write_header($file, $head);
			}
		}

		close F;
	}
}
