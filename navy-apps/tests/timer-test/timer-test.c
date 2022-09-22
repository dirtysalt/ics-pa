#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

int ok(struct timeval* before, struct timeval *now) {
   time_t sec_diff = now->tv_sec - before->tv_sec;
   time_t usec_diff = now->tv_usec - before->tv_usec;
   if (sec_diff > 0) return 1;
   if (sec_diff == 0 && usec_diff > 500000) return 1;
   return 0;
}

int xmain() {
	struct timeval last;
	for(;;) {
		struct timeval now;
		gettimeofday(&now, NULL);
		if (ok(&last, &now)) {
			printf("500ms passed\n");
			last = now;
		}
	}
	return 0;
}


#include <NDL.h>
int main() {
	NDL_Init(0);
	uint32_t last = NDL_GetTicks();
	for(;;) {
		uint32_t now = NDL_GetTicks();
		if ((now - last) > 500) {
			printf("500ms passed\n");
			last = now;
		}
	}
	return 0;
		
}
