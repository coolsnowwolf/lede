package metadata;
use base 'Exporter';
use strict;
use warnings;
our @EXPORT = qw(%package %vpackage %srcpackage %category %overrides clear_packages parse_package_metadata parse_target_metadata get_multiline @ignore %usernames %groupnames);

our %package;
our %vpackage;
our %srcpackage;
our %category;
our %overrides;
our @ignore;

our %usernames;
our %groupnames;
our %userids;
our %groupids;

sub get_multiline {
	my $fh = shift;
	my $prefix = shift;
	my $str;
	while (<$fh>) {
		last if /^@@/;
		$str .= (($_ and $prefix) ? $prefix . $_ : $_);
	}

	return $str ? $str : "";
}

sub confstr($) {
	my $conf = shift;
	$conf =~ tr#/\.\-/#___#;
	return $conf;
}

sub parse_package_metadata_usergroup($$$$$) {
	my $makefile = shift;
	my $typename = shift;
	my $names = shift;
	my $ids = shift;
	my $spec = shift;
	my $name;
	my $id;

	# the regex for name is taken from is_valid_name() of package shadow
	if ($spec =~ /^([a-z_][a-z0-9_-]*\$?)$/) {
		$name = $spec;
		$id = -1;
	} elsif ($spec =~ /^([a-z_][a-z0-9_-]*\$?)=(\d+)$/) {
		$name = $1;
		$id = $2;
	} else {
		warn "$makefile: invalid $typename spec $spec\n";
		return 0;
	}

	if ($id =~ /^[1-9]\d*$/) {
		if ($id >= 65536) {
			warn "$makefile: $typename $name id $id >= 65536";
			return 0;
		}
		if (not exists $ids->{$id}) {
			$ids->{$id} = {
				name => $name,
				makefile => $makefile,
			};
		} elsif ($ids->{$id}{name} ne $name) {
			warn "$makefile: $typename $name id $id is already taken by $ids->{$id}{makefile}\n";
			return 0;
		}
	} elsif ($id != -1) {
		warn "$makefile: $typename $name has invalid id $id\n";
		return 0;
	}

	if (not exists $names->{$name}) {
		$names->{$name} = {
			id => $id,
			makefile => $makefile,
		};
	} elsif ($names->{$name}{id} != $id) {
		warn "$makefile: id of $typename $name collides with that defined defined in $names->{$name}{makefile}\n";
		return 0;
	}
	return 1;
}

sub parse_target_metadata($) {
	my $file = shift;
	my ($target, @target, $profile);
	my %target;
	my $makefile;

	open FILE, "<$file" or do {
		warn "Can't open file '$file': $!\n";
		return;
	};
	while (<FILE>) {
		chomp;
		/^Source-Makefile: \s*((.+\/)([^\/]+)\/Makefile)\s*$/ and $makefile = $1;
		/^Target:\s*(.+)\s*$/ and do {
			my $name = $1;
			$target = {
				id => $name,
				board => $name,
				makefile => $makefile,
				boardconf => confstr($name),
				conf => confstr($name),
				profiles => [],
				features => [],
				depends => [],
				subtargets => []
			};
			push @target, $target;
			$target{$name} = $target;
			if ($name =~ /([^\/]+)\/([^\/]+)/) {
				push @{$target{$1}->{subtargets}}, $2;
				$target->{board} = $1;
				$target->{boardconf} = confstr($1);
				$target->{subtarget} = 1;
				$target->{parent} = $target{$1};
			}
		};
		/^Target-Name:\s*(.+)\s*$/ and $target->{name} = $1;
		/^Target-Arch:\s*(.+)\s*$/ and $target->{arch} = $1;
		/^Target-Arch-Packages:\s*(.+)\s*$/ and $target->{arch_packages} = $1;
		/^Target-Features:\s*(.+)\s*$/ and $target->{features} = [ split(/\s+/, $1) ];
		/^Target-Depends:\s*(.+)\s*$/ and $target->{depends} = [ split(/\s+/, $1) ];
		/^Target-Description:/ and $target->{desc} = get_multiline(*FILE);
		/^Target-Optimization:\s*(.+)\s*$/ and $target->{cflags} = $1;
		/^CPU-Type:\s*(.+)\s*$/ and $target->{cputype} = $1;
		/^Linux-Version:\s*(.+)\s*$/ and $target->{version} = $1;
		/^Linux-Release:\s*(.+)\s*$/ and $target->{release} = $1;
		/^Linux-Kernel-Arch:\s*(.+)\s*$/ and $target->{karch} = $1;
		/^Default-Subtarget:\s*(.+)\s*$/ and $target->{def_subtarget} = $1;
		/^Default-Packages:\s*(.+)\s*$/ and $target->{packages} = [ split(/\s+/, $1) ];
		/^Target-Profile:\s*(.+)\s*$/ and do {
			$profile = {
				id => $1,
				name => $1,
				priority => 999,
				packages => []
			};
			$1 =~ /^DEVICE_/ and $target->{has_devices} = 1;
			push @{$target->{profiles}}, $profile;
		};
		/^Target-Profile-Name:\s*(.+)\s*$/ and $profile->{name} = $1;
		/^Target-Profile-Priority:\s*(\d+)\s*$/ and do {
			$profile->{priority} = $1;
			$target->{sort} = 1;
		};
		/^Target-Profile-Packages:\s*(.*)\s*$/ and $profile->{packages} = [ split(/\s+/, $1) ];
		/^Target-Profile-Description:\s*(.*)\s*/ and $profile->{desc} = get_multiline(*FILE);
	}
	close FILE;
	foreach my $target (@target) {
		if (@{$target->{subtargets}} > 0) {
			$target->{profiles} = [];
			next;
		}
		@{$target->{profiles}} > 0 or $target->{profiles} = [
			{
				id => 'Default',
				name => 'Default',
				packages => []
			}
		];

		$target->{sort} and @{$target->{profiles}} = sort {
			$a->{priority} <=> $b->{priority} or
			$a->{name} cmp $b->{name};
		} @{$target->{profiles}};
	}
	return @target;
}

sub clear_packages() {
	%package = ();
	%vpackage = ();
	%srcpackage = ();
	%category = ();
	%overrides = ();
	%usernames = ();
	%groupnames = ();
}

sub parse_package_metadata($) {
	my $file = shift;
	my $pkg;
	my $src;
	my $override;
	my %ignore = map { $_ => 1 } @ignore;

	open FILE, "<$file" or do {
		warn "Cannot open '$file': $!\n";
		return undef;
	};
	while (<FILE>) {
		chomp;
		/^Source-Makefile: \s*((?:package\/)?((?:.+\/)?([^\/]+))\/Makefile)\s*$/ and do {
			$src = {
				makefile => $1,
				path => $2,
				name => $3,
				ignore => $ignore{$3},
				packages => [],
				buildtypes => [],
				builddepends => [],
			};
			$srcpackage{$3} = $src;
			$override = "";
			undef $pkg;
		};
		/^Override: \s*(.+?)\s*$/ and do {
			$override = $1;
			$overrides{$src->{name}} = 1;
		};
		next unless $src;
		/^Package:\s*(.+?)\s*$/ and do {
			$pkg = {};
			$pkg->{src} = $src;
			$pkg->{name} = $1;
			$pkg->{title} = "";
			$pkg->{depends} = [];
			$pkg->{mdepends} = [];
			$pkg->{provides} = [$1];
			$pkg->{tristate} = 1;
			$pkg->{override} = $override;
			$package{$1} = $pkg;
			push @{$src->{packages}}, $pkg;

			$vpackage{$1} or $vpackage{$1} = [];
			unshift @{$vpackage{$1}}, $pkg;
		};
		/^Build-Depends: \s*(.+)\s*$/ and $src->{builddepends} = [ split /\s+/, $1 ];
		/^Build-Depends\/(\w+): \s*(.+)\s*$/ and $src->{"builddepends/$1"} = [ split /\s+/, $2 ];
		/^Build-Types:\s*(.+)\s*$/ and $src->{buildtypes} = [ split /\s+/, $1 ];
		next unless $pkg;
		/^Version: \s*(.+)\s*$/ and $pkg->{version} = $1;
		/^Title: \s*(.+)\s*$/ and $pkg->{title} = $1;
		/^Menu: \s*(.+)\s*$/ and $pkg->{menu} = $1;
		/^Submenu: \s*(.+)\s*$/ and $pkg->{submenu} = $1;
		/^Submenu-Depends: \s*(.+)\s*$/ and $pkg->{submenudep} = $1;
		/^Source: \s*(.+)\s*$/ and $pkg->{source} = $1;
		/^License: \s*(.+)\s*$/ and $pkg->{license} = $1;
		/^LicenseFiles: \s*(.+)\s*$/ and $pkg->{licensefiles} = $1;
		/^Default: \s*(.+)\s*$/ and $pkg->{default} = $1;
		/^Provides: \s*(.+)\s*$/ and do {
			my @vpkg = split /\s+/, $1;
			@{$pkg->{provides}} = ($pkg->{name}, @vpkg);
			foreach my $vpkg (@vpkg) {
				$vpackage{$vpkg} or $vpackage{$vpkg} = [];
				push @{$vpackage{$vpkg}}, $pkg;
			}
		};
		/^Menu-Depends: \s*(.+)\s*$/ and $pkg->{mdepends} = [ split /\s+/, $1 ];
		/^Depends: \s*(.+)\s*$/ and $pkg->{depends} = [ split /\s+/, $1 ];
		/^Conflicts: \s*(.+)\s*$/ and $pkg->{conflicts} = [ split /\s+/, $1 ];
		/^Hidden: \s*(.+)\s*$/ and $pkg->{hidden} = 1;
		/^Build-Variant: \s*([\w\-]+)\s*/ and $pkg->{variant} = $1;
		/^Default-Variant: .*/ and $pkg->{variant_default} = 1;
		/^Build-Only: \s*(.+)\s*$/ and $pkg->{buildonly} = 1;
		/^Repository:\s*(.+?)\s*$/ and $pkg->{repository} = $1;
		/^Category: \s*(.+)\s*$/ and do {
			$pkg->{category} = $1;
			defined $category{$1} or $category{$1} = {};
			defined $category{$1}{$src->{name}} or $category{$1}{$src->{name}} = [];
			push @{$category{$1}{$src->{name}}}, $pkg;
		};
		/^Description: \s*(.*)\s*$/ and $pkg->{description} = "\t\t $1\n". get_multiline(*FILE, "\t\t ");
		/^Type: \s*(.+)\s*$/ and do {
			$pkg->{type} = [ split /\s+/, $1 ];
			undef $pkg->{tristate};
			foreach my $type (@{$pkg->{type}}) {
				$type =~ /ipkg/ and $pkg->{tristate} = 1;
			}
		};
		/^Config:\s*(.*)\s*$/ and $pkg->{config} = "$1\n".get_multiline(*FILE, "\t");
		/^Prereq-Check:/ and $pkg->{prereq} = 1;
		/^Require-User:\s*(.*?)\s*$/ and do {
			my @ugspecs = split /\s+/, $1;

			for my $ugspec (@ugspecs) {
				my @ugspec = split /:/, $ugspec, 2;
				if ($ugspec[0]) {
					parse_package_metadata_usergroup($src->{makefile}, "user", \%usernames, \%userids, $ugspec[0]) or return 0;
				}
				if ($ugspec[1]) {
					parse_package_metadata_usergroup($src->{makefile}, "group", \%groupnames, \%groupids, $ugspec[1]) or return 0;
				}
			}
		};
	}
	close FILE;
	return 1;
}

1;
