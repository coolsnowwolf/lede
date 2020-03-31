#!/bin/bash -eu

function usage()
{
	echo "$0 [nanopi-h3|nanopi-h5|nanopi-h6]"
	exit 1
}

function create_rootfs_tgz()
{
	echo "creating base rootfs tarball(can not use directly)..."
	
	cd ./friendlyelec
	rm -rf ./rootfs ./rootfs-friendlywrt.tgz

	case ${BOARD} in
	nanopi-h3)
		rootfs_dir=`readlink -f ../build_dir/target-arm_cortex-a7+neon-vfpv4_musl_eabi/root-sunxi`
		;;
	nanopi-h5|nanopi-h6)
		rootfs_dir=`readlink -f ../build_dir/target-aarch64_cortex-a53_musl/root-sunxi`
		;;
	*)
		echo "unsupported board: ${BOARD}"
		exit 1
		;;
	esac

	cp -rdf ${rootfs_dir} ./rootfs
	rm ./rootfs/lib/modules/* -rf
	tar czf rootfs-friendlywrt.tgz rootfs
	ls -l ./rootfs-friendlywrt.tgz
	echo "done"
	cd ->/dev/null
}

if [ $# -ne 1 ]; then
	usage
fi

BOARD=$1
USING_DATE=$(date +%Y%m%d)
if [ ! -e "config_${BOARD}.seed" ];then
	usage
fi

CPU_CORES=`cat /proc/cpuinfo | grep "processor" | wc -l`
VER=18.06.1

cd ..
./scripts/feeds update -a
./scripts/feeds install -a

cp friendlyelec/config_${BOARD}.seed .config
make defconfig

if [ ! -d dl ]; then
	echo "dl directory not exist. Will make download full package from openwrt site."
fi
make download -j${CPU_CORES}
make -j${CPU_CORES} #V=s

create_rootfs_tgz
