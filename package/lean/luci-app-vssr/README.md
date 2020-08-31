## luci-app-vssr [Hello World]
A new SSR SS V2ray Trojan luci app bese luci-app-ssr-plus  
<b>支持全部类型的节点分流</b>  
目前只适配最新版 argon主题 （其他主题下应该也可以用 但显示应该不会很完美）
目前Lean最新版本的openwrt 已经可以直接拉取源码到package/lean 下直接进行勾选并编译，由于有部分文件和ssr+ 同文件名所以不能同时编译。  

### 写在前面：
插件的初衷是优化操作体验和提升视觉感受，所以插件体积会比较大，并不适合小ROM机器使用。

还有些个人的想法，首先这只是个Luci app，说白了就是个GUI控制界面而已,本身并不能决定你设备的性能和节点的速度。  
决定你性能的东西是你的硬件方案 如cpu性能，是否支持硬件aes加速，还有就是几个主核心应用程序的版本。  
另外关于插件稳定性的问题，Luci除非自身有逻辑性的BUG，一般情况下是不会有稳定性差异的，  
稳定性的差异来自于你固件的内核，还有ss ssr v2ray 这几个核心插件的稳定性，当然你的节点才是影响稳定性的最大因素。  
Luci 能决定的只有操作起来是否便利、顺手，还有对几个核心应用功能的适配挖掘而已。

### Update Log 2020-08-30  v1.17-1

#### Updates

- FIX: 修复台台湾视频分流不起作用的问题。

详情见[具体日志](./relnotes.txt)。 

### Intro

1. 基于lean ssr+ 全新修改的Vssr（更名为Hello World） 主要做了很多的修改，同时感谢插件原作者所做出的的努力和贡献！ 
1. 节点列表支持国旗显示 TW节点为五星红旗， 节点列表页面 打开自动ping.  
1. 优化了在节点列表页面点击应用后节点切换的速度。同时也优化了自动切换的速度。  
1. 将节点订阅转移至 高级设置 请悉知 由于需要获取ip的国家code 新的订阅速度可能会比原来慢一点点 x86无影响。  
1. 给Hello World 增加了IP状态显示，在页面底部 左边显示当前节点国旗 ip 和中文国家 右边 是四个网站的访问状态  可以访问是彩色不能访问是灰色。  
1. 优化了国旗匹配方法，在部分带有emoji counrty code的节点名称中 优先使用 emoji code 匹配国旗。  
1. 建议搭配argon theme，能有最好的显示体验。  

新修改插件难免有bug 请不要大惊小怪。欢迎提交bug。

### How to use
假设你的lean openwrt（最新版本19.07） 在 lede 目录下
```
cd lede/package/lean/  

git clone https://github.com/jerrykuku/lua-maxminddb.git  #git lua-maxminddb 依赖

git clone https://github.com/jerrykuku/luci-app-vssr.git  

make menuconfig

make -j1 V=s
```

### 感谢
https://github.com/coolsnowwolf/lede

### My other project
Argon theme ：https://github.com/jerrykuku/luci-theme-argon

openwrt-nanopi-r1s-h5 ： https://github.com/jerrykuku/openwrt-nanopi-r1s-h5
