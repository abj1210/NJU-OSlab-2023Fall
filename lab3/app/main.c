#include "lib.h"
#include "types.h"

int data = 0;
int uEntry(void) {
	
	int ret = fork();
	int i = 8;
	printf("ret: %d\n", ret);
	if (ret == 0) {
		data = 2;
		while( i != 0) {
			i --;
			printf("Child Process: Pong %d, %d;\n", data, i);
			sleep(128);
		}
		printf("Child Process over, bye-bye~\n");
		exit();
	}
	else if (ret != -1) {
		data = 1;
		while( i != 0) {
			i --;
			printf("Father Process: Ping %d, %d;\n", data, i);
			sleep(128);
		}
		printf("Father Process over, bye-bye~\n");
		exit();
	}
	printf("!!!Can't reach here!!!\n");
	return 0;
}
