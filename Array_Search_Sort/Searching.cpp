#include "Searching.h"
#include <chrono>

using namespace std;
using namespace chrono;

// Linear Search
// Checks each intruder record one by one.
int linearSearch(Intruder arr[], int size, int key,
                 SearchStats &stats)
{
    stats = SearchStats{};

    auto start = high_resolution_clock::now();

    for (int i = 0; i < size; i++)
    {
        stats.comparisons++;

        if (arr[i].id == key)
        {
            auto end = high_resolution_clock::now();

            stats.executionTime =
                duration<double, milli>(end - start).count();

            return i;
        }
    }

    auto end = high_resolution_clock::now();

    stats.executionTime =
        duration<double, milli>(end - start).count();

    return -1;
}

// Binary Search
// Records must be sorted by ID in ascending order.
int binarySearch(Intruder arr[], int size, int key,
                 SearchStats &stats)
{
    stats = SearchStats{};

    auto start = high_resolution_clock::now();

    int low = 0;
    int high = size - 1;

    while (low <= high)
    {
        int mid = low + (high - low) / 2;

        stats.comparisons++;

        if (arr[mid].id == key)
        {
            auto end = high_resolution_clock::now();

            stats.executionTime =
                duration<double, milli>(end - start).count();

            return mid;
        }
        else if (key < arr[mid].id)
        {
            high = mid - 1;
        }
        else
        {
            low = mid + 1;
        }
    }

    auto end = high_resolution_clock::now();

    stats.executionTime =
        duration<double, milli>(end - start).count();

    return -1;
}