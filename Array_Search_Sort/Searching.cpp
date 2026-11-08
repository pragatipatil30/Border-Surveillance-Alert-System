#include "Searching.h"
// Linear Search: searches an intruder record using ID
int linearSearch(Intruder arr[], int size, int key)
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i].id == key)
        {
            return i;
        }
    }
    return -1;
}
// Binary Search: searches an integer array
// The array must be sorted in ascending order
int binarySearch(int arr[], int size, int key)
{
    int low = 0;
    int high = size - 1;
    while (low <= high)
    {
        int mid = (low + high) / 2;

        if (arr[mid] == key)
        {
            return mid;
        }
        else if (key < arr[mid])
        {
            high = mid - 1;
        }
        else
        {
            low = mid + 1;
        }
    }
    return -1;
}