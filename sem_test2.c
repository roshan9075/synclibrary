#include <stdio.h> 
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h> 
#include<linux/futex.h>
#include<sys/syscall.h>
//#include"spinlock.h" 
#include"synchronization.h"







struct semaphore s;


long c = 0, c1 = 0, c2 = 0, run = 1;
void *thread1(void *arg) {
	while(run == 1) {
		wait(&s);
		c++;
		sem_signal(&s);
		c1++;
	}
return NULL;
}
void *thread2(void *arg) {
	while(run == 1) {
		wait(&s);
		c++;
		sem_signal(&s);
		c2++;
	}
return NULL;
}
int main() {
	pthread_t th1, th2;
	//struct spinlock lk;
	//struct semaphore s;	
	init(&s, 1); 
	pthread_create(&th1, NULL, thread1, NULL);
	pthread_create(&th2, NULL, thread2, NULL);
	//fprintf(stdout, "Ending main\n");
	sleep(2);
	run = 0;
	pthread_join(th1, NULL);
	pthread_join(th2, NULL);
	fprintf(stdout, "c = %ld c1+c2 = %ld c1 = %ld c2 = %ld \n", c, c1+c2, c1, c2);
	fflush(stdout);
}

