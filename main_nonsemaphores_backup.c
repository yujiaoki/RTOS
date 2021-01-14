 /* main.c
  */
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>

#define INNERLOOP 1000
#define OUTERLOOP 2000

#define NPERIODICTASKS 3
#define NAPERIODICTASKS 0
#define NTASKS NPERIODICTASKS + NAPERIODICTASKS

long int periods[NTASKS];
struct timespec next_arrival_time[NTASKS];
double WCET[NTASKS];
pthread_attr_t attributes[NTASKS];
pthread_t thread_id[NTASKS];
struct sched_param parameters[NTASKS];
int missed_deadlines[NTASKS];
// the string variable of the result of implementing right now
char now_str[100];
 
// initialization of mutexes and conditions 
pthread_mutex_t mutex_task = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_task = PTHREAD_COND_INITIALIZER;


//code of periodic tasks
void task1_code( );
void task2_code( );
void task3_code( );

//characteristic function of the thread, only for timing and synchronization
//periodic tasks
void *task1( void *);
void *task2( void *);
void *task3( void *);

// initialization of mutexes and conditions (only for aperiodic scheduling)
// loop count of each thread 
int lc = 10;

int send_to_simple_mod(char call_msg[]) 
{
         int fd, result, len;
         char buf[10];
         const char *str;
         if ((fd = open ("/dev/simple", O_RDWR)) == -1) {
                  perror("open failed");
                  return -1;
         }
	 len = strlen(call_msg)+1;
         if ((result = write (fd, call_msg, len)) != len) 
	 {
                  perror("write failed");
                  return -1;
         }
	//  printf("%d bytes written \n", result);
	 close(fd);

}

char* connect_string(char* s1, const char* s2)
{
    int i = 0, j = 0;
    for(; s1[i] != '\0'; i++);
    for(; s2[j] != '\0'; j++)
    {
        s1[i + j] = s2[j];
    }
    s1[i + j] = s2[j];
    return s1;
}

int main() {
  	// set task periods in nanoseconds
	//the first task has period 300 millisecond
	//the second task has period 500 millisecond
	//the third task has period 800 millisecond
	//you can already order them according to their priority; 
	//if not, you will need to sort them
  	periods[0]= 300000000; //in nanoseconds
  	periods[1]= 500000000; //in nanoseconds
  	periods[2]= 800000000; //in nanoseconds

	//this is not strictly necessary, but it is convenient to
	//assign a name to the maximum and the minimum priotity in the
	//system. We call them priomin and priomax.

  	struct sched_param priomax;
  	priomax.sched_priority=sched_get_priority_max(SCHED_FIFO);
  	struct sched_param priomin;
  	priomin.sched_priority=sched_get_priority_min(SCHED_FIFO);

	// set the maximum priority to the current thread (you are required to be
  	// superuser). Check that the main thread is executed with superuser privileges
	// before doing anything else.

  	if (getuid() == 0)
    		pthread_setschedparam(pthread_self(),SCHED_FIFO,&priomax);

  	// execute all tasks in standalone modality in order to measure execution times
  	// (use gettimeofday). Use the computed values to update the worst case execution
  	// time of each task.


  	for (int i = 0; i < NTASKS; i++)
    	{

		// initializa time_1 and time_2 required to read the clock
		struct timespec time_1, time_2;
		clock_gettime(CLOCK_REALTIME, &time_1);

		//we should execute each task more than one for computing the WCET
		//periodic tasks
 	     	if (i==0) task1_code();
      		if (i==1) task2_code();
      		if (i==2) task3_code();

		clock_gettime(CLOCK_REALTIME, &time_2);


		// compute the Worst Case Execution Time (in a real case, we should repeat this many times under
		//different conditions, in order to have reliable values

      		WCET[i] = 1000000000*(time_2.tv_sec - time_1.tv_sec)
			       + (time_2.tv_nsec-time_1.tv_nsec);
      		printf("\nWorst Case Execution Time %d=%f \n", i, WCET[i]);
    	}

    	// compute U
	double U = WCET[0]/periods[0] + WCET[1]/periods[1] + WCET[2]/periods[2];

    	// compute Ulub by considering the fact that we have harmonic relationships between periods
	double Ulub = 1;
    	
	//if there are no harmonic relationships, use the following formula instead
	//double Ulub = NPERIODICTASKS*(pow(2.0,(1.0/NPERIODICTASKS)) -1);
	
	//check the sufficient conditions: if they are not satisfied, exit  
  	if (U > Ulub)
    	{
      		printf("\n U=%lf Ulub=%lf Non schedulable Task Set", U, Ulub);
      		return(-1);
    	}
  	printf("\n U=%lf Ulub=%lf Scheduable Task Set", U, Ulub);
  	fflush(stdout);
  	sleep(5);

  	// set the minimum priority to the current thread: this is now required because 
	//we will assign higher priorities to periodic threads to be soon created
	//pthread_setschedparam

  	if (getuid() == 0)
    		pthread_setschedparam(pthread_self() , SCHED_FIFO,&priomin);

  
  	// set the attributes of each task, including scheduling policy and priority
  	for (int i = 0; i < NPERIODICTASKS; i++)
    	{
		//initializa the attribute structure of task i
      		pthread_attr_init(&(attributes[i]));

		//set the attributes to tell the kernel that the priorities and policies are explicitly chosen,
		//not inherited from the main thread (pthread_attr_setinheritsched) 
      		pthread_attr_setinheritsched(&(attributes[i]), PTHREAD_EXPLICIT_SCHED);
      
		// set the attributes to set the SCHED_FIFO policy (pthread_attr_setschedpolicy)
		pthread_attr_setschedpolicy(&(attributes[i]), SCHED_FIFO);

		//properly set the parameters to assign the priority inversely proportional 
		//to the period
      		parameters[i].sched_priority = priomin.sched_priority+NTASKS - i;

		//set the attributes and the parameters of the current thread (pthread_attr_setschedparam)
      		pthread_attr_setschedparam(&(attributes[i]), &(parameters[i]));
    	}

	//delare the variable to contain the return values of pthread_create	
  	int iret[NTASKS];

	//declare variables to read the current time
	struct timespec time_1;
	clock_gettime(CLOCK_REALTIME, &time_1);

  	// set the next arrival time for each task. This is not the beginning of the first
	// period, but the end of the first period and beginning of the next one. 
  	for (int i = 0; i < NPERIODICTASKS; i++)
    	{
		long int next_arrival_nanoseconds = time_1.tv_nsec + periods[i];
		//then we compute the end of the first period and beginning of the next one
		next_arrival_time[i].tv_nsec= next_arrival_nanoseconds%1000000000;
		next_arrival_time[i].tv_sec= time_1.tv_sec + next_arrival_nanoseconds/1000000000;
       		missed_deadlines[i] = 0;
    	}

	

	// create all threads(pthread_create)
  	iret[0] = pthread_create( &(thread_id[0]), &(attributes[0]), task1, NULL);
  	iret[1] = pthread_create( &(thread_id[1]), &(attributes[1]), task2, NULL);
  	iret[2] = pthread_create( &(thread_id[2]), &(attributes[2]), task3, NULL);

  	// join all threads (pthread_join)
  	pthread_join( thread_id[0], NULL);
  	pthread_join( thread_id[1], NULL);
  	pthread_join( thread_id[2], NULL);

  	// set the next arrival time for each task. This is not the beginning of the first
	// period, but the end of the first period and beginning of the next one. 
  	for (int i = 0; i < NTASKS; i++)
    	{
      		printf ("\nMissed Deadlines Task %d=%d", i, missed_deadlines[i]);
			fflush(stdout);
    	}
  	exit(0);
}

// application specific task_1 code
void task1_code()
{
	//print the id of the current task
  	printf(" [1 "); fflush(stdout);
    // connecting as string
    connect_string (now_str,"[1");
    //send the result of implementing to simple module(simple.c simple.ko)
    send_to_simple_mod(now_str);
	//this double loop with random computation is only required to waste time
	int i,j;
	double uno;
  	for (i = 0; i < OUTERLOOP; i++)
    	{
      		for (j = 0; j < INNERLOOP; j++)
		{
			uno = rand()*rand()%10;
    		}
  	}
     // connecting as string
    connect_string(now_str,"1]");
    //send system call message to simple module(simple.c simple.ko)
    send_to_simple_mod(now_str);

  	//print the id of the current task
  	printf(" 1] "); fflush(stdout);
}

//thread code for task_1 (used only for temporization)
void *task1( void *ptr)
{
	// set thread affinity, that is the processor on which threads shall run
	cpu_set_t cset;
	CPU_ZERO (&cset);
	CPU_SET(0, &cset);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);

   	//execute the task one hundred times... it should be an infinite loop (too dangerous)
  	for (int i=0; i < lc; i++)
    	{
      		// execute application specific code
		task1_code();

		// it would be nice to check if we missed a deadline here... why don't
		// you try by yourself?

		// sleep until the end of the current period (which is also the start of the
		// new one
		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[0], NULL);

		// the thread is ready and can compute the end of the current period for
		// the next iteration
 		
		long int next_arrival_nanoseconds = next_arrival_time[0].tv_nsec + periods[0];
		next_arrival_time[0].tv_nsec= next_arrival_nanoseconds%1000000000;
		next_arrival_time[0].tv_sec= next_arrival_time[0].tv_sec + next_arrival_nanoseconds/1000000000;
    	}
}

void task2_code()
{
	//print the id of the current task
  	printf(" [2 "); fflush(stdout);
    // connecting as string
    connect_string (now_str,"[2");
    //send the result of implementing to simple module(simple.c simple.ko)
    send_to_simple_mod(now_str);
	//this double loop with random computation is only required to waste time
	int i,j;
	double uno;
  	for (i = 0; i < OUTERLOOP; i++)
    	{
      		for (j = 0; j < INNERLOOP; j++)
		{
			uno = rand()*rand()%10;
    		}
  	}
     // connecting as string
    connect_string(now_str,"2]");
    //send system call message to simple module(simple.c simple.ko)
   send_to_simple_mod(now_str);

  	//print the id of the current task
  	printf(" 2] "); fflush(stdout);
}


void *task2( void *ptr )
{
	// set thread affinity, that is the processor on which threads shall run
	cpu_set_t cset;
	CPU_ZERO (&cset);
	CPU_SET(0, &cset);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);

  	for (int i=0; i < lc; i++)
    	{
      		task2_code();

		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[1], NULL);
		long int next_arrival_nanoseconds = next_arrival_time[1].tv_nsec + periods[1];
		next_arrival_time[1].tv_nsec= next_arrival_nanoseconds%1000000000;
		next_arrival_time[1].tv_sec= next_arrival_time[1].tv_sec + next_arrival_nanoseconds/1000000000;
    	}
}

void task3_code()
{
	//print the id of the current task
  	printf(" [3 "); fflush(stdout);
    // connecting as string
    connect_string (now_str,"[3");
    //send the result of implementing to simple module(simple.c simple.ko)
    send_to_simple_mod(now_str);
	//this double loop with random computation is only required to waste time
	double uno;
  	for (int i = 0; i < OUTERLOOP; i++)
    	{
      		for (int j = 0; j < INNERLOOP; j++)
		{
			uno = rand()*rand()%10;
    		}
  	}
     // connecting as string
    connect_string(now_str,"3]");
    //send system call message to simple module(simple.c simple.ko)
    send_to_simple_mod(now_str);

  	//print the id of the current task
  	printf(" 3] "); fflush(stdout);
	
}

void *task3( void *ptr)
{
	// set thread affinity, that is the processor on which threads shall run
	cpu_set_t cset;
	CPU_ZERO (&cset);
	CPU_SET(0, &cset);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);

  	for (int i=0; i < lc; i++)
    	{
      		task3_code();

		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[2], NULL);
		long int next_arrival_nanoseconds = next_arrival_time[2].tv_nsec + periods[2];
		next_arrival_time[2].tv_nsec= next_arrival_nanoseconds%1000000000;
		next_arrival_time[2].tv_sec= next_arrival_time[2].tv_sec + next_arrival_nanoseconds/1000000000;
    }
}



