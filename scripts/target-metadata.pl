#!/usr/bin/env perl
use FindBin;
use lib "$FindBin::Bin";
use strict;
use metadata;
use Getopt::Long;

sub target_config_features(@) {
	my $ret;

	while ($_ = shift @_) {
		/^arm_v(\w+)$/ and $ret .= "\tselect arm_v$1\n";
		/^broken$/ and $ret .= "\tdepends on BROKEN\n";
		/^audio$/ and $ret .= "\tselect AUDIO_SUPPORT\n";
		/^display$/ and $ret .= "\tselect DISPLAY_SUPPORT\n";
		/^dt$/ and $ret .= "\tselect USES_DEVICETREE\n";
		/^gpio$/ and $ret .= "\tselect GPIO_SUPPORT\n";
		/^pci$/ and $ret .= "\tselect PCI_SUPPORT\n";
		/^pcie$/ and $ret .= "\tselect PCIE_SUPPORT\n";
		/^usb$/ and $ret .= "\tselect USB_SUPPORT\n";
		/^usbgadget$/ and $ret .= "\tselect USB_GADGET_SUPPORT\n";
		/^pcmcia$/ and $ret .= "\tselect PCMCIA_SUPPORT\n";
		/^rtc$/ and $ret .= "\tselect RTC_SUPPORT\n";
		/^squashfs$/ and $ret .= "\tselect USES_SQUASHFS\n";
		/^jffs2$/ and $ret .= "\tselect USES_JFFS2\n";
		/^jffs2_nand$/ and $ret .= "\tselect USES_JFFS2_NAND\n";
		/^ext4$/ and $ret .= "\tselect USES_EXT4\n";
		/^targz$/ and $ret .= "\tselect USES_TARGZ\n";
		/^cpiogz$/ and $ret .= "\tselect USES_CPIOGZ\n";
		/^minor$/ and $ret .= "\tselect USES_MINOR\n";
		/^ubifs$/ and $ret .= "\tselect USES_UBIFS\n";
		/^fpu$/ and $ret .= "\tselect HAS_FPU\n";
		/^spe_fpu$/ and $ret .= "\tselect HAS_SPE_FPU\n";
		/^ramdisk$/ and $ret .= "\tselect USES_INITRAMFS\n";
		/^powerpc64$/ and $ret .= "\tselect powerpc64\n";
		/^nommu$/ and $ret .= "\tselect NOMMU\n";
		/^mips16$/ and $ret .= "\tselect HAS_MIPS16\n";
		/^rfkill$/ and $ret .= "\tselect RFKILL_SUPPORT\n";
		/^low_mem$/ and $ret .= "\tselect LOW_MEMORY_FOOTPRINT\n";
		/^small_flash$/ and $ret .= "\tselect SMALL_FLASH\n";
		/^nand$/ and $ret .= "\tselect NAND_SUPPORT\n";
		/^virtio$/ and $ret .= "\tselect VIRTIO_SUPPORT\n";
		/^rootfs-part$/ and $ret .= "\tselect USES_ROOTFS_PART\n";
		/^boot-part$/ and $ret .= "\tselect USES_BOOT_PART\n";
	}
	return $ret;
}

sub target_name($) {
	my $target = shift;
	my $parent = $target->{parent};
	if ($parent) {
		return $target->{parent}->{name}." - ".$target->{name};
	} else {
		return $target->{name};
	}
}

sub kver($) {
	my $v = shift;
	$v =~ tr/\./_/;
	if (substr($v,0,2) eq "2_") {
		$v =~ /(\d+_\d+_\d+)(_\d+)?/ and $v = $1;
	} else {
		$v =~ /(\d+_\d+)(_\d+)?/ and $v = $1;
	}
	return $v;
}

sub print_target($) {
	my $target = shift;
	my $features = target_config_features(@{$target->{features}});
	my $help = $target->{desc};
	my $confstr;

	chomp $features;
	$features .= "\n";
	if ($help =~ /\w+/) {
		$help =~ s/^\s*/\t  /mg;
		$help = "\thelp\n$help";
	} else {
		undef $help;
	}

	my $v = kver($target->{version});
	if (@{$target->{subtargets}} == 0) {
	$confstr = <<EOF;
config TARGET_$target->{conf}
	bool "$target->{name}"
	select LINUX_$v
EOF
	}
	else {
		$confstr = <<EOF;
config TARGET_$target->{conf}
	bool "$target->{name}"
EOF
	}
	if ($target->{subtarget}) {
		$confstr .= "\tdepends on TARGET_$target->{boardconf}\n";
	}
	if (@{$target->{subtargets}} > 0) {
		$confstr .= "\tselect HAS_SUBTARGETS\n";
		grep { /broken/ } @{$target->{features}} and $confstr .= "\tdepends on BROKEN\n";
	} else {
		$confstr .= $features;
		if ($target->{arch} =~ /\w/) {
			$confstr .= "\tselect $target->{arch}\n";
		}
		if ($target->{has_devices}) {
			$confstr .= "\tselect HAS_DEVICES\n";
		}
	}

	foreach my $dep (@{$target->{depends}}) {
		my $mode = "depends on";
		my $flags;
		my $name;

		$dep =~ /^([@\+\-]+)(.+)$/;
		$flags = $1;
		$name = $2;

		next if $name =~ /:/;
		$flags =~ /-/ and $mode = "deselect";
		$flags =~ /\+/ and $mode = "select";
		$flags =~ /@/ and $confstr .= "\t$mode $name\n";
	}
	$confstr .= "$help\n\n";
	print $confstr;
}

sub merge_package_lists($$) {
	my $list1 = shift;
	my $list2 = shift;
	my @l = ();
	my %pkgs;

	foreach my $pkg (@$list1, @$list2) {
		$pkgs{$pkg} = 1;
	}
	foreach my $pkg (keys %pkgs) {
		push @l, $pkg unless ($pkg =~ /^-/ or $pkgs{"-$pkg"});
	}
	return sort(@l);
}

sub gen_target_config() {
	my $file = shift @ARGV;
	my @target = parse_target_metadata($file);
	my %defaults;

	my @target_sort = sort {
		target_name($a) cmp target_name($b);
	} @target;

	foreach my $target (@target_sort) {
		next if @{$target->{subtargets}} > 0;
		print <<EOF;
config DEFAULT_TARGET_$target->{conf}
	bool
	depends on TARGET_PER_DEVICE_ROOTFS
	default y if TARGET_$target->{conf}
EOF
		foreach my $pkg (@{$target->{packages}}) {
			print "\tselect DEFAULT_$pkg if TARGET_PER_DEVICE_ROOTFS\n";
		}
	}

	print <<EOF;
choice
	prompt "Target System"
	default TARGET_x86
	reset if !DEVEL
	
EOF

	foreach my $target (@target_sort) {
		next if $target->{subtarget};
		print_target($target);
	}

	print <<EOF;
endchoice

choice
	prompt "Subtarget" if HAS_SUBTARGETS
EOF
	foreach my $target (@target) {
		next unless $target->{def_subtarget};
		print <<EOF;
	default TARGET_$target->{conf}_$target->{def_subtarget} if TARGET_$target->{conf}
EOF
	}
	print <<EOF;

EOF
	foreach my $target (@target) {
		next unless $target->{subtarget};
		print_target($target);
	}

print <<EOF;
endchoice

choice
	prompt "Target Profile"
	default TARGET_MULTI_PROFILE if BUILDBOT

EOF
	foreach my $target (@target) {
		my $profile = $target->{profiles}->[0];
		$profile or next;
		print <<EOF;
	default TARGET_$target->{conf}_$profile->{id} if TARGET_$target->{conf} && !BUILDBOT
EOF
	}

	print <<EOF;

config TARGET_MULTI_PROFILE
	bool "Multiple devices"
	depends on HAS_DEVICES
	help
	Instead of only building a single image, or all images, this allows you
	to select images to be built for multiple devices in one build.

EOF

	foreach my $target (@target) {
		my $profiles = $target->{profiles};
		foreach my $profile (@{$target->{profiles}}) {
			print <<EOF;
config TARGET_$target->{conf}_$profile->{id}
	bool "$profile->{name}"
	depends on TARGET_$target->{conf}
EOF
			my @pkglist = merge_package_lists($target->{packages}, $profile->{packages});
			foreach my $pkg (@pkglist) {
				print "\tselect DEFAULT_$pkg\n";
				$defaults{$pkg} = 1;
			}
			my $help = $profile->{desc};
			if ($help =~ /\w+/) {
				$help =~ s/^\s*/\t  /mg;
				$help = "\thelp\n$help";
			} else {
				undef $help;
			}
			print "$help\n";
		}
	}

	print <<EOF;
endchoice

menu "Target Devices"
	depends on TARGET_MULTI_PROFILE

	config TARGET_ALL_PROFILES
		bool "Enable all profiles by default"
		default BUILDBOT

	config TARGET_PER_DEVICE_ROOTFS
		bool "Use a per-device root filesystem that adds profile packages"
		default BUILDBOT
		help
		When disabled, all device packages from all selected devices
		will be included in all images by default. (Marked as <*>) You will
		still be able to manually deselect any/all packages.
		When enabled, each device builds it's own image, including only the
		profile packages for that device.  (Marked as {M}) You will be able
		to change a package to included in all images by marking as {*}, but
		will not be able to disable a profile package completely.
		
		To get the most use of this setting, you must set in a .config stub
		before calling "make defconfig".  Selecting TARGET_MULTI_PROFILE and
		then manually selecting (via menuconfig for instance) this option
		will have pre-defaulted all profile packages to included, making this
		option appear to have had no effect.

EOF
	foreach my $target (@target) {
		my @profiles = sort {
			my $x = $a->{name};
			my $y = $b->{name};
			"\L$x" cmp "\L$y";
		} @{$target->{profiles}};
		foreach my $profile (@profiles) {
			next unless $profile->{id} =~ /^DEVICE_/;
			print <<EOF;
menuconfig TARGET_DEVICE_$target->{conf}_$profile->{id}
	bool "$profile->{name}"
	depends on TARGET_$target->{conf}
	default $profile->{default}
EOF
			my @pkglist = merge_package_lists($target->{packages}, $profile->{packages});
			foreach my $pkg (@pkglist) {
				print "\tselect DEFAULT_$pkg if !TARGET_PER_DEVICE_ROOTFS\n";
				print "\tselect MODULE_DEFAULT_$pkg if TARGET_PER_DEVICE_ROOTFS\n";
				$defaults{$pkg} = 1;
			}

			print <<EOF;


	config TARGET_DEVICE_PACKAGES_$target->{conf}_$profile->{id}
		string "$profile->{name} additional packages"
		default ""
		depends on TARGET_PER_DEVICE_ROOTFS
		depends on TARGET_DEVICE_$target->{conf}_$profile->{id}

EOF
		}
	}

	print <<EOF;

endmenu

config HAS_SUBTARGETS
	bool

config HAS_DEVICES
	bool

config TARGET_BOARD
	string

EOF
	foreach my $target (@target) {
		$target->{subtarget} or	print "\t\tdefault \"".$target->{board}."\" if TARGET_".$target->{conf}."\n";
	}
	print <<EOF;
config TARGET_SUBTARGET
	string
	default "generic" if !HAS_SUBTARGETS

EOF

	foreach my $target (@target) {
		foreach my $subtarget (@{$target->{subtargets}}) {
			print "\t\tdefault \"$subtarget\" if TARGET_".$target->{conf}."_$subtarget\n";
		}
	}
	print <<EOF;
config TARGET_PROFILE
	string
EOF
	foreach my $target (@target) {
		my $profiles = $target->{profiles};
		foreach my $profile (@$profiles) {
			print "\tdefault \"$profile->{id}\" if TARGET_$target->{conf}_$profile->{id}\n";
		}
	}

	print <<EOF;

config TARGET_ARCH_PACKAGES
	string
	
EOF
	foreach my $target (@target) {
		next if @{$target->{subtargets}} > 0;
		print "\t\tdefault \"".($target->{arch_packages} || $target->{board})."\" if TARGET_".$target->{conf}."\n";
	}
	print <<EOF;

config DEFAULT_TARGET_OPTIMIZATION
	string
EOF
	foreach my $target (@target) {
		next if @{$target->{subtargets}} > 0;
		print "\tdefault \"".$target->{cflags}."\" if TARGET_".$target->{conf}."\n";
	}
	print "\tdefault \"-Os -pipe -funit-at-a-time\"\n";
	print <<EOF;

config CPU_TYPE
	string
EOF
	foreach my $target (@target) {
		next if @{$target->{subtargets}} > 0;
		print "\tdefault \"".$target->{cputype}."\" if TARGET_".$target->{conf}."\n";
	}
	print "\tdefault \"\"\n";

	my %kver;
	foreach my $target (@target) {
		my $v = kver($target->{version});
		next if $kver{$v};
		$kver{$v} = 1;
		print <<EOF;

config LINUX_$v
	bool

EOF
	}
	foreach my $def (sort keys %defaults) {
		print <<EOF;
	config DEFAULT_$def
		bool

	config MODULE_DEFAULT_$def
		tristate
		depends on TARGET_PER_DEVICE_ROOTFS
		depends on m
		default m if DEFAULT_$def
		select PACKAGE_$def

EOF
	}
}

sub gen_profile_mk() {
	my $file = shift @ARGV;
	my $target = shift @ARGV;
	my @targets = parse_target_metadata($file);
	foreach my $cur (@targets) {
		next unless $cur->{id} eq $target;
		print "PROFILE_NAMES = ".join(" ", map { $_->{id} } @{$cur->{profiles}})."\n";
		foreach my $profile (@{$cur->{profiles}}) {
			print $profile->{id}.'_NAME:='.$profile->{name}."\n";
			print $profile->{id}.'_PACKAGES:='.join(' ', @{$profile->{packages}})."\n";
		}
	}
}

sub parse_command() {
	GetOptions("ignore=s", \@ignore);
	my $cmd = shift @ARGV;
	for ($cmd) {
		/^config$/ and return gen_target_config();
		/^profile_mk$/ and return gen_profile_mk();
	}
	die <<EOF
Available Commands:
	$0 config [file] 			Target metadata in Kconfig format
	$0 profile_mk [file] [target]		Profile metadata in makefile format

EOF
}

parse_command();
