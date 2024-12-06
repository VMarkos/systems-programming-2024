// source/malloc_002.c
// Use valgrind for memory leaks: valgrind.org
#include <stdio.h>
#include <stdlib.h>

void foo(int* arr, int size) { // One way to initialise an array using user input.
    for (int i = 0; i < size; i++) {
        scanf("%d", arr + i);
    }
}

int main() {
    int size = 5;
    int* arr = (int*) malloc(size * sizeof(int)); // <-- We allocate and free in the same routine.
    foo(arr, size);
    free(arr); // <-- This is usually easier to remember!
    return 0;
}