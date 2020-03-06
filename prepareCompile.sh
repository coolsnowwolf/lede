#/bin/sh

disablePkgsList="
./feeds/lienol/package/chinadns-ng 
./package/lean/baidupcs-web 
./package/lean/luci-app-baidupcs-web 
./feeds/packages/net/miniupnpd 
./feeds/packages/net/mwan3 
./feeds/packages/utils/ttyd
"

function disableDulicatedPkg()
{
	if [ -d $1 ];then
		rm -rf $1
		echo $1" Disabled."
	fi
}

./scripts/feeds update -a

for disablePkg in $disablePkgsList
do
	disableDulicatedPkg $disablePkg
done

./scripts/feeds update -i
./scripts/feeds install -a
