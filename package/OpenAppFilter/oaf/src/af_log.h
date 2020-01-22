#ifndef __AF_DEBUG_H__
#define __AF_DEBUG_H__
extern int af_log_lvl;
extern int af_test_mode;
#define LOG(level, fmt, ...) do { \
    if ((level) <= af_log_lvl) { \
        printk(fmt, ##__VA_ARGS__); \
    } \
} while (0)

#define LLOG(level, fmt, ...) do { \
	if ((level) <= af_log_lvl) { \
		pr_info_ratelimited(fmt, ##__VA_ARGS__); \
	} \
} while (0)


#define AF_ERROR(...)			LOG(0, ##__VA_ARGS__)
#define AF_WARN(...)         	LOG(1, ##__VA_ARGS__)
#define AF_INFO(...)         	LOG(2, ##__VA_ARGS__)
#define AF_DEBUG(...)       	LOG(3, ##__VA_ARGS__)

#define AF_LMT_ERROR(...)      	LLOG(0, ##__VA_ARGS__)
#define AF_LMT_WARN(...)       	LLOG(1, ##__VA_ARGS__)
#define AF_LMT_INFO(...)       	LLOG(2, ##__VA_ARGS__)
#define AF_LMT_DEBUG(...)     	LLOG(3, ##__VA_ARGS__)


#define TEST_MODE() (af_test_mode)
int af_log_init(void);
int af_log_exit(void);
#endif
