# luci-app-ssr-plus

## 说明

   此仓库只做个人备份用，如有需要请到插件原作者 [[Lean]](https://github.com/coolsnowwolf) 的 [[LEDE仓库]](https://github.com/coolsnowwolf/lede)进行反馈。

   源码来源：https://github.com/coolsnowwolf
   
   源码特色：
   - 1.免开门
   - 2.更新g-f-w列表，共5184条
   - 3.更新国内IP段，共8374条
   - 注：L大的lede源码已经重新上传此插件，建议使用L大的源。

## 使用方法一
```Brach
    #源码根目录，进入package文件夹
    cd package/
    #创建一个openwrt-packages
    mkdir openwrt-packages
    #进入新建的文件夹
    cd openwrt-packages
    #下载源码
    git clone https://github.com/maxlicheng/luci-app-ssr-plus.git
    #回到源码根目录
    cd ../..
    #拉取源码
    git pull
```

## 使用方法二
```Brach
    #源码根目录，编辑.gitignore文件
    vi .gitignore
    #在文件最后一行，加入
    git rm --cached package/lean/luci-app-ssr-plus/ -r
    #保存后，进入lean源码目录
    cd package/lean/
    #下载源码
    git clone https://github.com/maxlicheng/luci-app-ssr-plus.git
    #回到源码根目录
    cd ../..
    #拉取源码
    git pull
```

## 注意
 以上两种方法只能选其中一种，
采用任一方法后，ssr就不受git pull影响了。
