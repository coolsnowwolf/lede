[English](README-en.md) ←

# 借助 GitHub Actions 的 OpenWrt 在线集成自动编译.

## [![Build](https://img.shields.io/github/workflow/status/KFERMercer/OpenWrt/OpenWrt-CI/master?)](https://github.com/KFERMercer/OpenWrt/actions?query=workflow%3AOpenWrt-CI) [![Release](https://img.shields.io/github/release/KFERMercer/OpenWrt-CI?color=blue)](https://github.com/KFERMercer/OpenWrt-CI/releases)

支持自动定制固件, 自动调整依赖及生成配置文件, 无需上传配置. 兼容 [coolsnowwolf/lede](https://github.com/coolsnowwolf/lede) 以及 OpenWrt trunk.

同时支持自动合并推送上游提交 (也就是自动更新), 直接把`merge-upstream.yml`放入`.github/workflows/`即可 (默认上游为 coolsnowwolf/lede, 高级玩家请自行改写).

感谢[P3TERX](https://github.com/P3TERX/Actions-OpenWrt)珠玉在前.

---

> ### 这个CI脚本和[P3TERX/Actions-OpenWrt](https://github.com/P3TERX/Actions-OpenWrt)的同样是云编译, 有什么不同?

这个 CI 脚本可以帮助你在你的 OpenWrt 分支下构建当前库的Op固件, 无需上传配置文件, 不依赖外挂配置脚本, 真正的一次配置终生使用.

P3TERX/Actions-OpenWrt 的优势在于, 其可以独立于文件库存在, 可以快速搭建起可用的编译配置; 同时对新手极其友好. 但是由于 OpenWrt make configure 自身机制的缘故, 生成的配置文件鲁棒性较差, 需要经常重新生成上传配置, 有形中加重了维护负担, 使得使用起来不是很`优雅`, **但是强烈建议基础薄弱的玩家使用.**

> ### 这个CI脚本适合哪些人?

- 正在维护自己的 OpenWrt 分支的大佬/初学者/玩家 (可以快速测试自己的代码)
- 伸手党
- 没有精力维持一个专用编译机的佛系人士

## 使用教程:

### 在一切开始前, 你需要的是:

- GitHub 账号
- 申请使用 GitHub Actions
- [基本的Git技能](https://www.liaoxuefeng.com/wiki/896043488029600)
- 自己的OpenWrt分支 ([Lean源](https://github.com/coolsnowwolf/lede)或者[官方源](https://github.com/openwrt/openwrt/))
- 脑子

### 相关文章: 作者-糖炒栗子 

- [面向小白的Github_Action使用workflow自动编译lean_openwrt教程](https://zhuanlan.zhihu.com/p/94402324)
- [关于Github Action自动编译Lean_Openwrt的配置修改问题](https://zhuanlan.zhihu.com/p/94527343)

### 1. 注册GitHub账号并开启GitHub Actions (自行搜索方法).

### 2. fork [coolsnowwolf/lede](https://github.com/coolsnowwolf/lede) 或者 [OpenWrt trunk](https://github.com/openwrt/openwrt).

### 3. 上传`openwrt-ci.yml`和`merge-upstream.yml`到`/.github/workflows/`下.

### 4. 定制固件:

> 如果你希望定制你的固件:

代码里的注释部分详细介绍了如何在脚本中客制化你的固件. 简单来说, 你只需要解除注释相应行即可.

我已经准备一个[正确的编写示例](https://github.com/KFERMercer/OpenWrt-by-lean/blob/CI-demo/.github/workflows/openwrt-ci.yml), 此配置基于 Lean 最新代码, 开启了所有预先写好的定制选项, 同时已经[经过测试](https://github.com/KFERMercer/OpenWrt-by-lean/commit/d31390d3e7b5f178d4e3456d401ded557c207398/checks?check_suite_id=334570354)可用. 如果你缺乏对自己的信心, 可以随意借鉴.

### 5. 大功告成.

集成编译环境会自动开始编译. 现在返回你的库首页, 点击页面上方的`Actions`按钮就可以查看进度.

> ### 如何下载到编译完成的固件?

进入`Actions`标签页后, 如果相应的集成活动顺利完成 (显示为绿色), 点击页面右上方的`Artifacts`即可看到你的固件 (通常是一个压缩包). 点击即可开始下载.
