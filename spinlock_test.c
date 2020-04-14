#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include"synchronization.h"


long c = 0, c1 = 0, c2 = 0, run = 1;
struct spinlock lk;
void *thread1(void *arg) {
	while(run == 1) {
		spin_lock(&lk);
		c++;
		spin_unlock(&lk);
		c1++;
	}
return NULL;
}
void *thread2(void *arg) {
	while(run == 1) {
		spin_lock(&lk);
		c++;
		spin_unlock(&lk);
		c2++;
	}
return NULL;
}
int main() {
	pthread_t th1, th2;
	
	initlock(&lk); 
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
