ifExist0=`cat "$1"/target/linux/generic/config-4.14 | grep "CONFIG_IMQ_NUM_DEVS"`
cp -f "$1"/package/gargoyle/netfilter-match-modules/patches/690-imq.patch "$1"/target/linux/generic/hack-4.14/690-imq.patch
cp -f "$1"/package/gargoyle/netfilter-match-modules/patches/300-imq.patch "$1"/package/network/utils/iptables/patches/300-imq.patch
if [ -z "ifExist0" ]; then
	echo "# CONFIG_ZRAM is not set" >> "$1"/target/linux/generic/config-4.14
	echo "# CONFIG_ZSMALLOC is not set" >> "$1"/target/linux/generic/config-4.14
	echo "# CONFIG_ZX_TDM is not set" >> "$1"/target/linux/generic/config-4.14
	echo "CONFIG_IMQ_NUM_DEVS=2" >> "$1"/target/linux/generic/config-4.14
	echo "# CONFIG_IMQ_BEHAVIOR_AA is not set" >> "$1"/target/linux/generic/config-4.14
	echo "# CONFIG_IMQ_BEHAVIOR_AB is not set" >> "$1"/target/linux/generic/config-4.14
	echo "CONFIG_IMQ_BEHAVIOR_BA=y" >> "$1"/target/linux/generic/config-4.14
	echo "# CONFIG_IMQ_BEHAVIOR_BB is not set" >> "$1"/target/linux/generic/config-4.14
fi
insert_lines_at()
{
	insert_after=$1
	lines=$2
	file=$3
	default_end=$4 #if line not specified, 0=put at end 1=put at beginning, default=end

	file_length=$(cat $file | wc -l | sed 's/ .*$//g')
	if [ -z "$insert_after" ] ; then
		if [ $default_end = "0" ] ; then
			$insert_after=0
		else
			$insert_after=$(($file_length+1))
		fi
	fi
	remainder=$(($file_length - $insert_after))
		
	head -n $insert_after $file >.tmp.tmp
	printf "$lines\n" >>.tmp.tmp
	if [ $remainder -gt 0 ] ; then
		tail -n $remainder $file >>.tmp.tmp
	fi
	mv .tmp.tmp $file
}


#define paths
openwrt_buildroot_dir="$1"
module_dir="$2"

#patch modes
patch_openwrt="$3"
patch_kernel="$4"
if [ -z "$patch_openwrt" ] ; then
	patch_openwrt="1"
fi
if [ -z "$patch_kernel" ] ; then
	patch_kernel="1"
fi



if [ -z "$openwrt_buildroot_dir" ] ; then
	echo "ERROR: you must specify OpenWrt buildroot directory"
	exit
fi
if [ -z "$module_dir" ] ; then
	echo "ERROR: you must specify module source directory"
	exit
fi

if [ ! -e "$openwrt_buildroot_dir/.config" ] ; then
	echo "ERROR: you must have a build configuration specified to run this script (run make menuconfig or make sure you have a .config file in the buildroot dir"
	exit
fi


new_module_dirs=""
new_module_list=$(ls "$module_dir" 2>/dev/null)
for d in $new_module_list ; do
	if [ -d "$module_dir/$d" ] ; then
		new_name=$(cat $module_dir/$d/name 2>/dev/null)
		if [ -n "$new_name" ] ; then
			new_module_dirs="$d $new_module_dirs"
		fi	
	fi
done
if [ -z "$new_module_dirs" ] ; then
	#nothing to do, exit cleanly without error
	exit
fi

#make paths absolute
exec_dir=$(pwd);
cd "$openwrt_buildroot_dir"
openwrt_buildroot_dir=$(pwd)
cd "$exec_dir"
cd "$module_dir"
module_dir=$(pwd)
cd "$exec_dir"


cd "$openwrt_buildroot_dir"
mkdir -p nf-patch-build
rm -rf nf-patch-build/* 2>/dev/null #should be nothing there, should fail with error (which just gets dumped to /dev/null), but let's be sure

if [ ! -d dl ] ; then
	mkdir dl
fi

####################################################################################################
##### CREATE MAKEFILE THAT WILL DOWNLOAD LINUX SOURCE FOR TARGET SPECIFIED IN .config FILE #########
####################################################################################################

if [ "$patch_kernel" = 1 ] ; then
	target_name=$(cat .config | egrep  "CONFIG_TARGET_([^_]+)=y" | sed 's/^.*_//g' | sed 's/=y$//g' )
	if [ -z "$target_name" ] ; then
		test_names=$(cat .config | egrep  "CONFIG_TARGET_.*=y" | sed 's/CONFIG_TARGET_//g' | sed 's/_.*$//g' )
		for name in $test_names ; do
			for kernel in 2.2 2.4 2.6 2.8 3.0 3.2 3.4 ; do  #let's plan ahead!!!
				if [ -d "target/linux/$name-$kernel" ] ; then
					target_name="$name-$kernel"
				fi
			done
		done
	fi


	board_var=$(cat target/linux/$target_name/Makefile | grep "BOARD.*:=")
	kernel_var=$(cat target/linux/$target_name/Makefile | grep "KERNEL.*:=")
	linux_ver_var=$(cat target/linux/$target_name/Makefile | grep "LINUX_VERSION.*:=") 
	defines=$(printf "$board_var\n$kernel_var\n$linux_ver_var\n")

	cat << 'EOF' >nf-patch-build/linux-download-make
CP:=cp -fpR
TOPDIR:=..
INCLUDE_DIR:=$(TOPDIR)/include
SCRIPT_DIR:=$(TOPDIR)/scripts
DL_DIR:=$(TOPDIR)/dl
EOF

	printf "$defines\n" >> nf-patch-build/linux-download-make

	cat << 'EOF' >>nf-patch-build/linux-download-make
include $(INCLUDE_DIR)/kernel-version.mk
KERNEL_NAME:=$(shell echo "$(KERNEL)" | sed 's/ /\./g' |  sed 's/\.$$//g' )
KERNEL_PATCHVER_NAME:=$(shell echo "$(KERNEL_PATCHVER)" | sed 's/ /\./g' |  sed 's/\.$$//g' )

GENERIC_PLATFORM_DIR := $(TOPDIR)/target/linux/generic
PLATFORM_DIR:=$(TOPDIR)/target/linux/$(BOARD)

GENERIC_BACKPORT_PATCH_DIR := $(GENERIC_PLATFORM_DIR)/backport-$(KERNEL_NAME)
GENERIC_PENDING_PATCH_DIR := $(GENERIC_PLATFORM_DIR)/pending-$(KERNEL_NAME)
GENERIC_HACK_PATCH_DIR := $(GENERIC_PLATFORM_DIR)/hack-$(KERNEL_NAME)
GENERIC_PATCH_DIR := $(GENERIC_PLATFORM_DIR)/patches-$(KERNEL_NAME)

GENERIC_FILES_DIR := $(GENERIC_PLATFORM_DIR)/files
GENERIC_LINUX_CONFIG:=$(firstword $(wildcard $(GENERIC_PLATFORM_DIR)/config-$(KERNEL_PATCHVER_NAME) $(GENERIC_PLATFORM_DIR)/config-default))
PATCH_DIR := $(PLATFORM_DIR)/patches$(shell [ -d "$(PLATFORM_DIR)/patches-$(KERNEL_PATCHVER_NAME)" ] && printf -- "-$(KERNEL_PATCHVER_NAME)" || true )
FILES_DIR := $(PLATFORM_DIR)/files$(shell [ -d "$(PLATFORM_DIR)/files-$(KERNEL_PATCHVER_NAME)" ] && printf -- "-$(KERNEL_PATCHVER_NAME)" || true )
LINUX_CONFIG:=$(firstword $(wildcard $(foreach subdir,$(PLATFORM_DIR) $(PLATFORM_SUBDIR),$(subdir)/config-$(KERNEL_PATCHVER_NAME) $(subdir)/config-default)) $(PLATFORM_DIR)/config-$(KERNEL_PATCHVER_NAME))
LINUX_DIR:=linux
PKG_BUILD_DIR:=$(LINUX_DIR)
TARGET_BUILD:=1
LINUX_SOURCE:=linux-$(LINUX_VERSION).tar.xz
TESTING:=$(if $(findstring -rc,$(LINUX_VERSION)),/testing,)
ifeq ($(call qstrip,$(CONFIG_EXTERNAL_KERNEL_TREE))$(call qstrip,$(CONFIG_KERNEL_GIT_CLONE_URI)),)
  ifeq ($(word 1,$(subst ., ,$(KERNEL_BASE))),3)
    LINUX_SITE:=@KERNEL/linux/kernel/v3.x$(TESTING)
  else
    LINUX_SITE:=@KERNEL/linux/kernel/v$(word 1,$(subst ., ,$(KERNEL_BASE))).x$(TESTING)
  endif
endif


define filter_series
sed -e s,\\#.*,, $(1) | grep -E \[a-zA-Z0-9\]
endef


all:
	if [ ! -e "$(DL_DIR)/$(LINUX_SOURCE)" ] ; then TOPDIR="$(TOPDIR)"  $(SCRIPT_DIR)/download.pl $(DL_DIR) $(LINUX_SOURCE) $(LINUX_KERNEL_HASH) $(LINUX_SOURCE) $(LINUX_SITE) ; fi ; 
	cp $(DL_DIR)/$(LINUX_SOURCE) . 
	rm -rf linux linux-$(LINUX_VERSION)
	tar xfJ $(LINUX_SOURCE)
	if [  ! -e "$(DL_DIR)/$(LINUX_SOURCE)" ] ; then mv $(LINUX_SOURCE) "$(DL_DIR)/" ; else rm $(LINUX_SOURCE) ; fi
	mv linux-$(LINUX_VERSION) linux
	rm -rf $(PKG_BUILD_DIR)/patches; mkdir -p $(PKG_BUILD_DIR)/patches 
	if [ -d $(GENERIC_FILES_DIR) ]; then $(CP) $(GENERIC_FILES_DIR)/* $(LINUX_DIR)/; fi 
	if [ -d $(FILES_DIR) ]; then \
		$(CP) $(FILES_DIR)/* $(LINUX_DIR)/; \
		find $(LINUX_DIR)/ -name \*.rej | xargs rm -f; \
	fi

	if [ -d "$(GENERIC_BACKPORT_PATCH_DIR)" ] ; then $(SCRIPT_DIR)/patch-kernel.sh linux $(GENERIC_BACKPORT_PATCH_DIR) ; fi
	if [ -d "$(GENERIC_PENDING_PATCH_DIR)" ] ; then $(SCRIPT_DIR)/patch-kernel.sh linux $(GENERIC_PENDING_PATCH_DIR) ; fi
	if [ -d "$(GENERIC_HACK_PATCH_DIR)" ] ; then $(SCRIPT_DIR)/patch-kernel.sh linux $(GENERIC_HACK_PATCH_DIR) ; fi
	if [ -d "$(GENERIC_PATCH_DIR)" ] ; then $(SCRIPT_DIR)/patch-kernel.sh linux $(GENERIC_PATCH_DIR) ; fi
	if [ -d "$(PATCH_DIR)" ] ; then $(SCRIPT_DIR)/patch-kernel.sh linux $(PATCH_DIR) ; fi
	mkdir -p "$(PATCH_DIR)"

	echo $(GENERIC_BACKPORT_PATCH_DIR) > generic-backport-patch-dir
	echo $(GENERIC_PENDING_PATCH_DIR) > generic-pending-patch-dir
	echo $(GENERIC_HACK_PATCH_DIR) > generic-hack-patch-dir
	echo $(GENERIC_PATCH_DIR) > generic-patch-dir
	echo $(GENERIC_LINUX_CONFIG) > generic-config-file
	echo $(PATCH_DIR) > patch-dir
	echo $(LINUX_CONFIG) > config-file
EOF

	####################################################################################################
	##### NOW CREATE MAKEFILE THAT WILL DOWNLOAD IPTABLES SOURCE #######################################
	####################################################################################################
	echo 'TOPDIR:=..' >> nf-patch-build/iptables-download-make
	echo 'SCRIPT_DIR:=$(TOPDIR)/scripts' >> nf-patch-build/iptables-download-make
	echo 'DL_DIR:=$(TOPDIR)/dl' >> nf-patch-build/iptables-download-make
	egrep "CONFIG_LINUX_.*=y" .config | sed 's/=/:=/g' >> nf-patch-build/iptables-download-make

	package_include_line_num=$(cat package/network/utils/iptables/Makefile | egrep -n "include.*package.mk" | sed 's/:.*$//g' )
	head -n $package_include_line_num package/network/utils/iptables/Makefile | awk ' { if( ( $0 !~ /^include/ ) && ($0 !~ /^#/ )){ print $0 ; }} ' >> nf-patch-build/iptables-download-make

	echo '' >> nf-patch-build/iptables-download-make
	echo 'include $(TOPDIR)/include/download.mk' >> nf-patch-build/iptables-download-make
	echo '' >> nf-patch-build/iptables-download-make
	

	echo 'all:' >> nf-patch-build/iptables-download-make
	echo '	if [ ! -e "$(DL_DIR)/$(PKG_SOURCE)" ] ; then  TOPDIR="$(TOPDIR)" $(SCRIPT_DIR)/download.pl $(DL_DIR) $(PKG_SOURCE) $(PKG_HASH)  $(PKG_SOURCE)  $(PKG_SOURCE_URL) ; fi ; ' >> nf-patch-build/iptables-download-make
	echo '	cp $(DL_DIR)/$(PKG_SOURCE) . ' >>nf-patch-build/iptables-download-make
	echo '	tar xf $(PKG_SOURCE)' >>nf-patch-build/iptables-download-make
	echo '	rm -rf *.bz2 *.xz' >>nf-patch-build/iptables-download-make
	echo '	mv iptables* iptables' >>nf-patch-build/iptables-download-make
	echo '	$(SCRIPT_DIR)/patch-kernel.sh iptables $(TOPDIR)/package/network/utils/iptables/patches/' >>nf-patch-build/iptables-download-make
	echo '	echo $(TOPDIR)/package/network/utils/iptables/patches/ > iptables-patch-dir' >>nf-patch-build/iptables-download-make
fi



cd nf-patch-build


####################################################################################################
##### Build Patches  ###############################################################################
####################################################################################################

if [ "$patch_kernel" = 1 ] ; then
	mv linux-download-make Makefile
	make
	mv linux linux.orig
	cp -r linux.orig linux.new


	mv iptables-download-make Makefile
	make
	mv iptables iptables.orig
	cp -r iptables.orig iptables.new


	generic_config_file=$(cat generic-config-file)
	config_file=$(cat config-file)
	patch_dir=$(cat patch-dir)
	iptables_patch_dir=$(cat iptables-patch-dir)
	
	mkdir -p "$iptables_patch_dir"
	mkdir -p "$patch_dir"
fi

echo "new_module_dirs=$new_module_dirs"

for new_d in $new_module_dirs ; do
	new_d="$module_dir/$new_d"
	new_name=$(cat $new_d/name 2>/dev/null)
	upper_name=$(echo "$new_name" | tr "[:lower:]" "[:upper:]")
	lower_name=$(echo "$new_name" | tr "[:upper:]" "[:lower:]")
		
	echo "found $upper_name module, patching..."
	
	if [ "$patch_kernel" = 1 ] ; then		
		#copy files for netfilter module
		cp -r $new_d/module/* linux.new/net/ipv4/netfilter/
		cp -r $new_d/header/* linux.new/include/linux/netfilter_ipv4/
	
		#update netfilter Makefile
		match_comment_line_num=$(cat linux.new/net/ipv4/netfilter/Makefile | egrep -n "#.*[Mm][Aa][Tt][Cc][Hh]" | sed 's/:.*$//g' )
		config_line='obj-$(CONFIG_IP_NF_MATCH_'$upper_name') += ipt_'$lower_name'.o' 
		insert_lines_at "$match_comment_line_num" "$config_line" "linux.new/net/ipv4/netfilter/Makefile" "1"
		cp  "linux.new/net/ipv4/netfilter/Makefile" ./test1

		#update netfilter Config.in/Kconfig file
		if [ -e linux.new/net/ipv4/netfilter/Kconfig ] ; then
			end_line_num=$(cat linux.new/net/ipv4/netfilter/Kconfig | egrep -n "endmenu" | sed 's/:.*$//g' )
			insert_line_num=$(($end_line_num-1))
			config_lines=$(printf "%s\n"  "config IP_NF_MATCH_$upper_name" "	tristate \"$lower_name match support\"" "	depends on IP_NF_IPTABLES" "	help" "		This option enables $lower_name match support." "" "")
			insert_lines_at "$insert_line_num" "$config_lines" "linux.new/net/ipv4/netfilter/Kconfig" "1"
		fi
		if [ -e linux.new/net/ipv4/netfilter/Config.in ] ; then
			match_comment_line_num=$(cat linux.new/net/ipv4/netfilter/Config.in | egrep -n "#.*[Mm][Aa][Tt][Cc][Hh]" | sed 's/:.*$//g' )
			match_comment_line="  dep_tristate '  $lower_name match support' CONFIG_IP_NF_MATCH_$upper_name \$CONFIG_IP_NF_IPTABLES"
			insert_lines_at "$match_comment_line_num" "$match_comment_line" "linux.new/net/ipv4/netfilter/Config.in" "1"
			cp  "linux.new/net/ipv4/netfilter/Config.in" ./test2
		fi
	
		#copy files for iptables extension
		cp -r $new_d/extension/* iptables.new/extensions
		cp -r $new_d/header/*    iptables.new/include/linux/netfilter_ipv4/

		#create test file, which is used by iptables Makefile
		echo "#!/bin/sh" > "iptables.new/extensions/.$lower_name-test"
		echo "[ -f \$KERNEL_DIR/include/linux/netfilter_ipv4/ipt_$lower_name.h ] && echo $lower_name" >> "iptables.new/extensions/.$lower_name-test"
		chmod 777 "iptables.new/extensions/.$lower_name-test"

		#update config templates -- just for simplicity do so for both 2.4-generic and 2.6-generic 
		for config in $generic_config_file $config_file ; do
			echo "CONFIG_IP_NF_MATCH_$upper_name=m" >> $config
		done
	fi
	
	kernel_netfilter_mk="package/kernel/linux/modules/netfilter.mk"
	iptables_makefile="package/network/utils/iptables/Makefile"

	
	if [ "$patch_openwrt" = "1" ] ; then
		### 判断是否存在该模块
		ifExist1=`cat "../$kernel_netfilter_mk" | grep "ipt-${lower_name}" 2>/dev/null 2>&1`
		if [ -n "$ifExist1" ]; then
			echo "Already exists"
		else
			#add OpenWrt package definition for netfilter module
			echo "" >>../"$kernel_netfilter_mk"
			echo "" >>../"$kernel_netfilter_mk"
			echo "define KernelPackage/ipt-$lower_name" >>../"$kernel_netfilter_mk"

			echo "  SUBMENU:=\$(NF_MENU)" >>../"$kernel_netfilter_mk"
			echo "  TITLE:=$lower_name" >>../"$kernel_netfilter_mk"
			echo "  KCONFIG:=\$(KCONFIG_IPT_$upper_name)" >>../"$kernel_netfilter_mk"
			echo "  FILES:=\$(LINUX_DIR)/net/ipv4/netfilter/*$lower_name*.\$(LINUX_KMOD_SUFFIX)" >>../"$kernel_netfilter_mk"
			echo "  AUTOLOAD:=\$(call AutoLoad,45,\$(notdir \$(IPT_$upper_name-m)))" >>../"$kernel_netfilter_mk"
			if [ "$lower_name" = "layer7" ] ; then
				echo "	DEPENDS:= +kmod-ipt-core +kmod-ipt-conntrack" >>../"$kernel_netfilter_mk"
			else
				echo "	DEPENDS:= kmod-ipt-core" >>../"$kernel_netfilter_mk"
			fi
			echo "endef" >>../"$kernel_netfilter_mk"
			echo "\$(eval \$(call KernelPackage,ipt-$lower_name))" >>../"$kernel_netfilter_mk"
		fi
		
		#判断是否存在该模块
		ifExist2=`cat "../$iptables_makefile" | grep "iptables-mod-${lower_name}" 2>/dev/null 2>&1`
		if [ -n "$ifExist2" ]; then
			echo "Makefile Already exists"
		else
			#add OpenWrt package definition for iptables extension
			echo "" >>../"$iptables_makefile" 
			echo "" >>../"$iptables_makefile" 
			echo "define Package/iptables-mod-$lower_name" >>../"$iptables_makefile" 
			echo "\$(call Package/iptables/Module, +kmod-ipt-$lower_name)" >>../"$iptables_makefile" 
			echo "  TITLE:=$lower_name" >>../"$iptables_makefile" 
			echo "endef" >>../"$iptables_makefile" 
			echo "\$(eval \$(call BuildPlugin,iptables-mod-$lower_name,\$(IPT_$upper_name-m)))" >>../"$iptables_makefile" 
		fi
		
		#判断是否存在该模块
		ifExist3=`cat "../include/netfilter.mk" | grep "${upper_name}" 2>/dev/null 2>&1`
		if [ -n "$ifExist3" ]; then
			echo "include/netfilter.mk Already exists ${upper_name}"
		else
			#update include/netfilter.mk with new module
			echo "" >>../include/netfilter.mk
			echo "" >>../include/netfilter.mk
			echo "IPT_$upper_name-m :=" >>../include/netfilter.mk
			echo "IPT_$upper_name-\$(CONFIG_IP_NF_MATCH_$upper_name) += \$(P_V4)ipt_$lower_name" >>../include/netfilter.mk
			echo "IPT_BUILTIN += \$(IPT_$upper_name-y)" >>../include/netfilter.mk
		fi
	fi
done

echo "Patching imq"
ifExist4=`cat "../package/network/utils/iptables/Makefile" | grep "imq" 2>/dev/null 2>&1`
if [ -n "$ifExist4" ]; then
	echo "Already exists iptables"
else
	cd ../package
	patch -p2<../package/gargoyle/netfilter-match-modules/patches/010-imq-iptables.patch
	cd -
fi

ifExist5=`cat "../package/kernel/linux/modules/netfilter.mk" | grep "imq" 2>/dev/null 2>&1`
if [ -n "$ifExist5" ]; then
	echo "Already exists modules"
else
	cd ../package
	patch -p2<../package/gargoyle/netfilter-match-modules/patches/010-imq-modules.patch
	cd -
fi

ifExist6=`cat "../include/netfilter.mk" | grep "imq" 2>/dev/null 2>&1`
if [ -n "$ifExist6" ]; then
	echo "Already exists include"
else
	cd ../
	patch -p1<./package/gargoyle/netfilter-match-modules/patches/010-imq-include.patch
	cd -
fi
		
if [ "$patch_kernel" = 1 ] ; then	
	#build netfilter patch file
	rm -rf $patch_dir/650-custom_netfilter_match_modules.patch 2>/dev/null
	cd linux.new
	module_files=$(find net/ipv4/netfilter)
	include_files=$(find include/linux/netfilter_ipv4)
	test_files="$module_files $include_files"
	cd ..
	for t in $test_files ; do
		if [ ! -d "linux.new/$t" ] ; then
			if [ -e "linux.orig/$t" ] ; then
				diff -u "linux.orig/$t" "linux.new/$t" | sed "1c --- a\/$t" | sed "2c +++ b\/$t" >> $patch_dir/650-custom_netfilter_match_modules.patch
			else
				diff -u /dev/null "linux.new/$t" | sed "1c --- a\/dev\/null" | sed "2c +++ b\/$t" >> $patch_dir/650-custom_netfilter_match_modules.patch
			fi	
		fi
	done

	#build iptables patch file
	rm -f ../package/iptables/patches/650-custom_netfilter_match_modules.patch 2>/dev/null
	cd iptables.new
	extension_files=$(find extensions)
	include_files=$(find include/linux/netfilter_ipv4)
	cd ..
	for t in $extension_files $include_files ; do
		if [ ! -d "iptables.new/$t" ] ; then
			if [ -e "iptables.orig/$t" ] ; then
				diff -u "iptables.orig/$t" "iptables.new/$t" | sed "1c --- a\/$t" | sed "2c +++ b\/$t" >>$iptables_patch_dir/650-custom_netfilter_match_modules.patch
			else
				diff -u /dev/null "iptables.new/$t" | sed "1c --- a\/dev\/null" | sed "2c +++ b\/$t" >>$iptables_patch_dir/650-custom_netfilter_match_modules.patch 
			fi
		fi	
	done
fi

#cleanup
cd ..

rm -rf nf-patch-build

