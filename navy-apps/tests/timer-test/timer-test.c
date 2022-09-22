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

int main() {
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
