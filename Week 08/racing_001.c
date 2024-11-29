#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define SHM_KEY 1234
#define SHM_SIZE sizeof(int)

int main() {
    int shmid;
    int *counter;
    pid_t pid;
    // Create a shared memory segment
    shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }
    // Attach the shared memory segment
    counter = (int*)shmat(shmid, NULL, 0);
    if (counter == (int*)-1) {
        perror("shmat");
        exit(1);
    }
    *counter = 0;
    pid = fork();
    int* temp;
    if (pid == 0) { // Child process
        for (int i = 0; i < 10000000; i++) {
            (*counter)++;
        }
    } else { // Parent process
        for (int i = 0; i < 10000000; i++) {
            (*counter)++;
        }
        wait(NULL);
    }
    printf("Final counter value (%d): %d\n", pid, *counter);
    // Detach and remove the shared memory segment
    shmdt(counter);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}

/*
(*counter)++ is dissected as follows:
* *counter --> retrieve the value kept at `counter`
* *counter = *counter + 1; --> Increment the value by 1
    * Load *counter and 1 into memory (two distinct operations)
    * Actually compute `*counter + 1` (another distinct operation)
    * Store the output to *counter (another distinct operation)

1. A bad scenario:

| Parent Process        | Child Process         | *counter |
------------------------------------------------------------
| Retrieve *counter(10) |                       | 10       |
| Retrieve 1            |                       | 10       |
|                       | Retrieve *counter(10) | 10       |
| *counter + 1 (11)     |                       | 10       |
| *counter = 11         |                       | 11       |
|                       | Retrieve 1            | 11       |
|                       | *counter + 1 (11)     | 11       |
|                       | *counter = 11         | 11       | <-- This did not change!

2. A good scenario:

| Parent Process        | Child Process         | *counter |
------------------------------------------------------------
| Retrieve *counter(10) |                       | 10       |
| Retrieve 1            |                       | 10       |
| *counter + 1 (11)     |                       | 10       |
| *counter = 11         |                       | 11       |
|                       | Retrieve *counter(11) | 11       |
|                       | Retrieve 1            | 11       |
|                       | *counter + 1 (12)     | 11       |
|                       | *counter = 12         | 12       | <-- This *did* change!
*/