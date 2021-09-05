--- a/pth.h.in
+++ b/pth.h.in
@@ -43,7 +43,7 @@
 #include <time.h>          /* for struct timespec */
 #include <sys/time.h>      /* for struct timeval  */
 #include <sys/socket.h>    /* for sockaddr        */
-#include <sys/signal.h>    /* for sigset_t        */
+#include <signal.h>        /* for sigset_t        */
 @EXTRA_INCLUDE_SYS_SELECT_H@
 
     /* fallbacks for essential typedefs */
--- a/pthread.h.in
+++ b/pthread.h.in
@@ -111,7 +111,7 @@ typedef int __vendor_sched_param;
 #include <sys/types.h>     /* for ssize_t         */
 #include <sys/time.h>      /* for struct timeval  */
 #include <sys/socket.h>    /* for sockaddr        */
-#include <sys/signal.h>    /* for sigset_t        */
+#include <signal.h>        /* for sigset_t        */
 #include <time.h>          /* for struct timespec */
 #include <unistd.h>        /* for off_t           */
 @EXTRA_INCLUDE_SYS_SELECT_H@
