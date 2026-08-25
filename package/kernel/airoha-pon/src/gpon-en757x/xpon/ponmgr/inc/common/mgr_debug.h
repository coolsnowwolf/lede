#ifndef _MGR_DEBUG_H_
#define _MGR_DEBUG_H_

#include <regex.h>
#include <strings.h>

extern char *get_current_timestamp(void) ;

#define LOG_MESSAGE(F, B...)			printf("%s: " F, get_current_timestamp(), ##B) ; 

#ifdef CONFIG_DEBUG
	#define DEBUG_MESSAGE(F, B...)		printf("%s %s [%d]: " F, get_current_timestamp(), rindex(__FILE__, '/') + 1, __LINE__, ##B)
	#define DEBUG_PERROR(F, B...)		printf("%s %s [%d]: " F, get_current_timestamp(), rindex(__FILE__, '/') + 1, __LINE__, ##B)
	#define DEBUG_LOG(F, B...)			printf("%s %s [%d]: " F, get_current_timestamp(), rindex(__FILE__, '/') + 1, __LINE__, ##B)
#else
	#define DEBUG_MESSAGE(F, B...)		
	#define DEBUG_PERROR(F, B...)		
	#define DEBUG_LOG(F, B...)			LOG_MESSAGE(F, B...)
#endif

#endif /* _MGR_DEBUG_H_ */
