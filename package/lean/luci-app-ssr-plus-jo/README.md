
# luci-app-ssr-plus

This is a backup repoistory of luci-app-ssr-plus from Lean's OpenWrt project package.

Lean's OpenWrt source:

<https://github.com/coolsnowwolf/lede/>

Source from tree: 2915c44a11ca0ee40b51ff5d9c18a0da1951e170

Lean's luci-app-ssr-plus source (history):

<https://github.com/coolsnowwolf/lede/tree/2915c44a11ca0ee40b51ff5d9c18a0da1951e170/package/lean/luci-app-ssr-plus>

# luci-app-ssr-plus-jo

## 说明
   源码来源：https://github.com/coolsnowwolf
   
1.免开门

2.增加图标检查

3.增加ping延迟

4.udp2raw/UDPspeeder 支持

5.已适配官方master/19.07
## 使用方法
```Brach
    #源码根目录，进入package文件夹
    cd package
    #下载源码
    git clone https://github.com/Ameykyl/luci-app-ssr-plus-jo
    # 依赖包
    git clone https://github.com/Ameykyl/my
    #回到源码根目录
    cd ..
    make menuconfig
    #编译
    make package/luci-app-ssr-plus-jo/{clean,compile} V=s
    
    


