# luci-theme-argon
A new Luci theme for LEDE/OpenWRT  
Argon is a clean HTML5 theme for LuCI. It is based on luci-theme-material and Argon Template  
Suitable for Openwrt 18.06.4 (for test) And Lean Luci (stable)  

The old version is still in another branch call old. If you need that you can checkout that branch.


## How to use

Enter in your openwrt/package/lean  or  other

```
git clone https://github.com/jerrykuku/luci-theme-argon.git
make menuconfig #choose LUCI->Theme->Luci-theme-argon
make -j1 V=s
```
## Update log 20191109
1. Minimal background file size.
2. Automatically detect the number of background images.
3. Delete unused image and file.

## Screenshots
![](/Screenshots/pc/screenshot1.png)
![](/Screenshots/pc/screenshot2.png)

<img src="/Screenshots/phone/Screenshot_1.jpg" width="375" height="792">
<img src="/Screenshots/phone/Screenshot_2.jpg" width="375" height="792">
<img src="/Screenshots/phone/Screenshot_3.jpg" width="375" height="792">




## Thanks to 
luci-theme-material: https://github.com/LuttyYang/luci-theme-material/
