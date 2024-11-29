#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

typedef struct shared_int {
    int i; // Integer
    int p; // Parent
    int c; // Child
} shared_int;

#define SHM_KEY 1235
#define SHM_SIZE sizeof(shared_int)

int main() {
    int shmid;
    shared_int* counter;
    pid_t pid;
    // Create a shared memory segment
    shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }
    // Attach the shared memory segment
    counter = (shared_int*)shmat(shmid, NULL, 0);
    if (counter == (shared_int*)-1) {
        perror("shmat");
        exit(1);
    }
    counter->i = 0;
    counter->p = 0;
    counter->c = 0;
    pid = fork();
    if (pid == 0) { // Child process (turn == 1)
        for (int i = 0; i < 1000000; i++) {
            counter->c = 1; // The child wants to enter its critical section
            while (counter->p == 1) { // If the parent also wants to, then wait
                counter->c = 0;
            }
            counter->i++; // The critical section
            counter->c = 0; // The child is done
        }
    } else { // Parent process (turn == 0)
        for (int i = 0; i < 1000000; i++) {
            counter->p = 1; // The parent wants to enter its critical section
            while (counter->c == 1) { // If the child also wants to, then wait
                counter->p = 0;
            }
            counter->i++; // The critical section
            counter->p = 0; // The parent is done
        }
        wait(NULL);
    }
    printf("Final counter value: %d\n", counter->i);
    // Detach and remove the shared memory segment
    shmdt(counter);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}

/*
Things can go wrong in at least two ways:

1. The most common is having an output like:
```
Final counter value: 1841781
Final counter value: 2000000
```

This means that at some certain time, (roughly 150k times) both processes were executing their
critical sections **simultaneously**.

How?

Assume for a moment that the each line of code is atomic:

| Parent Process             | Child Process             | counter->p | counter->c |
------------------------------------------------------------------------------------
| counter->p = 1;            |                           | 1          | 0          |
| counter->c == 1; (false)   |                           | 1          | 0          |
|                            | counter->c = 1;           | 1          | 1          |
|                            | counter->p == 1; (true)   | 1          | 1          |
| counter->i++               |                           | 1          | 1          |
|                            | counter->p == 1; (true)   | 1          | 1          |
| counter->p = 0;            |                           | 0          | 1          | 

The trick here is that `counter->p == 1;` and `counter->c == 1;` are not atomic from 
the perspective of the CPU. So, one process might interrupt them and perform one or 
more operations in the middle, leading to both processes being found into their
critical sections.

2. The least common is having the process hang.

This might occur in this scenario:

| Parent Process             | Child Process             | counter->p | counter->c |
------------------------------------------------------------------------------------
| counter->p = 1;            |                           | 1          | 0          |
|                            | counter->c = 1;           | 1          | 1          |
| counter->c == 1; (true)    |                           | 1          | 1          |
|                            | counter->p == 1; (true)   | 1          | 1          |
| counter->c == 1; (true)    |                           | 1          | 1          |
|                            | counter->p == 1; (true)   | 1          | 1          |
| counter->c == 1; (true)    |                           | 1          | 1          |
|                            | counter->p == 1; (true)   | 1          | 1          |
| counter->c == 1; (true)    |                           | 1          | 1          |
|                            | counter->p == 1; (true)   | 1          | 1          |
| counter->c == 1; (true)    |                           | 1          | 1          |
|                            | counter->p == 1; (true)   | 1          | 1          |
...

This is mutual exclusion from accessing the same resource!

*/