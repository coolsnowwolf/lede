## 不成熟作品，已弃，但可以正常使用
合并watchcat和luci-app-mac

添加：开机启动修改mac，断网重连，断网重连并修改mac，周期性重连，周期性重连并修改mac

日志参照了luci-app-privoxy来编写

如果直接安装ipk，不能正确启动，应该在/model/cbi/watchpig.lua加上
if apply then
     io.popen("/etc/init.d/watchpig restart")
end
