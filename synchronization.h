





#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<linux/futex.h>
#include<sys/syscall.h>
#include<sys/time.h>
#include<stdlib.h>


typedef struct spinlock {
	int locked;
}spinlock;

//static inline int xchg(volatile int *addr, int newval);

void initlock(struct spinlock *lk);

void spin_lock(struct spinlock *lk);

void spin_unlock(struct spinlock *lk);



typedef struct semaphore {
	unsigned int val;
	struct spinlock lk;
	//unsigned int count;  //count to keep track of waiting processes
	int val1;//futex varriable
	//struct semaphore *s; 
}semaphore;


void init(struct semaphore *s, int initval);

static long sys_futex(void *addr1, int op, int val1, struct timespec *timeout, void *addr2, int val3);

void block (struct semaphore *s);

void wait(struct semaphore *s);

void sem_signal(struct semaphore *s);



typedef struct condition {
	int futex;	
	//proc *next;
	//proc *prev;
	struct spinlock listlock;
}condition;


/*static long sys_futex(void *addr1, int op, int val1, struct timespec *timeout, void *addr2, int val3);*/

void cond_init (condition *c);

void cond_wait (condition *c, spinlock s);

void do_signal (condition *c);

void do_broadcast (condition *c);




typedef struct rwlock {
	int n_readers;
	int n_pending_readers;
	int n_pending_writers;
	int readers;	
	int writers;
	struct spinlock lk;
	struct spinlock sl;
}rwlock;


/*static long sys_futex(void *addr1, int op, int val1, struct timespec *timeout, void *addr2, int val3);*/

void init_lock(struct rwlock *r);

void lockshared (struct rwlock *r);

void lockexcl(struct rwlock *r);

void unlockshared(struct rwlock *r);

void  unlockexcl(struct rwlock *r);

