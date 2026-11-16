#include "Sorting.h"
#include <string>
#include <chrono>
using namespace std;
using namespace chrono;
// Compare two intruder records using the selected criterion
static bool isGreater(const Intruder &a, const Intruder &b, SortCriteria criteria)
{
    switch (criteria)
    {
        case BY_ID:
            return a.id > b.id;

        case BY_THREAT_LEVEL:
            return a.threatlevel < b.threatlevel; // Highest threat first

        case BY_NAME:
            return a.name > b.name;

        case BY_LOCATION:
            return a.location > b.location;
    }

    return false;
}

// Swap two intruder records
static void swapIntruders(Intruder &a, Intruder &b)
{
    Intruder temp = a;
    a = b;
    b = temp;
}

// Bubble Sort
void bubbleSortIntruders(Intruder arr[], int size,
                         SortCriteria criteria, SortStats &stats)
{
    stats = SortStats{};

    if (arr == nullptr || size <= 1)
        return;

    auto start = high_resolution_clock::now();

    for (int i = 0; i < size - 1; i++)
    {
        stats.passes++;
        bool swapped = false;

        for (int j = 0; j < size - i - 1; j++)
        {
            stats.comparisons++;

            if (isGreater(arr[j], arr[j + 1], criteria))
            {
                swapIntruders(arr[j], arr[j + 1]);
                swapped = true;
            }
        }

        // Stop if no swaps occurred in this pass
        if (!swapped)
            break;
    }

    auto end = high_resolution_clock::now();
    stats.executionTime =
        duration<double, milli>(end - start).count();
}

// Insertion Sort
void insertionSortIntruders(Intruder arr[], int size,
                            SortCriteria criteria, SortStats &stats)
{
    stats = SortStats{};

    if (arr == nullptr || size <= 1)
        return;

    auto start = high_resolution_clock::now();

    for (int i = 1; i < size; i++)
    {
        stats.passes++;

        Intruder key = arr[i];
        int j = i - 1;

        while (j >= 0)
        {
            stats.comparisons++;

            if (isGreater(arr[j], key, criteria))
            {
                arr[j + 1] = arr[j];
                j--;
            }
            else
                break;
        }

        arr[j + 1] = key;
    }

    auto end = high_resolution_clock::now();
    stats.executionTime =
        duration<double, milli>(end - start).count();
}

// Merge helper function
static void merge(Intruder arr[], int left, int mid, int right,
                  SortCriteria criteria, SortStats &stats)
{
    int n1 = mid - left + 1;
    int n2 = right - mid;

    // Dynamically allocate temporary arrays
    Intruder *L = new Intruder[n1];
    Intruder *R = new Intruder[n2];

    for (int i = 0; i < n1; i++)
        L[i] = arr[left + i];

    for (int j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;

    while (i < n1 && j < n2)
    {
        stats.comparisons++;

        if (!isGreater(L[i], R[j], criteria))
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }

    while (i < n1)
        arr[k++] = L[i++];

    while (j < n2)
        arr[k++] = R[j++];

    // Free allocated memory
    delete[] L;
    delete[] R;
}

// Merge Sort helper
static void mergeSortHelper(Intruder arr[], int left, int right,
                            SortCriteria criteria, SortStats &stats)
{
    if (left < right)
    {
        stats.passes++;

        int mid = left + (right - left) / 2;

        mergeSortHelper(arr, left, mid, criteria, stats);
        mergeSortHelper(arr, mid + 1, right, criteria, stats);

        merge(arr, left, mid, right, criteria, stats);
    }
}

// Merge Sort
void mergeSortIntruders(Intruder arr[], int size,
                        int left, int right,
                        SortCriteria criteria, SortStats &stats)
{
    stats = SortStats{};

    if (arr == nullptr || size <= 0 ||
        left < 0 || right < left || right >= size)
        return;

    auto start = high_resolution_clock::now();

    mergeSortHelper(arr, left, right, criteria, stats);

    auto end = high_resolution_clock::now();
    stats.executionTime =
        duration<double, milli>(end - start).count();
}

// Quick Sort partition
static int partitionIntruders(Intruder arr[], int low, int high,
                              SortCriteria criteria, SortStats &stats)
{
    int mid = low + (high - low) / 2;
    swapIntruders(arr[mid], arr[high]);
    Intruder pivot = arr[high];
    int i = low - 1;

    for (int j = low; j < high; j++)
    {
        stats.comparisons++;

        if (!isGreater(arr[j], pivot, criteria))
        {
            i++;
            swapIntruders(arr[i], arr[j]);
        }
    }

    swapIntruders(arr[i + 1], arr[high]);

    return i + 1;
}

// Quick Sort helper
static void quickSortHelper(Intruder arr[], int low, int high,
                            SortCriteria criteria, SortStats &stats)
{
    if (low < high)
    {
        stats.passes++;

        int pi = partitionIntruders(arr, low, high, criteria, stats);

        quickSortHelper(arr, low, pi - 1, criteria, stats);
        quickSortHelper(arr, pi + 1, high, criteria, stats);
    }
}

// Quick Sort
void quickSortIntruders(Intruder arr[], int low, int high,
                        SortCriteria criteria, SortStats &stats)
{
    stats = SortStats{};

    if (arr == nullptr || low < 0 || high < low)
        return;

    auto start = high_resolution_clock::now();

    quickSortHelper(arr, low, high, criteria, stats);

    auto end = high_resolution_clock::now();
    stats.executionTime =
        duration<double, milli>(end - start).count();
}
