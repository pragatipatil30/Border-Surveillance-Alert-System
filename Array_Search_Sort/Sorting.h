#ifndef SORTING_H
#define SORTING_H

#include "../common/Intruder.h"

enum SortCriteria
{
    BY_ID,
    BY_THREAT_LEVEL,
    BY_NAME,
    BY_LOCATION
};

struct SortStats
{
    long long comparisons = 0;
    long long passes = 0;
    double executionTime = 0.0;
};

// Bubble Sort
void bubbleSortIntruders(Intruder arr[], int size,
                         SortCriteria criteria, SortStats &stats);

// Insertion Sort
void insertionSortIntruders(Intruder arr[], int size,
                            SortCriteria criteria, SortStats &stats);

// Merge Sort
void mergeSortIntruders(Intruder arr[], int left, int right,
                        SortCriteria criteria, SortStats &stats);

// Quick Sort
void quickSortIntruders(Intruder arr[], int low, int high,
                        SortCriteria criteria, SortStats &stats);

#endif