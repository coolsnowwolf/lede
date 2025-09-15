#!/bin/sh /etc/rc.common

# Adapted from Intel QAT1.5 qat_service. Portions copyright Intel Corporation

#################################################################
#
#   This file is provided under a dual BSD/GPLv2 license.  When using or 
#   redistributing this file, you may do so under either license.
# 
#   GPL LICENSE SUMMARY
# 
#   Copyright(c) 2007-2013 Intel Corporation. All rights reserved.
# 
#   This program is free software; you can redistribute it and/or modify 
#   it under the terms of version 2 of the GNU General Public License as
#   published by the Free Software Foundation.
# 
#   This program is distributed in the hope that it will be useful, but 
#   WITHOUT ANY WARRANTY; without even the implied warranty of 
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
#   General Public License for more details.
# 
#   You should have received a copy of the GNU General Public License 
#   along with this program; if not, write to the Free Software 
#   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
#   The full GNU General Public License is included in this distribution 
#   in the file called LICENSE.GPL.
# 
#   Contact Information:
#   Intel Corporation
# 
#   BSD LICENSE 
# 
#   Copyright(c) 2007-2013 Intel Corporation. All rights reserved.
#   All rights reserved.
# 
#   Redistribution and use in source and binary forms, with or without 
#   modification, are permitted provided that the following conditions 
#   are met:
# 
#     * Redistributions of source code must retain the above copyright 
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright 
#       notice, this list of conditions and the following disclaimer in 
#       the documentation and/or other materials provided with the 
#       distribution.
#     * Neither the name of Intel Corporation nor the names of its 
#       contributors may be used to endorse or promote products derived 
#       from this software without specific prior written permission.
# 
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
#   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
#   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
#   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
#   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
#   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
#   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
#   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
#   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
#   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
#   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# 
#  version: QAT1.5.L.1.10.0-80
#
#################################################################

# qat          Start/Stop the Intel QAT.
#
# 		description: modprobe the QAT icp_qa_al.ko, which loads dependant \
#       modules, before calling the user space \
#       utility to pass configuration parameters

START=29
STOP=99

PROG=/usr/sbin/adf_ctl
KMOD=icp_qa_al
NETKEY=icp_qat_netkey.ko

EXTRA_COMMANDS="status"
EXTRA_HELP="        status Show the status of the qat device"
 
status() {

	${PROG} status
    if [ "$?" -ne 0 ]
    then
        echo "No devices found. Please start the driver using:"
        echo "$0 start"
    fi

}

start() {

    # First check if the modules are already installed
    # install them as necessary and if they are LKMs
    # and not built-in kernel modules

    if [ `lsmod | grep -c "sha512"` == 0 ]; then
         if [ `cat /proc/kallsyms |grep -c sha512_generic` == 0 ]; then
             `modprobe sha512`
         fi
    fi

    if [ `lsmod | grep -c "sha256"` == 0 ]; then
         if [ `cat /proc/kallsyms |grep -c sha256_generic` == 0 ]; then
             `modprobe sha256`
         fi
    fi

    lsmod | grep ${KMOD} >/dev/null 2>&1 || modprobe ${KMOD}

    # Check device status, try to turn it on only if driver is loaded

    ${PROG} $2 status | grep state=down >/dev/null 2>&1 
    if [ $? = 0 ]; then
        ${PROG} $2 up
    fi

    # lsmod | grep ${NETKEY} >/dev/null 2>&1 || modprobe ${NETKEY} 2> /dev/null

    # Show device status

    ${PROG} $2 status
}

stop() {

	${PROG} $2 down

}

restart() {

    ${PROG} $2 down && ${PROG} $2 up

}

