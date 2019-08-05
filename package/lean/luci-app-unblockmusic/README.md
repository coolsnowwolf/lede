 - ### 欢迎加入我的Telegram群组：[@ctcgfw_openwrt_discuss](https://t.me/ctcgfw_openwrt_discuss)

# 说明
- 用于恢复网易云音乐无版权歌曲的OpenWRT/LEDE Luci插件
- 核心功能基于：[nondanee/UnblockNeteaseMusic](https://github.com/nondanee/UnblockNeteaseMusic)
- 基于[maxlicheng](https://github.com/maxlicheng/luci-app-unblockmusic)的Luci插件进行二改，使源项目代码更方便的在路由器上运行
- 温馨提示：本插件显式依赖node.js，如您的设备型号不支持，将无法使用

## 原理
- 其原理是采用 [网易云旧链/QQ/虾米/百度/酷狗/酷我/咕咪/JOOX] 等音源，替换网易云无版权歌曲链接
- 通俗地理解就是通过脚本，将主流客户端的音乐链接汇集到一个客户端上

## 编译
- 插件显式依赖node.js，OpenWRT/LEDE源码默认是v8.10.0版本，编译过程中下载会比较慢，可以提前将node-v8.10.0下载好上传到dl目录
- 下载链接(17.5M)：http://nodejs.org/dist/v8.10.0/node-v8.10.0.tar.xz  
```bash
    #进入OpenWRT/LEDE源码package目录
    cd package
    #克隆插件源码
    git clone https://github.com/project-openwrt/luci-app-unblockmusic.git
    #返回上一层目录
    cd ..
    #配置
    make menuconfig
    #在luci->application选中插件,编译
    make package/luci-app-unblockmusic/compile V=99
```

## 使用方法
### 1.路由器web界面插件配置
- a.在路由器web界面“服务”选项中找到“恢复网易云音乐无版权歌曲”
- b.勾选“启用UnblockNeteaseMusic”
- c.“音源接口”选择“默认”
- d.勾选“启用IPSet劫持相关请求”
- e.点击“保存&应用”
- 现在您局域网下的所有设备，无需任何设置即可正常播放网易云音乐无版权歌曲

## 效果图
### luci界面
  ![Image text](https://raw.githubusercontent.com/project-openwrt/luci-app-unblockmusic/master/views/views1.jpg)
  ![Image text](https://raw.githubusercontent.com/project-openwrt/luci-app-unblockmusic/master/views/views2.jpg)
  ![Image text](https://raw.githubusercontent.com/project-openwrt/luci-app-unblockmusic/master/views/views3.jpg)
### UWP网易云音乐客户端
  ![Image text](https://raw.githubusercontent.com/project-openwrt/luci-app-unblockmusic/master/views/views4.jpg)

## 协议
### [核心功能\(UnblockNeteaseMusic\)](https://github.com/nondanee/UnblockNeteaseMusic)使用[MIT](https://github.com/nondanee/UnblockNeteaseMusic/blob/master/LICENSE)协议
### [源项目](https://github.com/maxlicheng/luci-app-unblockmusic)使用[Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0)协议
### [本项目](https://github.com/project-openwrt/luci-app-unblockmusic)使用[GNU General Public License v3.0](https://github.com/project-openwrt/luci-app-unblockmusic/blob/master/LICENSE)协议
