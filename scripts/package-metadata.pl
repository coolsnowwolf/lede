#!/usr/bin/env perl
use FindBin;
use lib "$FindBin::Bin";
use strict;
use metadata;
use Getopt::Long;

my %board;

sub version_to_num($) {
	my $str = shift;
	my $num = 0;

	if (defined($str) && $str =~ /^\d+(?:\.\d+)+$/)
	{
		my @n = (split(/\./, $str), 0, 0, 0, 0);
		$num = ($n[0] << 24) | ($n[1] << 16) | ($n[2] << 8) | $n[3];
	}

	return $num;
}

sub version_filter_list(@) {
	my $cmpver = version_to_num(shift @_);
	my @items;

	foreach my $item (@_)
	{
		if ($item =~ s/@(lt|le|gt|ge|eq|ne)(\d+(?:\.\d+)+)\b//)
		{
			my $op = $1;
			my $symver = version_to_num($2);

			if ($symver > 0 && $cmpver > 0)
			{
				next unless (($op eq 'lt' && $cmpver <  $symver) ||
				             ($op eq 'le' && $cmpver <= $symver) ||
				             ($op eq 'gt' && $cmpver >  $symver) ||
				             ($op eq 'ge' && $cmpver >= $symver) ||
				             ($op eq 'eq' && $cmpver == $symver) ||
				             ($op eq 'ne' && $cmpver != $symver));
			}
		}

		push @items, $item;
	}

	return @items;
}

sub gen_kconfig_overrides() {
	my %config;
	my %kconfig;
	my $package;
	my $pkginfo = shift @ARGV;
	my $cfgfile = shift @ARGV;
	my $patchver = shift @ARGV;

	# parameter 2: build system config
	open FILE, "<$cfgfile" or return;
	while (<FILE>) {
		/^(CONFIG_.+?)=(.+)$/ and $config{$1} = 1;
	}
	close FILE;

	# parameter 1: package metadata
	open FILE, "<$pkginfo" or return;
	while (<FILE>) {
		/^Package:\s*(.+?)\s*$/ and $package = $1;
		/^Kernel-Config:\s*(.+?)\s*$/ and do {
			my @config = split /\s+/, $1;
			foreach my $config (version_filter_list($patchver, @config)) {
				my $val = 'm';
				my $override;
				if ($config =~ /^(.+?)=(.+)$/) {
					$config = $1;
					$override = 1;
					$val = $2;
				}
				if ($config{"CONFIG_PACKAGE_$package"} and ($config ne 'n')) {
					next if $kconfig{$config} eq 'y';
					$kconfig{$config} = $val;
				} elsif (!$override) {
					$kconfig{$config} or $kconfig{$config} = 'n';
				}
			}
		};
	};
	close FILE;

	foreach my $kconfig (sort keys %kconfig) {
		if ($kconfig{$kconfig} eq 'n') {
			print "# $kconfig is not set\n";
		} else {
			print "$kconfig=$kconfig{$kconfig}\n";
		}
	}
}

my %dep_check;
sub __find_package_dep($$) {
	my $pkg = shift;
	my $name = shift;
	my $deps = $pkg->{depends};

	return 0 unless defined $deps;
	foreach my $vpkg (@{$deps}) {
		foreach my $dep (@{$vpackage{$vpkg}}) {
			next if $dep_check{$dep->{name}};
			$dep_check{$dep->{name}} = 1;
			return 1 if $dep->{name} eq $name;
			return 1 if (__find_package_dep($dep, $name) == 1);
		}
	}
	return 0;
}

# wrapper to avoid infinite recursion
sub find_package_dep($$) {
	my $pkg = shift;
	my $name = shift;

	%dep_check = ();
	return __find_package_dep($pkg, $name);
}

sub package_depends($$) {
	my $a = shift;
	my $b = shift;
	my $ret;

	return 0 if ($a->{submenu} ne $b->{submenu});
	if (find_package_dep($a, $b->{name}) == 1) {
		$ret = 1;
	} elsif (find_package_dep($b, $a->{name}) == 1) {
		$ret = -1;
	} else {
		return 0;
	}
	return $ret;
}

sub mconf_depends {
	my $pkgname = shift;
	my $depends = shift;
	my $only_dep = shift;
	my $res;
	my $dep = shift;
	my $seen = shift;
	my $parent_condition = shift;
	$dep or $dep = {};
	$seen or $seen = {};
	my @t_depends;

	$depends or return;
	my @depends = @$depends;
	foreach my $depend (@depends) {
		my $m = "depends on";
		my $flags = "";
		$depend =~ s/^([@\+]+)// and $flags = $1;
		my $condition = $parent_condition;

		next if $condition eq $depend;
		next if $seen->{"$parent_condition:$depend"};
		next if $seen->{":$depend"};
		$seen->{"$parent_condition:$depend"} = 1;
		if ($depend =~ /^(.+):(.+)$/) {
			if ($1 ne "PACKAGE_$pkgname") {
				if ($condition) {
					$condition = "$condition && $1";
				} else {
					$condition = $1;
				}
			}
			$depend = $2;
		}
		if ($flags =~ /\+/) {
			my $vdep = $vpackage{$depend};
			if ($vdep) {
				my @vdeps;

				foreach my $v (@$vdep) {
					next if $v->{buildonly};
					if ($v->{variant_default}) {
						unshift @vdeps, $v->{name};
					} else {
						push @vdeps, $v->{name};
					}
				}

				$depend = shift @vdeps;

				if (@vdeps > 1) {
					$condition = ($condition ? "$condition && " : '') . join("&&", map { "PACKAGE_$_<PACKAGE_$pkgname" } @vdeps);
				} elsif (@vdeps > 0) {
					$condition = ($condition ? "$condition && " : '') . "PACKAGE_${vdeps[0]}<PACKAGE_$pkgname";
				}
			}

			# Menuconfig will not treat 'select FOO' as a real dependency
			# thus if FOO depends on other config options, these dependencies
			# will not be checked. To fix this, we simply emit all of FOO's
			# depends here as well.
			$package{$depend} and push @t_depends, [ $package{$depend}->{depends}, $condition ];

			$m = "select";
			next if $only_dep;

			$flags =~ /@/ or $depend = "PACKAGE_$depend";
		} else {
			my $vdep = $vpackage{$depend};
			if ($vdep && @$vdep > 0) {
				$depend = join("||", map { "PACKAGE_".$_->{name} } @$vdep);
			} else {
				$flags =~ /@/ or $depend = "PACKAGE_$depend";
			}
		}

		if ($condition) {
			if ($m =~ /select/) {
				next if $depend eq $condition;
				$depend = "$depend if $condition";
			} else {
				next if $dep->{"$depend if $condition"};
				$depend = "!($condition) || $depend" unless $dep->{$condition} eq 'select';
			}
		}
		$dep->{$depend} =~ /select/ or $dep->{$depend} = $m;
	}

	foreach my $tdep (@t_depends) {
		mconf_depends($pkgname, $tdep->[0], 1, $dep, $seen, $tdep->[1]);
	}

	foreach my $depend (keys %$dep) {
		my $m = $dep->{$depend};
		$res .= "\t\t$m $depend\n";
	}
	return $res;
}

sub mconf_conflicts {
	my $pkgname = shift;
	my $depends = shift;
	my $res = "";

	foreach my $depend (@$depends) {
		next unless $package{$depend};
		$res .= "\t\tdepends on m || (PACKAGE_$depend != y)\n";
	}
	return $res;
}

sub print_package_config_category($) {
	my $cat = shift;
	my %menus;
	my %menu_dep;

	return unless $category{$cat};

	print "menu \"$cat\"\n\n";
	my %spkg = %{$category{$cat}};

	foreach my $spkg (sort {uc($a) cmp uc($b)} keys %spkg) {
		foreach my $pkg (@{$spkg{$spkg}}) {
			next if $pkg->{buildonly};
			my $menu = $pkg->{submenu};
			if ($menu) {
				$menu_dep{$menu} or $menu_dep{$menu} = $pkg->{submenudep};
			} else {
				$menu = 'undef';
			}
			$menus{$menu} or $menus{$menu} = [];
			push @{$menus{$menu}}, $pkg;
		}
	}
	my @menus = sort {
		($a eq 'undef' ?  1 : 0) or
		($b eq 'undef' ? -1 : 0) or
		($a cmp $b)
	} keys %menus;

	foreach my $menu (@menus) {
		my @pkgs = sort {
			package_depends($a, $b) or
			($a->{name} cmp $b->{name})
		} @{$menus{$menu}};
		if ($menu ne 'undef') {
			$menu_dep{$menu} and print "if $menu_dep{$menu}\n";
			print "menu \"$menu\"\n";
		}
		foreach my $pkg (@pkgs) {
			next if $pkg->{src}{ignore};
			my $title = $pkg->{name};
			my $c = (72 - length($pkg->{name}) - length($pkg->{title}));
			if ($c > 0) {
				$title .= ("." x $c). " ". $pkg->{title};
			}
			$title = "\"$title\"";
			print "\t";
			$pkg->{menu} and print "menu";
			print "config PACKAGE_".$pkg->{name}."\n";
			$pkg->{hidden} and $title = "";
			print "\t\t".($pkg->{tristate} ? 'tristate' : 'bool')." $title\n";
			print "\t\tdefault y if DEFAULT_".$pkg->{name}."\n";
			unless ($pkg->{hidden}) {
				my @def = ("ALL");
				if (!exists($pkg->{repository})) {
					push @def, "ALL_NONSHARED";
				}
				if ($pkg->{name} =~ /^kmod-/) {
					push @def, "ALL_KMODS";
				}
				$pkg->{default} ||= "m if " . join("||", @def);
			}
			if ($pkg->{default}) {
				foreach my $default (split /\s*,\s*/, $pkg->{default}) {
					print "\t\tdefault $default\n";
				}
			}
			print mconf_depends($pkg->{name}, $pkg->{depends}, 0);
			print mconf_depends($pkg->{name}, $pkg->{mdepends}, 0);
			print mconf_conflicts($pkg->{name}, $pkg->{conflicts});
			print "\t\thelp\n";
			print $pkg->{description};
			print "\n";

			$pkg->{config} and print $pkg->{config}."\n";
		}
		if ($menu ne 'undef') {
			print "endmenu\n";
			$menu_dep{$menu} and print "endif\n";
		}
	}
	print "endmenu\n\n";

	undef $category{$cat};
}

sub print_package_overrides() {
	keys %overrides > 0 or return;
	print "\tconfig OVERRIDE_PKGS\n";
	print "\t\tstring\n";
	print "\t\tdefault \"".join(" ", sort keys %overrides)."\"\n\n";
}

sub gen_package_config() {
	parse_package_metadata($ARGV[0]) or exit 1;
	print "menuconfig IMAGEOPT\n\tbool \"Image configuration\"\n\tdefault n\n";
	print "source \"package/*/image-config.in\"\n";
	if (scalar glob "package/feeds/*/*/image-config.in") {
	    print "source \"package/feeds/*/*/image-config.in\"\n";
	}
	print_package_config_category 'Base system';
	foreach my $cat (sort {uc($a) cmp uc($b)} keys %category) {
		print_package_config_category $cat;
	}
	print_package_overrides();
}

sub and_condition($) {
	my $condition = shift;
	my @spl_and = split('\&\&', $condition);
	if (@spl_and == 1) {
		return "\$(CONFIG_$spl_and[0])";
	}
	return "\$(and " . join (',', map("\$(CONFIG_$_)", @spl_and)) . ")";
}

sub gen_condition ($) {
	my $condition = shift;
	# remove '!()', just as include/package-ipkg.mk does
	$condition =~ s/[()!]//g;
	return join("", map(and_condition($_), split('\|\|', $condition)));
}

sub get_conditional_dep($$) {
	my $condition = shift;
	my $depstr = shift;
	if ($condition) {
		if ($condition =~ /^!(.+)/) {
			return "\$(if " . gen_condition($1) . ",,$depstr)";
		} else {
			return "\$(if " . gen_condition($condition) . ",$depstr)";
		}
	} else {
		return $depstr;
	}
}

sub gen_package_mk() {
	my $line;

	parse_package_metadata($ARGV[0]) or exit 1;
	foreach my $srcname (sort {uc($a) cmp uc($b)} keys %srcpackage) {
		my $src = $srcpackage{$srcname};
		my $variant_default;
		my %deplines = ('' => {});

		foreach my $pkg (@{$src->{packages}}) {
			foreach my $dep (@{$pkg->{depends}}) {
				next if ($dep =~ /@/);

				my $condition;

				$dep =~ s/\+//g;
				if ($dep =~ /^(.+):(.+)/) {
					$condition = $1;
					$dep = $2;
				}

				my $vpkg_dep = $vpackage{$dep};
				unless (defined $vpkg_dep) {
					warn sprintf "WARNING: Makefile '%s' has a dependency on '%s', which does not exist\n",
						$src->{makefile}, $dep;
					next;
				}

				# Filter out self-depends
				my @vdeps = grep { $srcname ne $_->{src}{name} } @{$vpkg_dep};

				foreach my $vdep (@vdeps) {
					my $depstr = sprintf '$(curdir)/%s/compile', $vdep->{src}{path};
					if (@vdeps > 1) {
						$depstr = sprintf '$(if $(CONFIG_PACKAGE_%s),%s)', $vdep->{name}, $depstr;
					}
					my $depline = get_conditional_dep($condition, $depstr);
					if ($depline) {
						$deplines{''}{$depline}++;
					}
				}
			}

			my $config = '';
			$config = sprintf '$(CONFIG_PACKAGE_%s)', $pkg->{name} unless $pkg->{buildonly};

			$pkg->{prereq} and printf "prereq-%s += %s\n", $config, $src->{path};

			next if $pkg->{buildonly};

			printf "package-%s += %s\n", $config, $src->{path};

			if ($pkg->{variant}) {
				if (!defined($variant_default) or $pkg->{variant_default}) {
					$variant_default = $pkg->{variant};
				}
				printf "\$(curdir)/%s/variants += \$(if %s,%s)\n", $src->{path}, $config, $pkg->{variant};
			}
		}

		if (defined($variant_default)) {
			printf "\$(curdir)/%s/default-variant := %s\n", $src->{path}, $variant_default;
		}

		unless (grep {!$_->{buildonly}} @{$src->{packages}}) {
			printf "package- += %s\n", $src->{path};
		}

		if (@{$src->{buildtypes}} > 0) {
			printf "buildtypes-%s = %s\n", $src->{path}, join(' ', @{$src->{buildtypes}});
		}

		foreach my $type ('', @{$src->{buildtypes}}) {
			my $suffix = '';

			$suffix = "/$type" if $type;

			next unless $src->{"builddepends$suffix"};

			defined $deplines{$suffix} or $deplines{$suffix} = {};

			foreach my $dep (@{$src->{"builddepends$suffix"}}) {
				my $depsuffix = "";
				my $deptype = "";
				my $condition;

				if ($dep =~ /^(.+):(.+)/) {
					$condition = $1;
					$dep = $2;
				}
				if ($dep =~ /^(.+)\/(.+)/) {
					$dep = $1;
					$deptype = $2;
					$depsuffix = "/$2";
				}

				next if $srcname.$suffix eq $dep.$depsuffix;

				my $src_dep = $srcpackage{$dep};
				unless (defined($src_dep) && (!$deptype || grep { $_ eq $deptype } @{$src_dep->{buildtypes}})) {
					warn sprintf "WARNING: Makefile '%s' has a build dependency on '%s', which does not exist\n",
						$src->{makefile}, $dep.$depsuffix;
					next;
				}

				my $depstr = sprintf '$(curdir)/%s/compile', $src_dep->{path}.$depsuffix;
				my $depline = get_conditional_dep($condition, $depstr);
				if ($depline) {
					$deplines{$suffix}{$depline}++;
				}
			}
		}

		foreach my $suffix (sort keys %deplines) {
			my $depline = join(" ", sort keys %{$deplines{$suffix}});
			if ($depline) {
				$line .= sprintf "\$(curdir)/%s/compile += %s\n", $src->{path}.$suffix, $depline;
			}
		}
	}

	if ($line ne "") {
		print "\n$line";
	}
}

sub gen_package_source() {
	parse_package_metadata($ARGV[0]) or exit 1;
	foreach my $name (sort {uc($a) cmp uc($b)} keys %package) {
		my $pkg = $package{$name};
		if ($pkg->{name} && $pkg->{source}) {
			print "$pkg->{name}: ";
			print "$pkg->{source}\n";
		}
	}
}

sub gen_package_auxiliary() {
	parse_package_metadata($ARGV[0]) or exit 1;
	foreach my $name (sort {uc($a) cmp uc($b)} keys %package) {
		my $pkg = $package{$name};
		if ($pkg->{name} && $pkg->{repository}) {
			print "Package/$name/subdir = $pkg->{repository}\n";
		}
		my %depends;
		foreach my $dep (@{$pkg->{depends} || []}) {
			if ($dep =~ m!^\+?(?:[^:]+:)?([^@]+)$!) {
				$depends{$1}++;
			}
		}
		my @depends = sort keys %depends;
		if (@depends > 0) {
			foreach my $n (@{$pkg->{provides}}) {
				print "Package/$n/depends = @depends\n";
			}
		}
	}
}

sub gen_package_license($) {
	my $level = shift;
	parse_package_metadata($ARGV[0]) or exit 1;
	foreach my $name (sort {uc($a) cmp uc($b)} keys %package) {
		my $pkg = $package{$name};
		if ($pkg->{name}) {
			if ($pkg->{license}) {
				print "$pkg->{name}: ";
				print "$pkg->{license}\n";
				if ($pkg->{licensefiles} && $level == 0) {
					print "\tFiles: $pkg->{licensefiles}\n";
				}
			} else {
				if ($level == 1) {
					print "$pkg->{name}: Missing license! ";
					print "Please fix $pkg->{src}{makefile}\n";
				}
			}
		}
	}
}

sub gen_version_filtered_list() {
	foreach my $item (version_filter_list(@ARGV)) {
		print "$item\n";
	}
}

sub gen_usergroup_list() {
	parse_package_metadata($ARGV[0]) or exit 1;
	for my $name (keys %usernames) {
		print "user $name $usernames{$name}{id} $usernames{$name}{makefile}\n";
	}
	for my $name (keys %groupnames) {
		print "group $name $groupnames{$name}{id} $groupnames{$name}{makefile}\n";
	}
}

sub parse_command() {
	GetOptions("ignore=s", \@ignore);
	my $cmd = shift @ARGV;
	for ($cmd) {
		/^mk$/ and return gen_package_mk();
		/^config$/ and return gen_package_config();
		/^kconfig/ and return gen_kconfig_overrides();
		/^source$/ and return gen_package_source();
		/^pkgaux$/ and return gen_package_auxiliary();
		/^license$/ and return gen_package_license(0);
		/^licensefull$/ and return gen_package_license(1);
		/^usergroup$/ and return gen_usergroup_list();
		/^version_filter$/ and return gen_version_filtered_list();
	}
	die <<EOF
Available Commands:
	$0 mk [file]				Package metadata in makefile format
	$0 config [file] 			Package metadata in Kconfig format
	$0 kconfig [file] [config] [patchver]	Kernel config overrides
	$0 source [file] 			Package source file information
	$0 pkgaux [file]			Package auxiliary variables in makefile format
	$0 license [file] 			Package license information
	$0 licensefull [file] 			Package license information (full list)
	$0 usergroup [file]			Package usergroup allocation list
	$0 version_filter [patchver] [list...]	Filter list of version tagged strings

Options:
	--ignore <name>				Ignore the source package <name>
EOF
}

parse_command();
