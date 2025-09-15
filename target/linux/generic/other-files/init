#!/bin/sh
# Copyright (C) 2006 OpenWrt.org
export INITRAMFS=1

# switch to tmpfs to allow run daemons in jail on initramfs boot
DIRS=$(echo *)
NEW_ROOT=/new_root

mkdir -p $NEW_ROOT
mount -t tmpfs tmpfs $NEW_ROOT

cp -pr $DIRS $NEW_ROOT

exec switch_root $NEW_ROOT /sbin/init
