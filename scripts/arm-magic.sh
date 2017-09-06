#!/usr/bin/env bash
#
#   Empty/wrong machtype-workaround generator
#
#   Copyright (C) 2006-2012 Imre Kaloz <kaloz@openwrt.org>
#   based on linux/arch/arm/boot/compressed/head-xscale.S
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# NOTE: for now it's for only IXP4xx in big endian mode

# list of supported boards, in "boardname machtypeid" format
for board in "avila 526" "gateway7001 731" "nslu2 597" "nas100d 865" "wg302v1 889" "wg302v2 890" "pronghorn 928" "pronghornmetro 1040" "compex 1273" "wrt300nv2 1077" "loft 849" "dsmg600 964" "fsg3 1091" "ap1000 1543" "tw2662 1658" "tw5334 1664" "ixdpg425 604" "cambria 1468" "sidewinder 1041" "ap42x 4418"
do
  set -- $board
  hexid=$(printf %x\\n $2)
  if [ "$2" -lt "256" ]; then
    # we have a low machtypeid, we just need a "mov" (e3a)
    printf "\xe3\xa0\x10\x$hexid" > $BIN_DIR/$IMG_PREFIX-$1-zImage
  else
    # we have a high machtypeid, we need a "mov" (e3a) and an "orr" (e38)
    if [ "$2" -lt "4096" ]; then
      printf "\xe3\xa0\x10\x$(echo $hexid|cut -b "2 3")\xe3\x81\x1c\x$(echo $hexid|cut -b 1)" > $BIN_DIR/$IMG_PREFIX-$1-zImage
    else
      printf "\xe3\xa0\x10\x$(echo $hexid|cut -b "3 4")\xe3\x81\x1c\x$(echo $hexid|cut -b "1 2")" > $BIN_DIR/$IMG_PREFIX-$1-zImage
    fi
  fi
    # generate the image
    cat $BIN_DIR/$IMG_PREFIX-zImage >> $BIN_DIR/$IMG_PREFIX-$1-zImage
done
