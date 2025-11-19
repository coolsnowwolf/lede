# this script is use to install leigod plugin
# sbin_dir="/usr/sbin/leigod"
ver_name="version"
init_file_name="acc"
binary_prefix="acc-gw.linux"
common_file_name="plugin_common.sh"
uninstall_file_name="leigod_uninstall.sh"
download_base_url="http://119.3.40.126/router_plugin"

# get_device_os
# current support os: Linux
get_device_os() {
  os=$(uname)
  if [ $? == "0" ]; then
    return 0
  fi
  echo "os cant be get"
  return -1
}

# get_device_arch, 
# current support arch: arm64 arm x86_64 mips 
get_device_arch() {
  arch=$(uname -m)
  if [ $? == "0" ]; then
    return 0
  fi
  echo "arch cant be get"
  return -1
}

# get_xiaomi_name check if is xiaomi 
get_xiaomi_name() {
  local name=$(uci get misc.hardware.displayName)
  if [[ $? == "0" && ${name} != "" ]]; then
    echo "router is xiaomi series, name: ${name}"
    sbin_dir="/userdisk/appdata/leigod"
    init_dir="/userdisk/appdata/leigod"
    is_xiaomi=true
    return 0
  fi
  local name=$(uci get misc.hardware.model)
  if [[ $? == "0" && ${name} != "" ]]; then
    echo "router is xiaomi series, name: ${name}"
    sbin_dir="/userdisk/appdata/leigod"
    init_dir="/userdisk/appdata/leigod"
    is_xiaomi=true
    return 0
  fi

  local name=$(uci get misc.hardware.model)
  if [[ $? == "0" && ${name} != "" ]]; then
    echo "router is xiaomi series, name: ${name}"
    sbin_dir="/userdisk/appdata/leigod"
    init_dir="/userdisk/appdata/leigod"
    is_xiaomi=true
    return 0
  fi
  echo "router is not xiaomi, use general openwrt"
  sbin_dir="/usr/sbin/leigod"
  init_dir="/etc/init.d"
  # stop openwrt service first
  echo "stop openwrt acc service first, in casue install failed"
  /etc/init.d/acc stop
  sleep 1
  show_openwrt_suggestion
  return 0
}

# get_asus_name get asus name
get_merlin_party() {
  if [[ -d "/koolshare" ]]; then
    echo "router is merlin series, name: $(nvram get build_name)"
    nvram set 3rd-party=merlin
    is_merlin=true
    sbin_dir="/koolshare/leigod/acc"
    init_dir="/koolshare/init.d"
    echo "message"
    return 0
  elif [[ -d "/jffs/softcenter" ]]; then
    echo "router is swrt series, name: $(nvram get build_name)"
    is_swrt=true
    nvram set 3rd-party=swrt
    sbin_dir="/jffs/softcenter/leigod/acc"
    init_dir="/jffs/softcenter/init.d"
    return 0
  fi
  # check merlin
  echo "route is not merlin, use general asus"
  return 0
}

# get_device_firmware get device firmware
# current support firmware: openwrt merlin
get_device_firmware() {
  # openwrt file exist
  if [ -f "/etc/openwrt_release" ]; then
    echo "firmware is openwrt series"
    is_openwrt=true
    get_xiaomi_name
  elif [[ -f "/etc/image_version" ]] || [[ -d "/koolshare" ]] || [[ -d "/jffs/softcenter" ]]; then
    echo "firmware is asus series"
    is_asus=true
    echo "stop asus acc service first, in casue install failed"
    PIDS=$(ps | grep acc | grep -v grep | awk '{print $1}')
    # check if process exist
    get_merlin_party
  fi
}

# install_openwrt_package install openwrt 
install_binary() {
  # create sbin dir 
  mkdir -p ${sbin_dir}
  # create name 
  local acc_name=${binary_prefix}.${arch}
  local download_bin_url=${download_base_url}/${acc_name}
  echo "install ${acc_name} to ${sbin_dir}"
  # download file 
  curl -s -o ${sbin_dir}/${acc_name} ${download_bin_url}
  if [ $? != "0" ]; then
    echo "download and install binary failed"
    return -1
  fi
  echo "install binary success"
  chmod +x ${sbin_dir}/${acc_name}
  if [ $? != "0" ]; then
    echo "add binary permission failed"
    return -1
  fi
  echo "add acc binary permission success"
  # download common file 
  local download_common_url=${download_base_url}/${common_file_name}
  curl -s -o ${sbin_dir}/${common_file_name} ${download_common_url}
  if [ $? != "0" ]; then
    echo "download and install common file failed"
    return -1
  fi

  # remote uninstall_file_name
  local remote_uninstall_file_name=${download_base_url}/"plugin_uninstall.sh"
  curl -s -o ${sbin_dir}/${uninstall_file_name} ${remote_uninstall_file_name}
  if [ $? != "0" ]; then
    echo "download and install uninstall file failed"
    return -1
  fi
  echo "add uninstall permission success"
  local ver_file=${sbin_dir}/${ver_name}
  touch ${ver_file}
  if [ $? != "0" ]; then
    echo "create version file failed"
    return -1
  fi
  # add version to file 
  echo "version=1.3.0.30" > ${ver_file}
  echo "add version file success"
}

# remove_binary remove binary
remove_binary() {
  rm -r ${sbin_dir}
}

# install xiaomi monitor 
install_xiaomi_monitor() {
  local cron_path="/etc/crontabs/root"
  local monitor_file_name="plugin_monitor.sh"
  local download_monitor_url=${download_base_url}/${monitor_file_name}
  curl -o ${sbin_dir}/${monitor_file_name} ${download_monitor_url}
  if [ $? != "0" ]; then
    echo "download monitor file failed"
    return -1
  fi
  chmod +x ${sbin_dir}/${monitor_file_name}
  # download 
  echo "download monitor file success"
  echo "*/1 * * * * ${sbin_dir}/${monitor_file_name}" >> ${cron_path}
  if [ $? != "0" ]; then
    echo "add monitor to cron failed"
    return -1
  fi
  echo "add monitor to cron success"
}

# install_openwrt_series_config save openwrt config 
install_openwrt_series_config() {
  # create accelerator config
  touch /etc/config/accelerator
  if [ $? != "0" ]; then
    echo "make acc config file failed"
    return -1
  fi
  if [ ${install_env} == "test" ]; then
    # use uci to add config 
    uci set accelerator.base=system
    uci set accelerator.bind=bind
    uci set accelerator.device=hardware
    uci set accelerator.Phone=acceleration
    uci set accelerator.PC=acceleration
    uci set accelerator.Game=acceleration
    uci set accelerator.Unknown=acceleration
    uci set accelerator.base.url='https://test-opapi.nn.com/speed/router/plug/check'
    uci set accelerator.base.heart='https://test-opapi.nn.com/speed/router/heartbeat'
    uci set accelerator.base.base_url='https://test-opapi.nn.com/speed'
    uci commit accelerator
  elif [ ${install_env} == "test1" ]; then
    # use uci to add config 
    uci set accelerator.base=system
    uci set accelerator.bind=bind
    uci set accelerator.device=hardware
    uci set accelerator.Phone=acceleration
    uci set accelerator.PC=acceleration
    uci set accelerator.Game=acceleration
    uci set accelerator.Unknown=acceleration
    uci set accelerator.base.url='https://test1-opapi.nn.com/speed/router/plug/check'
    uci set accelerator.base.heart='https://test1-opapi.nn.com/speed/router/heartbeat'
    uci set accelerator.base.base_url='https://test1-opapi.nn.com/speed'
    uci commit accelerator
  else
    # use uci to add config 
    uci set accelerator.base=system
    uci set accelerator.bind=bind
    uci set accelerator.device=hardware
    uci set accelerator.Phone=acceleration
    uci set accelerator.PC=acceleration
    uci set accelerator.Game=acceleration
    uci set accelerator.Unknown=acceleration
    uci set accelerator.base.url='https://opapi.nn.com/speed/router/plug/check'
    uci set accelerator.base.heart='https://opapi.nn.com/speed/router/heartbeat'
    uci set accelerator.base.base_url='https://opapi.nn.com/speed'
    uci commit accelerator
  fi
  if [ $? != "0" ]; then
    echo "create openwrt config unit failed"
    return -1
  fi
  echo "create openwrt config unit success"
}

# install_openwrt_series_luasrc install openwrt lua src
install_openwrt_series_luasrc() {
  lua_base="/usr/lib/lua/luci"
  # download index file
  curl --create-dirs -o ${lua_base}/controller/acc.lua ${download_base_url}/openwrt/controller/acc.lua
  if [ $? != "0" ]; then
    echo "download acc.lua failed"
    return -1
  fi
  # download service view file
  curl --create-dirs -o ${lua_base}/model/cbi/leigod/service.lua ${download_base_url}/openwrt/model/cbi/leigod/service.lua
  if [ $? != "0" ]; then
    echo "download service.lua failed"
    return -1
  fi
  # download device view file
  curl --create-dirs -o ${lua_base}/model/cbi/leigod/device.lua ${download_base_url}/openwrt/model/cbi/leigod/device.lua
  if [ $? != "0" ]; then
    echo "download device.lua failed"
    return -1
  fi
  # download notice view file
  curl --create-dirs -o ${lua_base}/model/cbi/leigod/notice.lua ${download_base_url}/openwrt/model/cbi/leigod/notice.lua
  if [ $? != "0" ]; then
    echo "download notice.lua failed"
    return -1
  fi
  # download service view file
  curl --create-dirs -o ${lua_base}/view/leigod/notice.htm ${download_base_url}/openwrt/view/leigod/notice.htm
  if [ $? != "0" ]; then
    echo "download notice.htm failed"
    return -1
  fi
  # download service view file
  curl --create-dirs -o ${lua_base}/view/leigod/service.htm ${download_base_url}/openwrt/view/leigod/service.htm
  if [ $? != "0" ]; then
    echo "download service.htm failed"
    return -1
  fi
  # download service translate file
  curl --create-dirs -o ${lua_base}/i18n/acc.zh-cn.lmo ${download_base_url}/openwrt/po/zh-cn/acc.zh-cn.lmo
  if [ $? != "0" ]; then
    echo "download acc.zh-cn.lmo failed"
    return -1
  fi
  echo "download lua src success"
}

install_openwrt_series_web() {
  local luci_base="/usr/lib/lua/luci"
  
}

# remove_openwrt_series_config remove openwrt config 
remove_openwrt_series_config() {
  rm /etc/config/accelerator
}

# install asus series config 
install_asus_series_config() {
  mkdir -p ${sbin_dir}/config
  if [ ${install_env} == "test" ]; then
  # install test asus config
  echo "
[base]
url="https://test-opapi.nn.com/speed/router/plug/check"
channel="2"
appid="nnMobile_d0k3duup"
heart="https://test-opapi.nn.com/speed/router/heartbeat"
base_url="https://test-opapi.nn.com/speed"

[update]
domain="https://test-opapi.nn.com/nn-version/version/plug/upgrade"

[device]
  " > ${sbin_dir}/config/accelerator
  elif [ ${install_env} == "test1" ]; then
  # install test1 asus config
  echo "
[base]
url="https://test1-opapi.nn.com/speed/router/plug/check"
channel="2"
appid="nnMobile_d0k3duup"
heart="https://test1-opapi.nn.com/speed/router/heartbeat"
base_url="https://test1-opapi.nn.com/speed"

[update]
domain="https://test1-opapi.nn.com/nn-version/version/plug/upgrade"

[device]
  " > ${sbin_dir}/config/accelerator
  else
  # install formel asus config
  echo "
[base]
url="https://opapi.nn.com/speed/router/plug/check"
channel="2"
appid="nnMobile_d0k3duup"
heart="https://opapi.nn.com/speed/router/heartbeat"
base_url="https://opapi.nn.com/speed"

[update]
domain="https://opapi.nn.com/nn-version/version/plug/upgrade"

[device]
  " > ${sbin_dir}/config/accelerator

  fi

  echo "create asus series config success"
}

# remove_asus_series_config remove asus config 
remove_asus_series_config() {
  rm -r ${init_dir}
}

# install_openwrt_init install openwrt to init 
install_openwrt_series_init() {
  local remote_init_name="openwrt_init.sh"
  local download_init_url=${download_base_url}/${remote_init_name}
  # download init file 
  curl -o ${init_dir}/${init_file_name} ${download_init_url}
  if [ $? != "0" ]; then
    echo "download init file failed"
    return -1
  fi
  echo "download init file success"
  #  add permission to file 
  chmod +x ${init_dir}/${init_file_name}
  if [ $? != "0" ]; then
    echo "add init permission failed"
    return -1
  fi
  echo "add init file permission success"
  ${init_dir}/${init_file_name} enable
  echo "set accelerator autostart success"
  ${init_dir}/${init_file_name} start
  if [ $? != "0" ]; then
    echo "start accelerator failed"
    return -1 
  fi
  echo "start accelerator success"
}

# remove_openwrt_series_init remove openwrt init
remove_openwrt_series_init() {
  ${init_dir}/${init_file_name} disable
  ${init_dir}/${init_file_name} stop
  rm ${init_dir}/${init_file_name}
}

# install merlin init 
install_merlin_init() {
  local remote_init_name="asus_init.sh"
  local download_init_url=${download_base_url}/${remote_init_name}
  # download init file 
  curl -o ${sbin_dir}/${init_file_name} ${download_init_url}
  if [ $? != "0" ]; then
    echo "download init file failed"
    return -1
  fi
  # add permission 
  chmod +x ${sbin_dir}/${init_file_name}
  # create link 
  local link_init_name="S99LeigodAcc.sh"
  local link_init_file=${init_dir}/${link_init_name}
  ln -sf ${sbin_dir}/${init_file_name} ${link_init_file}
  if [ $? != "0" ]; then
    echo "create merlin init link failed"
    return -1
  fi
  echo "create merlin link file success"
  ${link_init_file} start
  echo "acc start success"
}

# show_openwrt_suggestion show openwrt install suggest
show_openwrt_suggestion() {
  echo "
  雷神OpenWrt插件安装建议:

  当前雷神路由器支持两种加速模式,
  1. tproxy加速模式(速度更快, CPU占用率更低)
  2. tun加速模式(需要依赖少, 安装灵活)
  
  需要您根据以上的加速模式, 安装对应的依赖库, 
  如下列出两种模式对应的安装依赖: 
  1. TProxy模式:  libpcap iptables kmod-ipt-nat iptables-mod-tproxy kmod-ipt-tproxy kmod-netem(可选) tc-full(可选) kmod-ipt-ipset ipset curl
  2. Tun模式:     libpcap iptables kmod-tun kmod-ipt-nat kmod-ipt-ipset ipset curl

  如何安装依赖:
  1. 升级依赖:   opkg update 
  2. 安装依赖:   opkg install xxx

  为了安装方便, 请选择一个模式, 复制以下命令到终端运行: 
  Tproxy模式: 
  opkg update 
  opkg install libpcap iptables kmod-ipt-nat iptables-mod-tproxy kmod-ipt-tproxy kmod-netem tc-full kmod-ipt-ipset ipset

  Tun模式:
  opkg update 
  opkg install libpcap iptables kmod-tun kmod-ipt-nat kmod-ipt-ipset ipset curl

  关于steamdeck的支持说明
  steamdeck设备请选择加速电脑游戏
  
  关于手机设备的支持:
  1. 安卓支持说明 
  当前代理仅支持ipv4代理, 请更改dhcp配置，更改完配置请重启路由器，
  配置路径在 /etc/config/dhcp 
  config dhcp 'lan'
    ... 此处是一些其他配置
    ra 'disable'
    dhcpv6 'disable'
    list ra_flags 'none'
    ... 此处是一些其他配置

  2. 关于ios设备的支持说明
  ios设备, 安装完插件后, 为了精准识别, 请在ios上选择忘记wifi, 然后重新连接即可
  "
}

# preinstall_check check env
preinstall_check() {
  # check os 
  get_device_os
  if [ ${os} != "Linux" ]; then
    echo "current os not support, os: ${os}"
    return -1
  fi
  # check arch
  get_device_arch
  if [[ ${arch} != "x86_64" && ${arch} != "aarch64" && ${arch} != "arm" && ${arch} != "mips" && ${arch} != "armv7l" ]];then
    echo "current arch not support, arch: ${arch}"
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
    arch="arm"
  fi
  # support plugin
  echo "current system support plugin, system: ${os}-${arch}"
  get_device_firmware
  return 0
}

# show_install_success show install has been installed 
show_install_success() {
  echo "install success"
  echo "雷神路由器插件安装已完成"
  echo "请加群632342113体验"
}

