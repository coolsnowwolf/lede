#!/bin/sh /etc/rc.common

#################################################################

# qat          Start/Stop the Intel QAT.
#

START=29
STOP=99

ADF_CTL=/usr/sbin/adf_ctl
QAT_GID=200
INTEL_VENDORID="8086"
C3XX_DEVICE_PCI_ID="19e2"
NUM_C3XXX_DEVICES=$(lspci -n | egrep -c "$INTEL_VENDORID:$C3XX_DEVICE_PCI_ID")
HUGE_PAGE_DIR=/dev/hugepages
HUGE_PAGE_MOUNT=$(mount | grep hugetlbfs | awk '{print $3}')
MAX_HUGE_PAGES=200
MAX_HUGE_PAGES_PER_PROCESS=10

EXTRA_COMMANDS="status"
EXTRA_HELP="        status Show the status of the qat device"
 

qat_started() {

    ${ADF_CTL} status > /dev/null 2>&1 
    return $?

}


status() {

    ${ADF_CTL} status
    if [ "$?" -ne 0 ]
    then
        echo "No devices found. Please start the driver using:"
        echo "$0 start"
    fi

}

insert_digest_modules() {

    if [ $(lsmod | grep -c "sha512") == 0 ]; then

         if [ $(cat /proc/kallsyms |grep -c sha512_) == 0 ]; then
             modprobe sha512_ssse3
         else
             echo "$0 no sha512_ssse3 module to load"
             return 1
         fi

    fi

    if [ $(lsmod | grep -c "sha256") == 0 ]; then

         if [ $(cat /proc/kallsyms |grep -c sha256_) == 0 ]; then
             modprobe sha256_ssse3
         else
             echo "$0 no sha256_ssse3 module to load"
             return 1
         fi

    fi

    return 0

}

mount_hugetlbfs() {

    if [ "${HUGE_PAGE_MOUNT}" != "${HUGE_PAGE_DIR}" ]; then

        [ ! -d ${HUGE_PAGE_DIR} ] && mkdir ${HUGE_PAGE_DIR}

        mount -t hugetlbfs hugetlbfs ${HUGE_PAGE_DIR} -o mode=1770 -o gid=${QAT_GID}

        if [ $? -ne 0 ]; then
            echo "$0: error mounting hugetlbfs on ${HUGE_PAGE_DIR}"
            return 1
        fi

    fi

    return 0

}

insert_modules() {

    # common functions kernel module

    if [ $(lsmod | grep "intel_qat" | wc -l) == "0" ]; then
        modprobe intel_qat
    fi

    [ $? -ne 0 ] && return 1

    # contiguous pinned memory driver
    # requires huge pages support in the kernel

    if [ $(lsmod | grep "usdm_drv" | wc -l) == "0" ]; then
        modprobe usdm_drv max_huge_pages=${MAX_HUGE_PAGES} max_huge_pages_per_process=${MAX_HUGE_PAGES_PER_PROCESS}
    fi

    [ $? -ne 0 ] && return 1

    # qat device driver

    if [ $(lsmod | grep "qat_c3xxx" | wc -l) == "0" ]; then
        modprobe qat_c3xxx
    fi

    [ $? -ne 0 ] && return 1

    # qat kernel api

    if [ $(lsmod | grep "qat_api" | wc -l) == "0" ]; then
        modprobe qat_api
    fi

    return $?

}

start() {

    /sbin/depmod -a 2> /dev/null

    qat_started

    if [ $? -ne 0 ]; then

        insert_digest_modules && [ $? -eq 0 ] && mount_hugetlbfs

        if [ $? -eq 0 ]; then

            if [ ${NUM_C3XXX_DEVICES} != 0 ]; then
                insert_modules
                [ $? -eq 0 ] && ${ADF_CTL} restart
            fi

        fi

        else echo "QAT already started $?"; fi

    ${ADF_CTL} status

}

stop() {

    qat_started

    if [ $? -eq 0 ]; then

        ${ADF_CTL} down
        rmmod  usdm_drv
        rmmod  qat_api
        rmmod  qat_c3xxx
        rmmod  intel_qat
        umount ${HUGE_PAGE_DIR}

    fi

}

restart() {

    ${ADF_CTL} restart

}

