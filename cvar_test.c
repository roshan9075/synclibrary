#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include<linux/futex.h>
#include<sys/syscall.h>
#include"synchronization.h"



const size_t NUMTHREADS = 5;

/* a global count of the number of threads finished working. It will
   be protected by mutex and changes to it will be signalled to the
   main thread via cond */

int done = 0;

struct spinlock lk;
struct condition cond;
struct spinlock lock;




void* ThreadEntry( void* id ) {
  const int myid = (long)id; // force the pointer to be a 64bit integer
  initlock(&lock);
  //const int workloops = 5;
  const int workloops = 2;
  for( int i=0; i<workloops; i++ )
    {
      printf( "[thread %d] working (%d/%d)\n", myid, i, workloops );
      sleep(1); // simulate doing some costly work
    }
  
  // we're going to manipulate done and use the cond, so we need the mutex
  spin_lock( &lock);

  // increase the count of threads that have finished their work.
  done++;
  printf( "[thread %d] done is now %d. Signalling cond.\n", myid, done );

  				
   do_signal( &cond );// wait up the main thread (if it is sleeping) to test the value of done  
   
  spin_unlock( &lock );

  return NULL;
}

 //initlock(&lock);
int main( int argc, char** argv )
{
  initlock(&lk);
  //initlock(&l);
  initlock(&lock);
  puts( "[thread main] starting" );

  pthread_t threads[NUMTHREADS];

  for( int t=0; t<NUMTHREADS; t++ )
    pthread_create( &threads[t], NULL, ThreadEntry, (void*)(long)t );

  // we're going to test "done" so we need the mutex for safety
  spin_lock( &lock);

  // are the other threads still busy?
  while( done < NUMTHREADS )
    {
      printf( "[thread main] done is %d which is < %d so waiting on cond\n", 
	      done, (int)NUMTHREADS );
      
      /* block this thread until another thread signals cond. While
	 blocked, the mutex is released, then re-aquired before this
	 thread is woken up and the call returns. */ 
      cond_wait( & cond,  lk ); 
      
      puts( "[thread main] wake - cond was signalled." ); 
      
      /* we go around the loop with the lock held */
    }
  //spin_unlock( &lock );
  printf( "[thread main] done == %d so everyone is done\n", (int)NUMTHREADS );
  
  spin_unlock( &lock );
  
  return 0;
}
