#!/bin/sh /etc/rc.common

# start prio
START=50
# use proc to start service
USE_PROCD=1
# package name
PACKAGE_NAME="acc.ipk"
# binary dir
sbin_dir="/usr/sbin/leigod"
# binary name
binary_prefix="acc-gw.linux"


# download acceleration binary
download_acc_binary() {
    echo "begin to download file... ..."
    # create download 
    local url="http://119.3.40.126/${PACKAGE_NAME}"
    echo "download path: ${url}"
    local path="/tmp/${PACKAGE_NAME}"
    local code=$(curl --connect-timeout 2 -L -s -k -o "${path}" -w "%{http_code}" "${url}")
    for i in $(seq 1 2)  
    do
        if [ "$?" != "0" -o "${code}" != "200" -o ! -f "${path}" ]; then
            echo "download failed: ${code}"
            rm -f ${path}
            sleep 5
            continue
        fi
        echo "download success"
        break
    done

    echo "download file success... ..."
}

# mkdir acc dir
mk_acc_dir() {
    echo "begin to mkdir... ..."
    # should make sure dir exist
    local cmd="mkdir -p ${BINARY_DIR}"
    echo "exec mkdir dir: ${cmd}"
    local code=`$cmd`
    echo "mkdir success... ..."
}

# decompress file
decompress_file() {
    echo "begin to decompress file... ..."
    # compress file path
    local path="/tmp/${PACKAGE_NAME}"
    # decompress command
    local cmd="tar -zxvf ${path} -C ${BINARY_DIR}"
    local code=`${cmd}`
    echo "decompress file success... ..."
}

# install ipk
install_package() {
  echo "begin to overwrite package"
  # local cmd = "opkg install /tmp/${PACKAGE_NAME} --force-overwrite"
  # `$cmd` 
  echo "overwrite finish, begin to start"
}

# main
prepare_env() {
    # mkdir 
    mk_acc_dir
    # download file
    download_acc_binary
    # decompress file
    install_package
}

main() {
    prepare_env
}

# get_device_arch, 
# current support arch: arm64 arm x86_64 mips 
get_device_arch() {
  arch=$(uname -m)
  if [ $? != "0" ]; then
    echo "arch cant be get"
    return -1
  fi
  # fix arch 
  if [ ${arch} == "x86_64" ]; then
    echo "match x86_64 -> amd64"
    arch="amd64"
  elif [ ${arch} == "aarch64" ]; then
    echo "match aarch64 -> arm64"
    arch="arm64"
  elif [ ${arch} == "mips" ]; then
    echo "match mips -> mipsle"
    arch="mipsle"
  elif [ ${arch} == "armv7l" ]; then
    echo "match armv7l -> arm"
    arch="arm"
  fi
  return 0
}

# get_xiaomi_name check if is xiaomi 
get_xiaomi_name() {
  local name=$(uci get misc.hardware.displayName)
  if [[ $? == "0" && ${name} != "" ]]; then
    echo "router is xiaomi series, name: ${name}"
    sbin_dir="/userdisk/appdata/leigod"
    init_dir="/userdisk/appdata/leigod"
    args="--mode tun"
    is_xiaomi=true
    return 0
  fi
  local name=$(uci get misc.hardware.model)
  if [[ $? == "0" && ${name} != "" ]]; then
    echo "router is xiaomi series, name: ${name}"
    sbin_dir="/userdisk/appdata/leigod"
    init_dir="/userdisk/appdata/leigod"
    args="--mode tun"
    is_xiaomi=true
    return 0
  fi
  echo "router is not xiaomi, use general openwrt"
  sbin_dir="/usr/sbin/leigod"
  init_dir="/etc/init.d"
  return 0
}

# run service as daemon
start_service_daemon() {
    # open once instance
    procd_open_instance
    # run binary
    local cmd="${sbin_dir}/${binary_prefix}.${arch} ${args}"
    echo "start service: ${cmd}"
    procd_set_param command ${cmd}
    # auto start when failed
    procd_set_param respawn
    # close instance
    procd_close_instance
    # start check update
    # update_plugin &
}


# proc start service
start_service() {

    config_load "accelerator"
    
    local enabled
    config_get_bool enabled base enabled 0
    
    local tun
    config_get_bool tun base tun 0

    [ "${enabled}" -gt 0 ] || return    
    [ "${tun}" -eq 1 ] && args="--mode tun"

    get_device_arch
    start_service_daemon
}

