#ifdef VENDOR_FEATURE7_SUPPORT
#ifndef	__ARRIS_WPS_GPIO_H__
#define	__ARRIS_WPS_GPIO_H__

#define GPIO_WPS_SW       50
#define GPIO_WPS_LED      31
#define WPS_LED_HIGH      0
#define WPS_LED_LOW       1

#define WPS_CONNECT_TIME              400 /* 120 sec * ( iterations per sec ) */
#define WPS_LED_CHECK_TIME            3 /* sec  3 * 100ms */
#define WPS_PBC_MAX_TIME              10 /* sec 3s = 10 * 300ms */
#define WIFI_24_RADIO     1
#define WIFI_50_RADIO     9

#define LED_AP_STOP        66        /* used to flag whether AP is stop */

#define print_console(fmt, args...) ({fprintf(stdout, fmt, ##args); })
#define print_err(fmt, args...)     ({fprintf(stderr, fmt, ##args); })


/* ++ Anjan: silence undefined reference */
#ifndef ARRIS_MODULE_PRESENT
static inline int arris_wps_gpio_check_wps_button(void) { return 0; }

static inline void arris_wps_led_stat_set(int radio, int ledSta) {}

static inline void arris_wps_led_handler(int radio) {}

#else /* ARRIS_MODULE_PRESENT */
int arris_wps_gpio_check_wps_button(void);

void arris_wps_led_stat_set(int radio, int ledSta);

void arris_wps_led_handler(int radio);
#endif /* ARRIS_MODULE_PRESENT */


#endif
#endif /* VENDOR_FEATURE7_SUPPORT */
