#include "litmus.h"
#include <stdio.h>
#include <stdlib.h>
#define __STDC_FORMAT_MACROS
#include<inttypes.h>
#include <unistd.h>

#define OPTSTR "x:c:i:s:"

static void usage(char *error) {
	fprintf(stderr, "Error: %s\n", error);
	fprintf(stderr,
		"Usage:\n"
		"	setextracxs [-x 0,1] [-c cpu] [-i init_itr] [-s step_itr\n"
		"\n"
		"0 to disable extra loops in cxs path; 1 to enable.\n"
		"cpu: target cpu.\n"
		"init_itr: initial number of  introduced iterations\n"
		"step_itr: extra number of iterations introduced\n");
	exit(EXIT_FAILURE);
}


int main(int argc, char **argv) {

	struct extra_cxs_t _extra_cxs;
	int opt;

	if (argc != 9)
		usage("Arguments missing.");
	    
	while ((opt = getopt(argc, argv, OPTSTR)) != -1) {
		switch (opt) {
		case 'x':
			_extra_cxs.do_extra_cxs = atoi(optarg);
			break;
		case 'c':
			_extra_cxs.extra_cxs_cpu = atoi(optarg);
			break;
		case 'i':
			_extra_cxs.init_extra_cxs = atoi(optarg);
			break;
		case 's':
			_extra_cxs.step_extra_cxs = atoi(optarg);
			break;
		case '?':
		default:
			usage("Bad argument.");
		break;
		}
	}

	if (!set_extra_cxs(&_extra_cxs)) {
		printf("Extra context_switch(): \n"
		       "do_extra_cxs = %d\n"
		       "extra_cxs_cpu = %d\n"
		       "init_extra_cxs = %u\n"
		       "step_extra_cxs = %u\n",
		       _extra_cxs.do_extra_cxs,
		       _extra_cxs.extra_cxs_cpu,
		       _extra_cxs.init_extra_cxs,
		       _extra_cxs.step_extra_cxs);
	} else {
		printf("Failed to set extra_cxs \n");
	}
	return 0;
}
