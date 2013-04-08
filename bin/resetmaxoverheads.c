#include "litmus.h"
#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include<inttypes.h>

int main(int argc, char **argv) {


	
	if (!reset_max_overheads()) {
		printf("The maximum system overhead are reset\n");
	}
	return 0;
}
