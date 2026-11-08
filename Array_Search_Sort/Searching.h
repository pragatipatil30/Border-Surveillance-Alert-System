#ifndef SEARCHING_H
#define SEARCHING_H

#include "../common/Intruder.h"

struct SearchStats
{
    long long comparisons = 0;
    double executionTime = 0.0;
};

// Linear Search
int linearSearch(Intruder arr[], int size, int key,
                 SearchStats &stats);

// Binary Search
// Records must be sorted by ID in ascending order.
int binarySearch(Intruder arr[], int size, int key,
                 SearchStats &stats);

#endif