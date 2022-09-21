#include "types.h"
#include "user.h"
#include "stat.h"

int
main(int argc, char *argv[])
{	
	int pid;
	int mask = atoi(argv[1]);
	
	// 예외처리 인수부족
	if(argc < 3){
		printf(1, "Least 3 Instruments!\nUsage : ssu_trace [int] [mask] [traced command]\n");
		exit();
	}
	
	pid = fork();
	if(pid < 0){
		printf(1, "init: fork failed\n");
		exit();
	}
	if(pid == 0){
		if(trace(mask) == 0){
			exec(argv[2], argv+2);
		} else {
			printf(1, "tracing failed\nUsage : ssu_trace [int] [mask] [traced command]\n");
		}
		exit();
	}
	wait();
	exit();
}
