// source/malloc_001.c
#include <stdio.h>
#include <stdlib.h>

int* foo(int size) { // One way to initialise an array using user input.
    int* arr = (int*) malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        printf("Enter arr[%d]: ", i);
        scanf("%d", arr + i);
        /*
        cout << "Enter arr[" << i << "]: ";
        cin >> *(arr + i);
        */
    }
    return arr;
}

void printArr(int* arr, int size) {
    printf("[ ");
    for (int i = 0; i < size; i++) {
        printf("%d ", *(arr + i));
    }
    printf("]\n");
}

int main() {
    int* arr = foo(5);
    printArr(arr, 5);
    free(arr); // <-- This is sometimes hard to remember!
    return 0;
}