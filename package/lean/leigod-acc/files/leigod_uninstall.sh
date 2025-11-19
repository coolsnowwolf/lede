#!/bin/sh

# this script is use to install leigod plugin
ver_name="version"
init_file_name="acc"
binary_prefix="acc-gw.linux"
common_file_name="plugin_common.sh"
download_base_url="http://119.3.40.126/router_plugin"

# include common file 
. ${common_file_name}

# preinstall_check 
# check and set env
preinstall_check

if [ ${is_openwrt} ]; then
  echo "remove openwrt config"
  remove_openwrt_series_config
  remove_openwrt_series_init
fi

# remove_binary remove binary
remove_binary
