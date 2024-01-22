#include "lib.h"
#include "types.h"

void dpp(int num){
	int i=0, ret=0;
	sem_t sem[5], accessctrl;
	sem_init(&accessctrl, 4);
	for(int i=0;i<num;i++)sem_init(&sem[i], 1);
	for(i=0;i<num-1;i++){
		if(ret==0)ret = fork();
		else if(ret>0) break;
	}
	int id=get_pid();
	for(int i=0;i<2;i++){
		printf("Philosopher %d: think\n", id);
		sleep(128);
		sem_wait(&accessctrl);
		sem_wait(&sem[id-1]);
		sem_wait(&sem[(id)%num]);
		printf("Philosopher %d: eat\n", id);
		sleep(128);
		sem_post(&accessctrl);
		sem_post(&sem[(id)%num]);
		sem_post(&sem[id-1]);
		
	}
	sem_destroy(&sem[id]);
	if(id!=0)exit();
	sem_destroy(&accessctrl);
	exit();
}
void pcp(int num){
	int i=0, ret=0;
	sem_t empty, full, mutex;
	sem_init(&empty, 5);
	sem_init(&full, 0);
	sem_init(&mutex, 1);
	for(i=0;i<num;i++){
		if(ret==0)ret = fork();
		else if(ret>0) break;
	}
	int id = get_pid();
	if(id==1){
		for(int i=0;i<8;i++){
			sem_wait(&full);
			sem_wait(&mutex);
			printf("Consumer : consume\n");
			sleep(128);
			sem_post(&mutex);
			sem_post(&empty);
		}
	}
	else{
		for(int i=0;i<2;i++){
			sem_wait(&empty);
			sem_wait(&mutex);
			printf("Producer %d: produce\n", id-1);
			sleep(128);
			sem_post(&mutex);
			sem_post(&full);
		}
	}
	exit();
}

int uEntry(void) {
	//dpp(5);
	pcp(4);
	// For lab4.1
	// Test 'scanf' 
	int dec = 0;
	int hex = 0;
	char str[6];
	char cha = 0;
	int ret = 0;
	while(1){
		printf("Input:\" Test %%c Test %%6s %%d %%x\"\n");
		ret = scanf(" Test %c Test %6s %d %x", &cha, str, &dec, &hex);
		printf("Ret: %d; %c, %s, %d, %x.\n", ret, cha, str, dec, hex);
		if (ret == 4)
			break;
	}
	
	// For lab4.2
	// Test 'Semaphore'

	int i = 4;

	sem_t sem;
	printf("Father Process: Semaphore Initializing.\n");
	ret = sem_init(&sem, 2);
	if (ret == -1) {
		printf("Father Process: Semaphore Initializing Failed.\n");
		exit();
	}

	ret = fork();
	if (ret == 0) {
		while( i != 0) {
			i --;
			printf("Child Process: Semaphore Waiting.\n");
			sem_wait(&sem);
			printf("Child Process: In Critical Area.\n");
		}
		printf("Child Process: Semaphore Destroying.\n");
		sem_destroy(&sem);
		exit();
	}
	else if (ret != -1) {
		while( i != 0) {
			i --;
			printf("Father Process: Sleeping.\n");
			sleep(128);
			printf("Father Process: Semaphore Posting.\n");
			sem_post(&sem);
		}
		printf("Father Process: Semaphore Destroying.\n");
		sem_destroy(&sem);
		exit();
	}

	// For lab4.3
	// TODO: You need to design and test the philosopher problem.
	// Producer-Consumer problem and Reader& Writer Problem are optional.
	// Note that you can create your own functions.
	// Requirements are demonstrated in the guide.
	
	return 0;
}
