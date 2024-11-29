#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

typedef struct shared_int {
    int i; // Shared counter
    int turn; // Shared turn flag
} shared_int;

#define SHM_KEY 1234
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
    char* buf;
    counter->i = 0;
    counter->turn = 0;
    pid = fork();
    if (pid == 0) { // Child process (turn == 1)
        for (int i = 0; i < 10; i++) {
            while (counter->turn != 1) { // This is roughly "Wait for your turn!"
                continue;
            }
            counter->i++; // This is the critical section for the child
            counter->turn = 0; // This is "signaling" that it is the parent's turn
        }
    } else { // Parent process (turn == 0)
        for (int i = 0; i < 10; i++) {
            // scanf("%s", buf); // Think of this as a call to a third-party process that takes loooong
            while (counter->turn != 0) { // This is roughly "Wait for your turn!"
                continue;
            }
            counter->i++; // This is the critical section for the parent
            counter->turn = 1; // This is "signaling" that it is the child's turn
        }
        wait(NULL);
    }
    printf("Final counter value: %d\n", counter->i);
    // Detach and remove the shared memory segment
    shmdt(counter);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}