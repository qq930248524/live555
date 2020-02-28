#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "videoMonitor.h"

VideoMonitor VM;

int main()
{
	VM.init();

	int fun;
	while(1){
		scanf("%d", &fun);

		switch(fun){
			case 0:
				if(VM.startMonitor() == 0){
					printf("start monitor!\n");
				}
				break;
			case 1:
				if(VM.stopMonitor() == 0){
					printf("stop monitor!\n");
				}
				break;
			default:
				break;
		}
	}

	return 0;
}

