#include <stdio.h>

void swap(long* a, long* b) {
    long temp = *a;
    *a = *b;
    *b = temp;
}

long median_of_three(long arr[], int low, int high) {
    int mid = low + (high - low) / 2;

    if (arr[low] > arr[mid])
        swap(&arr[low], &arr[mid]);
    if (arr[low] > arr[high])
        swap(&arr[low], &arr[high]);
    if (arr[mid] > arr[high])
        swap(&arr[mid], &arr[high]);

    return arr[mid];
}

int partition(long arr[], int low, int high) {
    long pivot = median_of_three(arr, low, high);
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

int main() {
    long arr[] = {10, 7, 8, 9, 1, 5};
    int n = sizeof(arr) / sizeof(arr[0]);

    quicksort(arr, n);

    printf("Sorted array: \n");
    for (int i = 0; i < n; i++)
        printf("%ld ", arr[i]);
    printf("\n");

    return 0;
}
