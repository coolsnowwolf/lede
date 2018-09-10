#!/usr/bin/env bash
#
# Licensed under the terms of the GNU GPL License version 2 or later.
#
# Author: Jason Wu <jason.hy.wu@gmail.com>
# with modifications for multi-DTB-same-image by:
# Mathew McBride <matt@traverse.com.au>
#
# U-Boot firmware supports the booting of images in the Flattened Image
# Tree (FIT) format.  The FIT format uses a device tree structure to
# describe a kernel image, device tree blob, ramdisk, etc.  This script
# creates an Image Tree Source (.its file) which can be passed to the
# 'mkimage' utility to generate an Image Tree Blob (.itb file).  The .itb
# file can then be booted by U-Boot (or other bootloaders which support
# FIT images).  See doc/uImage.FIT/howto.txt in U-Boot source code for
# additional information on FIT images.
#
# This tools supports:
#   - multi-configuration
#   - multi-image support - multiple kernel/fdt/ramdsik
#   - per image configuration:
#     - hash algorithm and generated required subnodes
#     - compression
#     - signature and generated required subnodes
#
set -e

# image config limit
MAX_IMG=50
# conf config limit
MAX_CONF=10

# declare main data array
declare -a img_array
declare -a conf_array

# initialize array with empty values
for (( index=1; index<=$MAX_IMG; index++ )); do
	declare -a img$index
	for i in {0..13}; do
		eval img${index}[$i]=""
	done
done

for (( index=1; index<=$MAX_CONF; index++ )); do
	declare -a conf$index
	for i in {0..9}; do
		eval conf${index}[$i]=""
	done
done

# imgX array index information
#	0: type of image - kernel, fdt, ramdsik
#	1: image location
#	2: image index
#	3: loadaddr of image
#	4: entrypoint of image
#	5: compression
#	6: hash algorithm
#	7: part of the configuration
#	8: Human friend name for the image
#	9: key file name
#	10: signature
# 	11: conf friendly name

# confX array index information
#	0: conf number
#	1: kernel conf
#	2: fdt conf
#	3: rootfs conf
#	4: kernel key file
#	5: fdt key file
#	6: rootfs key file
#	7: kernel sign_algorithm
#	8: fdt sign_algorithm
#	9: rootfs sign_algorithm
#	10: conf friendly name

usage() {
	echo "Usage: `basename $0` -A arch -v version -o its_file" \
		"-k kernel -a addr -e entry [-C none] [-h sha1] [-c conf]"
	echo -e "Example1:\n\tkernel image ker_img1 with no compression +"
	echo -e "\tsha1 hash + fdt dtb1 with sha1 and crc32 hash for conf 1"
	echo -e "\t $ `basename $0` -A arm -v 4.4 \ "
	echo -e "\t      -k ker_img1 -C none -h sha1 -e 0x8000 -a 0x8000 -c 1 \ "
	echo -e "\t      -d dtb1 -h sha1 -h crc32 -c 1\n"
	echo "General settings:"
	echo -e "\t-A ==> set architecture to 'arch'"
	echo -e "\t-v ==> set kernel version to 'version'"
	echo -e "\t-o ==> create output file 'its_file' [optional]"
	echo "Input image type:"
	echo -e "\t-k ==> kernel image 'kernel'"
	echo -e "\t-d ==> Device Tree Blob 'dtb'"
	echo -e "\t-r ==> ramdisk image 'ramdisk"
	echo "Per image configurations:"
	echo -e "\t-C ==> set compression type 'comp'"
	echo -e "\t-c ==> set image config (multiple -c allowed)"
	echo -e "\t-a ==> set load address to 'addr' (hex)"
	echo -e "\t-e ==> set entry point to 'entry' (hex)"
	echo -e "\t-D ==> human friendly 'name' (one word only)"
	echo -e "\t-h ==> set hash algorithm (multiple -h allowed)"
	echo -e "\t-s ==> set signature for given config image"
	echo -e "\t-K ==> set key file for given config image"
	exit 1
}

array_check()
{
	local a=999
	local max_a=0
	local max_i=0

	if echo $1 | grep -q img; then
		max_a=$MAX_IMG
		max_i=13
		let a=$(echo $1 | awk -F "img" '{print $2}')
	elif echo $1 | grep -q conf; then
		max_a=$MAX_CONF
		max_i=10
		let a=$(echo $1 | awk -F "conf" '{print $2}')
	fi
	if [ ${a} -lt 0 -o ${a} -gt ${max_a} -o \
		${2} -lt 0 -o ${2} -gt ${max_i} ]; then
		echo "WARNING: Invalid array name, skipping!!!"
		return 255
	fi
}

#
# $1:	array name
# $2:	index
# $3:	value
# $4:	append operation
#
array_put()
{
	# check if array is declared
	array_check $1 $2 || return 0
	if [ -z "$4" ]; then
		eval $1[$2]=$3
	else
		eval $1[$2]=\"\${$1[$2]} $3\"
	fi
}

#
# $1:	array name
# $2:	index
#
array_get()
{
	local val
	eval val=\${$1[$2]}
	echo $val
}

parse_args() {
	local i=-1 k=-1 d=-1 r=-1
	while getopts ":A:a:C:c:D:d:e:h:k:K:o:v:r:s:n:" OPTION; do
		case $OPTION in
			A ) ARCH=$OPTARG;;
			a ) array_put img$i 3 $OPTARG;;
			C ) value_sanity_chk compression $OPTARG;
				array_put img$i 5 $OPTARG;;
			c ) array_put img$i 7 $OPTARG append;;
			D ) array_put img$i 8 $OPTARG;;
			d ) i=$(($i + 1));
				d=$(($d + 1));
				img_array[$i]=img$i;
				array_put img$i 0 fdt;
				array_put img$i 1 $OPTARG;
				array_put img$i 2 $d;
				;;
			e ) array_put img$i 4 $OPTARG;;
			h ) value_sanity_chk hash $OPTARG;
				array_put img$i 6 $OPTARG append;;
			k ) i=$(($i + 1));
				k=$(($k + 1));
				img_array[$i]=img$i;
				array_put img$i 0 "kernel";
				array_put img$i 1 $OPTARG;
				array_put img$i 2 $k;
				;;
			K ) array_put img$i 9 $OPTARG;;
			n ) array_put img$i 11 $OPTARG;;
			o ) OUTPUT=$OPTARG;;
			v ) VERSION=$OPTARG;;
			r ) i=$(($i + 1));
				r=$(($r + 1));
				img_array[$i]=img$i;
				array_put img$i 0 "ramdisk";
				array_put img$i 1 $OPTARG;
				array_put img$i 2 $r;
				;;
			s ) value_sanity_chk signature $OPTARG;
				array_put img$i 10 $OPTARG;
				;;
			* ) echo "Invalid option passed to '$0' (options:$@)"
			usage;;
		esac
	done
	[ -n "${OUTPUT}" ] || OUTPUT=fitimage.its
	[ -n "${VERSION}" ] || VERSION="Unknown"
	[ -n "${ARCH}" ] || ARCH=arm
}

#
# sanity check for signature, compression and hash
#
value_sanity_chk()
{
	local valid=""
	case $1 in
		signature) valid="sha-1,rsa-2048 sha-256,rsa-2048 sha-256,rsa-4096";;
		compression) valid="gzip bzip2 none";;
		hash) valid="sha1 md5 crc32";;
	esac
	if ! echo $valid | grep -q "$2"; then
		echo "Error: Invalid $1 provided '$2'"
		echo "Valid options are: $valid"
		exit 255
	fi
}

#
# Emit the fitImage section bits
#
# $1: Section bit type: fitstart   - its header
#                       imagestart - image section start
#                       confstart  - configuration section start
#                       sectend    - section end
#                       fitend     - fitimage end
# $2: optional variable for confstart section
#
emit_its() {
	case $1 in
	fitstart)
		cat << EOF > ${OUTPUT}
/dts-v1/;

/ {
	description = "U-Boot fitImage for ${VERSION} kernel";
	#address-cells = <1>;
EOF
	;;
	imagestart)
		echo -e "\n\timages {" >> ${OUTPUT};;
	confstart)
#		echo -e "\tconfigurations {\n\t\tdefault = \"conf@${2:-0}\";" \
	echo -e "\tconfigurations {\n" \
			>> ${OUTPUT};;
	sectend)
		echo -e "\t};" >> ${OUTPUT};;
	fitend)
		echo -e "};" >> ${OUTPUT};;
	esac
}

#
# Emit kernel image node
#
emit_kernel() {
	local image=${1}
	local count=${2:-${MAX_IMG}}
	local loaddaddr=${3:-0x8000}
	local entrypoint=${4:-0x8000}
	local compresson=${5:-none}
	local checksum=${6:-sha1}
	local name=${7}

	[ -z "${name}" ] || name=" ${name}"
	cat << EOF >> ${OUTPUT}
		kernel@${count} {
			description = "Linux Kernel${name}";
			data = /incbin/("${image}");
			type = "kernel";
			arch = "${ARCH}";
			os = "linux";
			compression = "${compresson}";
			load = <${loaddaddr}>;
			entry = <${entrypoint}>;
EOF
	emit_cksum ${checksum}

	if [ -z "$SIGN_IN_CONF" ] ; then
		emit_signature "$9" "" "" "$8" "" ""
	fi

	echo "		};" >> ${OUTPUT}
}

#
# Emit fdt node
#
emit_fdt() {
	local image=${1}
	local count=${2:-${MAX_IMG}}
	local compresson=${3:-none}
	local checksum=${4:-sha1}
	local name=${5}
	local loadaddr=${6}

	[ -z "${name}" ] || name=" ${name}"
	cat << EOF >> ${OUTPUT}
		fdt@${count} {
			description = "Flattened Device Tree blob${name}";
			data = /incbin/("${image}");
			type = "flat_dt";
			arch = "${ARCH}";
			load = <${loadaddr}>;
			compression = "none";
EOF
	emit_cksum ${checksum}
	if [ -z "$SIGN_IN_CONF" ] ; then
		emit_signature "" "$7" "" "" "$6" ""
	fi
	echo "		};" >> ${OUTPUT}
}

#
# Emit ramdisk node
#
emit_ramdisk() {
	local image=${1}
	local count=${2:-${MAX_IMG}}
	local compresson=${3:-none}
	local checksum=${4:-sha1}
	local name=${5}

	[ -z "${name}" ] || name=" ${name}"
	cat << EOF >> ${OUTPUT}
		ramdisk@${count} {
			description = "ramdisk${name}";
			data = /incbin/("${image}");
			type = "ramdisk";
			arch = "${ARCH}";
			os = "linux";
			compression = "${compresson}";
EOF
	emit_cksum ${checksum}
	if [ -z "$SIGN_IN_CONF" ] ; then
		emit_signature "" "" "$7" "" "" "$6"
	fi
	echo "		};" >> ${OUTPUT}
}

#
# Emit check sum sub node
#
emit_cksum() {
	csum_list=$@
	count=1
	for csum in ${csum_list}; do
		cat << EOF >> ${OUTPUT}
			hash@${count} {
				algo = "${csum}";
			};
EOF
		count=`expr ${count} + 1`
	done
}

#
# Emit signature sub node
#
emit_signature() {
	local kernel=$1
	local fdt=$2
	local rootfs=$3
	local kernel_key=$4
	local fdt_key=$5
	local rootfs_key=$6
	local imgs=""
	local count=0
	local chk_list="" algo="" algos="" i=""

	for i in kernel fdt rootfs; do
		eval algo=\$$i
		eval key=\$${i}_key
		[ -n "$algo" ] || continue
		if ! echo "$algos" | grep -q $algo; then
			if [ -z "$algos" ]; then
				algos=$algo
			else
				algos="${algos} $algo"
			fi
		fi
		if ! echo "$keys" | grep -q $key; then
			if [ -z "$keys" ]; then
				keys=$key
			else
				keys="${keys} $key"
			fi
		fi
	done

	for algo in $algos; do
		for key in $keys; do
			img=""
			for i in kernel fdt rootfs; do
				eval tmp_algo=\$$i
				eval tmp_key=\$${i}_key
				[ "$tmp_algo" == "$algo" ] || continue
				[ "$tmp_key" == "$key" ] || continue
				if [ -z "$img" ]; then
					img=$i
				else
					img=${img},$i
				fi
			done

			[ -n "$img" ] || continue
			cat << EOF >> ${OUTPUT}
			signature@${count} {
				algo = "${algo}";
				key-name-hint = "${key}";
EOF
			if [ -n "$SIGN_IN_CONF" ] ; then
				echo "			sign-images = \"$img\";" >> ${OUTPUT}
			fi
			echo "			};" >> ${OUTPUT}

			count=`expr ${count} + 1`
		done
	done
}

#
# Emit config sub nodes
#
emit_config() {
	local conf_csum="sha1"

	config_name="conf@${1}"
	if [ ! -z "${11}" ]; then
		config_name="${11}"
	fi 
	if [ -z "${2}" ]; then
		echo "Error: config has no kernel img, skipping conf node!"
		return 0
	fi

	# Test if we have any DTBs at all
	if [ -z "${3}" ] ; then
		conf_desc="Boot Linux kernel"
		fdt_line=""
	else
		conf_desc="Boot Linux kernel with FDT blob"
		fdt_line="
			fdt = \"fdt@${3}\";"
	fi

	# Test if we have any ROOTFS at all
	if [ -n "${4}" ] ; then
		conf_desc="$conf_desc + ramdisk"
		fdt_line="${fdt_line}
			ramdisk = \"ramdisk@${4}\";"
	fi

	kernel_line="kernel = \"kernel@${2}\";"

	cat << EOF >> ${OUTPUT}
		${config_name} {
			description = "${conf_desc}";
			${kernel_line}${fdt_line}
			hash@1 {
				algo = "${conf_csum}";
			};
EOF
	if [ -n "$SIGN_IN_CONF" ] ; then
		emit_signature "$5" "$6" "$7" "$8" "$9" "${10}"
	fi

	echo "		};" >> ${OUTPUT}
}

#
# remove prefix space
#
remove_prefix_space()
{
	echo "$@" | sed "s:^ ::g"
}

#
# generate image nodes and its subnodes
#
emit_image_nodes()
{
	local t img_c img_i img_index chk
	local img_type img_path img_count img_loadadr img_entrypoint \
		img_compression img_hash img_conf img_name img_key img_sign \
		img_index

	emit_its imagestart
	for t in "kernel" "fdt" "ramdisk"; do
		img_index=0
		for a in ${img_array[@]}; do
			img_type=$(array_get $a 0)
			img_path=$(array_get $a 1)
			img_count=$(array_get $a 2)
			img_loadadr=$(array_get $a 3)
			img_entrypoint=$(array_get $a 4)
			img_compression=$(array_get $a 5)
			img_hash=$(array_get $a 6)
			img_conf=$(array_get $a 7)
			img_name=$(array_get $a 8)
			img_key=$(array_get $a 9)
			img_sign=$(array_get $a 10)
			img_cname=$(array_get $a 11)
			
			img_conf=$(remove_prefix_space $img_conf)
			img_hash=$(remove_prefix_space $img_hash)

			[ "${img_type}" == $t ] || continue
			# generate sub nodes
			eval chk=\$DEF_$t
			[ -n "${chk}" ] || eval DEF_$t=$img_count
			case $t in
				kernel) emit_kernel "$img_path" "$img_count" \
					"$img_loadadr" "$img_entrypoint" \
					"$img_compression" "$img_hash" \
					"$img_name" "$img_key" "$img_sign";;
				fdt) emit_fdt "$img_path" "$img_count" \
					"$img_compression" "$img_hash" \
					"$img_name" "$img_loadadr" "$img_key" "$img_sign"  ;;

				ramdisk) emit_ramdisk "$img_path" "$img_count" \
					"$img_compression" "$img_hash" \
					"$img_name" "$img_key" "$img_sign";;
			esac

			# set up configuration data
			for img_c in $img_conf; do
				img_i=""
				#set up default configuration if its not set
				[ -n "$DEF_CONFIG" ] || DEF_CONFIG=$img_c
				[ -z "${img_c}" ] || conf_array[$img_c]=conf$img_c
				array_put conf$img_c 0 ${img_c}
				case $t in
					kernel) img_i=1;;
					fdt) img_i=2;;
					ramdisk) img_i=3;;
				esac
				array_put conf$img_c $img_i $img_index
				array_put conf$img_c $(($img_i + 3)) ${img_sign}
				array_put conf$img_c $(($img_i + 6)) ${img_key}
				array_put conf$img_c 10 $img_cname
			done
			img_index=$((img_index + 1))
		done
	done
	emit_its sectend
}

#
# generate configuration node and its subnodes
#
emit_configuration_nodes ()
{
	local count kernel fdt ramdisk ker_file fdt_file rfs_file ker_sign \
		fdt_sign rfs_sign
	emit_its confstart $DEF_CONFIG
	for a in ${conf_array[@]}; do
		count=$(array_get $a 0)
		kernel=$(array_get $a 1)
		fdt=$(array_get $a 2)
		ramdisk=$(array_get $a 3)
		er_file=$(array_get $a 4)
		fdt_file=$(array_get $a 5)
		rfs_file=$(array_get $a 6)
		ker_sign=$(array_get $a 7)
		fdt_sign=$(array_get $a 8)
		rfs_sign=$(array_get $a 9)
		cname=$(array_get $a 10)
		emit_config "$count" "$kernel" "$fdt" "$ramdisk" "$ker_file" \
			"$fdt_file" "$rfs_file" "$ker_sign" "$fdt_sign" \
			"$rfs_sign" "${cname}"
	done
	if [ -z "${DEF_CONFIG}" ]; then
		emit_config "0" "$DEF_kernel" "$DEF_fdt" "$DEF_ramdisk"
	fi
	emit_its sectend
}

# Set to none empty to create signature sub node under images node
SIGN_IN_CONF=${SIGN_IN_CONF:-""}
# Set to default config used
DEF_CONFIG=${DEF_CONFIG:-""}

parse_args $@

emit_its fitstart
emit_image_nodes
emit_configuration_nodes
emit_its fitend
