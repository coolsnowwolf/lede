# Simple script for building drm projects
# Usage:
#   source drm_configure.sh path/to/kernel/source cid(default 0x316) platform(default PCIE_GEN6)
# If success, then use "m" to make projects.
# Make all projects
#   root@drm-root$ m
# Clean all projects
#   root@drm-root$ m clean
# Make each project standalone
# root@drm-root/bo-helper$ m
# root@drm-root/bo-helper$ m clean
# root@drm-root/driver$ m
# root@drm-root/driver$ m clean
# root@drm-root/libdrm$ m
# root@drm-root/libdrm$ m clean

DRM_DRIVER_DIR_NAME=$(pwd)

function is_sourced() {
    #if [ -n "$ZSH_VERSION" ]; then
    #    case $ZSH_EVAL_CONTEXT in *:file:*) return 0;; esac
    #else
    #    case ${0##*/} in dash|-dash|bash|-bash|ksh|-ksh|sh|-sh) return 0;; esac
    #fi
    case ${0##*/} in bash|-bash) return 0;; esac
    return 1  # NOT sourced.
}


function show_usage() {
    echo "Execute detected, please source this script instead of executing directly."
    echo "!!!ONLY SUPPORT BASH !!!."
    echo "    \"source drm_configure.sh path/to/kernel/source\""
    return 0
}

function show_error() {
    echo "DRM root dir not found, please clone the whole repo with command:"
    echo "    \"repo init -u ssh://cnxxxx@192.168.32.113:29418/dpu/drm/manifest -b master\""
    return -1
}

function usage()
{
    cat << EOF
-----------------------------------------------------------------------
usage:
    source drm_configure.sh path/to/kernel/source CID PLATFORM

    CID         CHIP_MODEL        PLATFORM
   ------------------------------------------------------------------
    0x315        9500              PCIE_GEN6/PCIE_GEN7/ARM64
    0x316        9400              PCIE_GEN6/PCIE_GEN7/ARM64
    0x326        9400              PCIE_GEN6/PCIE_GEN7/ARM64
    0x327        9400              PCIE_GEN6/PCIE_GEN7/ARM64
    0x329        9400              PCIE_GEN6/PCIE_GEN7/ARM64
    0x32a        9400              PCIE_GEN6/PCIE_GEN7/ARM64
    0x323        9400              PCIE_GEN6/PCIE_GEN7/ARM64
    0x31b        FTD330              PCIE_GEN6/PCIE_GEN7/ARM64
    0x311        FTD330              PCIE_GEN6/PCIE_GEN7/ARM64
    0x310        FTD330              PCIE_GEN6/PCIE_GEN7/ARM64
    0x30b        FTD330              PCIE_GEN6/PCIE_GEN7/ARM64
    0x331        FTD330              PCIE_GEN6/PCIE_GEN7/ARM64
    0x335        FTD330              PCIE_GEN6/PCIE_GEN7/ARM64
   ------------------------------------------------------------------

-----------------------------------------------------------------------
EOF
}

function check_cross_compile() {
    # Check ARCH
    if [ "${BUILD_ARCH}" == "" ]; then
        export BUILD_ARCH=x86_64
        return 0
    fi
    if [[ "${BUILD_ARCH}" =~ "x86" ]]; then
        return 0
    fi

    # Check cross compile
    if [ "${CROSS_COMPILE}" == "" ]; then
        echo -e "\033[31mBUILD_ARCH is \"${BUILD_ARCH}\", please set CROSS_COMPILE !\033[0m"
        return -1
    fi
    # Check toolchain path
    command -v ${CROSS_COMPILE}gcc > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo -e "\033[31mCan't find \"${CROSS_COMPILE}\", please check toolchain path!\033[0m"
        return -1
    fi

    return 0
}


function config_platform_env() {
    if [ "$PLATFORM" = "PCIE_GEN6" ]; then
        #default PCIE_GEN6 X86_64
        export BUILD_ARCH=x86_64
        export FTD330_DRM_CONFIG_PCIE=1
        export CONFIG_PHYTIUM_PCIE_GEN7=n
    elif [ "$PLATFORM" = "PCIE_GEN7" ]; then
        export BUILD_ARCH=x86_64
        export FTD330_DRM_CONFIG_PCIE=1
        export CONFIG_PHYTIUM_PCIE_GEN7=y
    elif [ "$PLATFORM" = "ARM64" ]; then
        export BUILD_ARCH=arm64
        export FTD330_DRM_CONFIG_PCIE=0
        export CONFIG_PHYTIUM_PCIE_GEN7=n
    else
        echo "unknown platform."
        usage
        return -1
    fi

    return 0
}

function clear_config_env() {
    unset FTD330_DRM_CONFIG_PCIE
    unset CONFIG_PHYTIUM_PCIE_GEN7
    unset CONFIG_PHYTIUM_CHIP_FTD330
    unset CONFIG_PHYTIUM_CHIP_9x00
    unset CONFIG_PHYTIUM_DC9500_0x315
    unset CONFIG_PHYTIUM_DC9400_0x316
    unset CONFIG_PHYTIUM_DC9400_0x326
    unset CONFIG_PHYTIUM_DC9400_0x327
    unset CONFIG_PHYTIUM_DC9400_0x329
    unset CONFIG_PHYTIUM_DC9400_0x32a
    unset CONFIG_PHYTIUM_DC9200_0x323
    unset CONFIG_PHYTIUM_DCFTD330_0x31b
    unset CONFIG_PHYTIUM_DCFTD330_0x311
    unset CONFIG_PHYTIUM_DCFTD330_0x310
    unset CONFIG_PHYTIUM_DCFTD330_0x30b
    unset CONFIG_PHYTIUM_DCFTD330_0x331
    unset CONFIG_PHYTIUM_DCFTD330_0x335
    export CONFIG_PHYTIUM_PCIE_GEN7=n
    export CONFIG_PHYTIUM_CHIP_FTD330=n
    export CONFIG_PHYTIUM_CHIP_9x00=n
    export CONFIG_PHYTIUM_DC9500_0x315=n
    export CONFIG_PHYTIUM_DC9400_0x316=n
    export CONFIG_PHYTIUM_DC9400_0x326=n
    export CONFIG_PHYTIUM_DC9400_0x327=n
    export CONFIG_PHYTIUM_DC9400_0x329=n
    export CONFIG_PHYTIUM_DC9400_0x32a=n
    export CONFIG_PHYTIUM_DC9200_0x323=n
    export CONFIG_PHYTIUM_DCFTD330_0x31b=n
    export CONFIG_PHYTIUM_DCFTD330_0x311=n
    export CONFIG_PHYTIUM_DCFTD330_0x310=n
    export CONFIG_PHYTIUM_DCFTD330_0x30b=n
    export CONFIG_PHYTIUM_DCFTD330_0x331=n
    export CONFIG_PHYTIUM_DCFTD330_0x335=n
}

function config_env() {
    CID="$1"
    check_cross_compile
    if [ $? -ne 0 ]; then
        return -1
    fi

    export CROSS_COMPILE=$CROSS_COMPILE
    export DRM_DRIVER_DIR=$DRM_DRIVER_DIR_NAME
    echo "export BUILD_ARCH=$BUILD_ARCH"
    echo "export CROSS_COMPILE=$CROSS_COMPILE"
    echo "export DRM_DRIVER_DIR=$DRM_DRIVER_DIR_NAME"

    return 0
}


function build_driver() {
#    pushd $DRM_DRIVER_DIR >/dev/null
    make KERNEL_SRC=${KERNEL_SOURCE} \
        ARCH=${BUILD_ARCH} \
        CROSS_COMPILE=${CROSS_COMPILE} \
    ret=$?
#    popd >/dev/null
    return $ret
}

function clean_driver() {
#    pushd $DRM_DRIVER_DIR >/dev/null
    make KERNEL_SRC=${KERNEL_SOURCE} ARCH=${BUILD_ARCH} CROSS_COMPILE=${CROSS_COMPILE} clean
#    popd >/dev/null
}

function get_mdir() {
    MTARGET=$(basename $(pwd))
    case $MTARGET in
        driver|bo-helper|libdrm)
            return 0
            ;;
        *)
    esac
}

function m () {
    if [ $# -eq 0 ]; then
        get_mdir
	config_platform_env
	build_driver
    elif [ $1 == 'clean' ]; then
        get_mdir
        MTARGET=${MTARGET}-clean
	clean_driver
	clear_config_env
    else
        MTARGET=$1
    fi
    
}

export KERNEL_SOURCE=$1
export PLATFORM=$3
