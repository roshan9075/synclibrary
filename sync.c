#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<linux/futex.h>
#include<sys/syscall.h>
#include<sys/time.h>
#include<stdlib.h>
#include"synchronization.h"





/*
static inline int xchg(volatile int *addr, int newval) {
   int result;
   
   // The + in "+m" denotes a read-modify-write operand.
   asm volatile("lock; xchgl %0, %1" :
                "+m" (*addr), "=a" (result) :
                "1" (newval) :
                "cc");
   return result;
 }
*/




void spin_lock(struct spinlock *lk) {
	//while(xchg(&lk->locked, 1) != 0)
	while(__sync_val_compare_and_swap (&lk->locked, 0 ,1))
		;
	sched_yield();
}

void initlock(struct spinlock *lk) {
	lk->locked = 0;
}


void spin_unlock(struct spinlock *lk) {
	//xchg(&lk->locked, 0);
	__sync_val_compare_and_swap (&lk->locked, 1 ,0);
}


void init (struct semaphore *s, int initval) {
	s->val = initval;
	initlock(&(s->lk));

}

static long sys_futex(void *addr1, int op, int val1, struct timespec *timeout, void *addr2, int val3) {
	
	return syscall(SYS_futex, addr1, op, val1, timeout, addr2, val3);
}






long x1;
void block (struct semaphore *s) {
	spin_unlock(&(s->lk));
	x1 = sys_futex(&s->val1, FUTEX_WAIT, s->val1, NULL, 0, 0);	
			
	//x1 = sys_futex(&s->val1, FUTEX_CMP_REQUEUE, s->val1, NULL, 0, 0);
		
	sched_yield();
	
	//return sys_futex(&s->val1, FUTEX_WAIT, s->val1, NULL, 0, 0);
}

void wait(struct semaphore *s) {
	spin_lock(&(s->lk));
	//(s->val)--;
	//spin_unlock(&(s->lk));
	while(s->val <= 0) {
		//s->count++;
		//spin_unlock(&(s->lk));		
		//x1 = sys_futex(&s->val1, FUTEX_CMP_REQUEUE, s->val1, NULL, 0, 0);
		//x1 = sys_futex(&s->val1, FUTEX_WAIT, s->val1, NULL, 0, 0);
		block(s);
		//sched_yield();
		spin_lock(&(s->lk));
	}
	//spin_unlock(&(s->lk));
	//spin_lock(&(s->lk));
	(s->val)--;
	spin_unlock(&(s->lk));
	
}
long x2;

void sem_signal(struct semaphore *s) {
	spin_lock(&(s->lk));
	//(s->val)++;
	//spin_unlock(&(s->lk));
	/*if(s->count) {
		printf("%d", s->count);		
		//long x = sys_futex(&s->val1, FUTEX_WAKE, 1, NULL, 0, 0);		
		while((sys_futex(&s->val1, FUTEX_WAKE_OP, 1, NULL, 0, 0)) < 1) {
			sched_yield();
		}
		s->count--;
	}	
	else {*/	
		(s->val)++;
	//}
	/*if(s->val <= 0){
		while((x = sys_futex(&s->val1, FUTEX_WAKE, 1, NULL, 0, 0)) < 1)	
			sched_yield();	
	}*/
	x2= (sys_futex(&s->val1, FUTEX_WAKE, 1, NULL, 0, 0));
	sched_yield();
	//x = dequeue(s->sl) and enqueue(readyq, x);
	spin_unlock(&(s->lk));
	
}



void cond_init (condition *c) {
	initlock(&(c->listlock));

}

//struct condition cond;

long x;
void cond_wait (condition *c, spinlock s) {
	spin_lock(&c->listlock);
	x = sys_futex(&c->futex, FUTEX_CMP_REQUEUE, c->futex, NULL, 0, 0);	
	//x = sys_futex(&c->futex, FUTEX_WAIT, c->futex, NULL, 0, 0);  //add self to the linked list;
	spin_unlock(&c->listlock);
	spin_lock(&s);
	x = sys_futex(&c->futex, FUTEX_WAIT, c->futex, NULL, 0, 0);
	sched_yield();			//swtch();
	spin_unlock(&s);
	//return;
}


long x3;
long x4;
void do_signal (condition *c) {
	spin_lock(&c->listlock);
	x3 = sys_futex(&c->futex, FUTEX_WAKE, 1, NULL, 0, 0);	
	//remove one thread from linked list if it is nonempty;
	spin_unlock(&c->listlock);
	//if(x1 > 0) {
		//spin_lock(&s);
		x4 = sys_futex(&c->futex, FUTEX_CMP_REQUEUE, c->futex, NULL, 0, 0);
		sched_yield();			//swtch();
		//spin_unlock(&s);
		//sched_yield();
	//}
	//return;
}



void do_broadcast (condition *c) {
	spin_lock(&c->listlock);
	while (sys_futex(&c->futex, FUTEX_WAKE, 1, NULL, 0, 0) < 1)	{
		sched_yield();		
		//remove a thread from linked list;
		//make it runnable;
	}
	spin_unlock(&c->listlock);
}









//struct rwlock rw;

void init_lock(struct rwlock *r) {
	r->n_readers = 0;
	r->n_pending_readers = 0;
	r->n_pending_writers = 0;
	initlock(&(r->lk));
	initlock(&(r->sl));
}
long x9;
void lockshared (struct rwlock *r)	{
	spin_lock(&(r->lk));
	/*block until a reader is active*/
	while(r->n_readers < 0) {
		r->n_pending_readers++;		
		spin_unlock(&r->lk);
		spin_lock(&r->sl);
		x9 = sys_futex(&r->readers, FUTEX_WAIT, r->readers, NULL, 0, 0);
		//move the current process to readersq;
		
		spin_unlock(&(r->sl));
		sched_yield();
		spin_lock(&r->lk);
	}
	r->n_readers++;
	//r->n_pending_readers--;
	//assert (n_readers >= 0)
	spin_unlock(&(r->lk));
	
}
long x8;
void lockexcl(struct rwlock *r) {
	spin_lock(&(r->lk));
	while(r->n_readers != 0) {
		r->n_pending_writers++;		
		spin_unlock(&r->lk);
		spin_lock(&r->sl);		
		x8 = sys_futex(&r->writers, FUTEX_WAIT, r->writers, NULL, 0, 0);		
		//sys_futex(&r->writers, FUTEX_WAIT, r->writers, NULL, 0, 0);
		//move the current process to writersq;
		
		spin_unlock(&(r->sl));
		sched_yield();
		spin_lock(&(r->lk));
	}
	//assert n_readers = 0;
	(r->n_readers) = -1;
	spin_unlock(&(r->lk));
	
}

long x7;
void unlockshared(struct rwlock *r) {
	spin_lock(&(r->lk));
	r->n_readers--;
	if(r->n_readers == 0) {
		if(r->n_pending_writers > 0) {
			r->n_pending_writers--;			
			spin_unlock(&r->lk);
			//spin_lock(&r->sl);
			x7 = sys_futex(&r->writers, FUTEX_WAKE, 1, NULL, 0, 0);
			//spin_unlock(&r->sl);
			sched_yield();//move one writer to readyq;
			spin_lock(&r->lk);
			
		}
	}
	spin_unlock(&(r->lk));
	
}
long x6;
void  unlockexcl(struct rwlock *r) {
	spin_lock(&(r->lk));
	if(r->n_pending_readers) {
		while(r->n_pending_readers) {
			r->n_pending_readers--;			
			spin_unlock(&r->lk);
			//spin_lock(&r->sl);
			x6 = sys_futex(&r->readers, FUTEX_REQUEUE, 1, NULL, 0, 0);
			//spin_unlock(&r->sl);
			sched_yield();//move one reader to readyq;
			spin_lock(&r->lk);
			
		}
	}
	r->n_readers = 0;
	spin_unlock(&(r->lk));
	
}










