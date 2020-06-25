# openwrt-adguardhome

[![Last commit](https://img.shields.io/github/last-commit/KFERMercer/openwrt-adguardhome)](https://github.com/KFERMercer/openwrt-adguardhome/commits/master)

---

Openwrt 可用的 [AdGuardHome](https://github.com/AdguardTeam/AdGuardHome).

## 使用方法

启动:

```shell
adguardhome &
```

停止:

```shell
killall -p adguardhome
```

## 注意

- 此软件包不是 AdGuardHome 的 LuCI 插件, 想要在 LuCI 中配置运行 AdGuardHome, 请点击[此处](https://github.com/KFERMercer/luci-app-adguardhome).

- 代码二次发布时:
  - 请沿袭我的 Release 版本号. 不要另行开始计数, 以免造成版本混乱.
  - 请注明此仓库链接.

---

[AdGuardHome](https://github.com/AdguardTeam/AdGuardHome) for OpenWrt.

## Usage:

start program:

```shell
adguardhome &
```

stop:

```shell
killall -p adguardhome
```

## Precautions

- This package is not AdGuardHome's LuCI interface. Click [here](https://github.com/KFERMercer/luci-app-adguardhome) If you want configure AdGuardHome from LuCI.

- Suggest to follow my release counting, so as not to cause version confusion.
