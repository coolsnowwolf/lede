#!/bin/sh
JOBS=`grep -c ^processor /proc/cpuinfo`
cat ./HATLab-GateBoard-One.feeds > ./feeds.conf.default
./scripts/feeds update -a

rm -rf ./feeds/packages/net/smartdns/
git clone https://github.com/pymumu/openwrt-smartdns ./feeds/packages/net/smartdns/
rm -rf ./feeds/luci/applications/luci-app-smartdns/
git clone https://github.com/pymumu/luci-app-smartdns ./feeds/luci/applications/luci-app-smartdns/

./scripts/feeds install -a
rm -f ./.config*
touch ./.config
cat ./HATLab-GateBoard-One.config > ./.config
make defconfig
make download -j${JOBS}
make -j${JOBS
