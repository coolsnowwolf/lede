#ifdef VENDOR_FEATURE7_SUPPORT
/*  Copyright 2014, ARRIS Enterprises, Inc., All rights reserved                       */

#ifndef _ARRIS_MOD_API_H
#define _ARRIS_MOD_API_H


#include <linux/kernel.h>

#define ARRISMOD_DEFINE(type, fn, args...)    \
{						\
type(*fn)(args) = NULL;                      \
EXPORT_SYMBOL(fn)				\
}						\

/* FIXME: silence undefined reference */
#ifndef ARRIS_MODULE_PRESENT

static inline void arris_event_send_hook_fn(int cpu,
						int logtype,
						int dummy,
						char *msg,
						int msglen)
{
	printk(KERN_ERR"\nARRIS_EVENT::: %s\n", msg);
}
extern void (*f)(int, int, int, char*, int);
#define arris_event_send_hook	f

#else

#define arris_event_send_hook	NULL

#define ARRISMOD_EXTERN(type, fn, args...) {	\
	extern type (*fn)(args);		\
}
#endif /* ARRIS_MODULE_PRESENT */

#define ARRISMOD_CALL(fn, args...) {		\
	if (fn) {				\
		fn(args);	                \
	}                                       \
}

#define ARRISMOD_CALL_SET(val, fn, args...) {		\
	if (fn) {					\
		val = fn(args)				\
	}						\
}

#define ARRISMOD_CALL_BREAK(fn, args...) {		\
	if (fn) {					\
		if (fn(args) < 0)			\
		break;                                  \
	}						\
}

#define ARRISMOD_CALL_EXIT(fn, args...) {		\
	if (fn) {					\
		if (fn(args) < 0)			\
		return;                                 \
	}						\
}

#define ARRISMOD_RETURN(fn, ret_trig, ret_val, args...) {	\
	if (fn) {						\
		if (fn(args) == ret_trig)			\
			return ret_val;				\
	}							\
}

#endif /* _ARRIS_MOD_API_H */
#endif /* VENDOR_FEATURE7_SUPPORT*/

