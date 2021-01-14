# RTOS final assignment

Student id: 5053193
Name : Yuji Aoki

## Requirements

Task is following below,

(1) Design an application with 3 threads, whose periods are 300ms, 500ms, and 800ms.

(2) The threads shall just "waste time", as we did in the exercise with threads.

(3) Design a simple driver with only open, close, write system calls.

(4) During its execution, every tasks 

	(i) opens the special file associated with the driver;

	(ii ) writes to the driver its own identifier plus open square brackets (i.e., [1, [2, or [3)

	(iii) close the special files

	(iv) performs operations (i.e., wasting time)

	(v) performs (i)(ii) and (iii) again in order to write to the driver its own identifier, but with closed square brackets (i.e., 1], 2], or 3]).

(5) The write system call simply writes on the kernel log the string that was received from the thread. A typical output of the system, by reading the kernel log, may be the following [11][2[11]2][3[11]3]  . This clearly shows that some threads can be preempted by other threads (if this does not happen, try to increase the computational time of longer tasks).

(6) Finally, modify the code of all tasks in order to use semaphores. Every thread now protects all its operations (i) to (v) with a semaphore, which basically prevents other tasks from preempting it. Specifically, use semaphores with priority ceiling.  


I explain one by one.

1. >Design an application with 3 threads, whose periods are 300ms, 500ms, and 800ms. 

I set the periods of each threads in main.c following below,
```c 
periods[0]= 300000000; //in nanoseconds
periods[1]= 500000000; //in nanoseconds
periods[2]= 800000000; //in nanoseconds
```

2. >The threads shall just "waste time", as we did in the exercise with threads.

the part of "waste time" is following below,just created double for loop and generate random number
```c 
int i,j;
	double uno;
  	for (i = 0; i < OUTERLOOP; i++)
    	{
      		for (j = 0; j < INNERLOOP; j++)
		{
			uno = rand()*rand()%10;
    		}
  	}
```

3&4. > Design a simple driver with only open, close, write system calls.

The part equivalent to this function is followoing below,

```c 
int send_to_simple_mod(char call_msg[]) 
{
         int fd, result, len;
         char buf[10];
         const char *str;
         if ((fd = open ("/dev/simple", O_RDWR |O_CREAT| O_APPEND,0666)) == -1) {
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
```
5. >  The write system call simply writes on the kernel log the string that was received from the thread. A typical output of the system, by reading the kernel log, may be the following [11][2[11]2][3[11]3]  . This clearly shows that some threads can be preempted by other threads (if this does not happen, try to increase the computational time of longer tasks).

I just check the kernel log the system call writes by `dmesg` (you can get the system call by main_nonsemaphores.c)

6. > Finally, modify the code of all tasks in order to use semaphores. Every thread now protects all its operations (i) to (v) with a semaphore, which basically prevents other tasks from preempting it. Specifically, use semaphores with priority ceiling. 

I consider the version of using semaphores. (you can get the system call by main.c)

## how to build and execute

1. `sudo su`
2. `make`
3. `insmod simple.ko`
4. `sbin/lsmod`
5. `cat /proc/devices`
6. `mknod /dev/simple c <majornumber> 0 `
7. task(6) `g++ -pthread main.c -o main` 
(in case of no use semaphores task(1)~(5)) `g++ -pthread main_nonsemaphoses.c -o nonsem` 
8. task(6) `./main` 
(in case of no use semaphores  task(1)~(5)) `./nonsem` 
9. `dmesg` (in order to get the kernel log the write system call writes)
