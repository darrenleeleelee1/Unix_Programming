#include <stdio.h>

void swap(long* a, long* b) {
    long temp = *a;
    *a = *b;
    *b = temp;
}

int partition(long arr[], int low, int high) {
    long pivot = arr[high]; // Choosing the last element as the pivot
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void quicksort_recursive(long arr[], int low, int high) {
    if (low < high) {
        int pivot_index = partition(arr, low, high);

        quicksort_recursive(arr, low, pivot_index - 1);
        quicksort_recursive(arr, pivot_index + 1, high);
    }
}

void quicksort(long *numbers, int n) {
    quicksort_recursive(numbers, 0, n - 1);
}
