### device-tree
```C
&mdio {
	rtk_gpio: rtl837x-gsw@1 {
		compatible = "realtek,rtl837x";
		#address-cells = <1>;
		#size-cells = <1>;
		reg = <29>;
		reset-gpios = <&pio 42 GPIO_ACTIVE_HIGH>; // optional
		rtl837x,cpu-port = <0>;
		rtl837x,sds0mode = "1000base-x";
		ethernet = <&gmac0>;
		sfp = <&sfp1>; // optional
		sds0-rx-swap; // optional
		// sds0-tx-swap; // optional
		sds1-rx-swap; // optional
		// sds1-tx-swap; // optional
		phy-mdi-reverse; // optional
		phy-tx-polarity-swap; // optional
		gpio-controller; // optional
		#gpio-cells = <2>; // optional
		// base = <0>; optional
	};
};
```

### TODO
- ~~添加上行口连接状态检查  目前重启后有概率无法和上行口建立连接~~
- 玄学CPU口在关闭后再开启有概率无法连接
- 添加i2c驱动支持
