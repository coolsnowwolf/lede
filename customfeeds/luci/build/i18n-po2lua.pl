#!/usr/bin/perl

@ARGV == 2 || die "Usage: $0 <source-dir> <dest-dir>\n";

my $source_dir  = shift @ARGV;
my $target_dir  = shift @ARGV;

if( ! -d $target_dir )
{
	system('mkdir', '-p', $target_dir);
}

if( open F, "find $source_dir -type f -name '*.po' |" )
{
	while( chomp( my $file = readline F ) )
	{
		my ( $lang, $basename ) = $file =~ m{.+/(\w+)/([^/]+)\.po$};
		$lang = lc $lang;
		$lang =~ s/_/-/g;

		printf "Generating %-40s ", "$target_dir/$basename.$lang.lmo";
		system("./build/po2lmo", $file, "$target_dir/$basename.$lang.lmo");
		print ( -f "$target_dir/$basename.$lang.lmo" ? "done\n" : "empty\n" );
	}

	close F;
}
