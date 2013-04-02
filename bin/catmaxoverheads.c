#include "litmus.h"
#include <stdio.h>


int main(int argc, char **argv) {

	struct max_overheads_t max_overheads;
	get_max_overheads(&max_overheads);

	printf("The maximum system overhead are %llu\n %llu\n %llu\n %llu\n %llu\n %llu\n %llu\n ",
	       max_overheads.cxs,
	       max_overheads.sched,
	       max_overheads.sched2,
	       max_overheads.release,
	       max_overheads.send_resched,
	       max_overheads.release_latency,
	       max_overheads.tick);


	return 0;
}
