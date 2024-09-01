From 930d10b5a37004c428ad1c9747a1424e85567e3f Mon Sep 17 00:00:00 2001
From: jjm2473 <1129525450@qq.com>
Date: Wed, 15 Mar 2023 16:18:26 +0800
Subject: [PATCH] rockchip: add pwm-fan for R4S(E)

---
 .../boot/dts/rockchip/rk3399-nanopi-r4s.dts   | 36 +++++++++++++++++++
 .../boot/dts/rockchip/rk3399-nanopi-r4se.dts  | 36 +++++++++++++++++++
 2 files changed, 72 insertions(+)

--- a/arch/arm64/boot/dts/rockchip/rk3399-nanopi-r4s.dts
+++ b/arch/arm64/boot/dts/rockchip/rk3399-nanopi-r4s.dts
@@ -69,6 +69,42 @@
 		regulator-always-on;
 		regulator-boot-on;
 	};
+
+        fan: pwm-fan {
+                compatible = "pwm-fan";
+                cooling-levels = <0 18 102 170 255>;
+                #cooling-cells = <2>;
+                fan-supply = <&vdd_5v>;
+                pwms = <&pwm1 0 50000 0>;
+        };
+};
+
+&cpu_thermal {
+        trips {
+                cpu_warm: cpu_warm {
+                        temperature = <55000>;
+                        hysteresis = <2000>;
+                        type = "active";
+                };
+
+                cpu_hot: cpu_hot {
+                        temperature = <65000>;
+                        hysteresis = <2000>;
+                        type = "active";
+                };
+        };
+
+        cooling-maps {
+                map2 {
+                        trip = <&cpu_warm>;
+                        cooling-device = <&fan THERMAL_NO_LIMIT 1>;
+                };
+
+                map3 {
+                        trip = <&cpu_hot>;
+                        cooling-device = <&fan 2 THERMAL_NO_LIMIT>;
+                };
+        };
 };
 
 &emmc_phy {
