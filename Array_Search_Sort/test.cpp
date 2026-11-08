#include <iostream>
#include "Searching.h"

using namespace std;

int main()
{
    // Intruder records sorted by ID
    Intruder records[5] = {
        {101, "Priya", "South", 5},
        {102, "Amit", "East", 4},
        {103, "Karan", "North", 3},
        {104, "Sneha", "West", 1},
        {105, "Rahul", "North", 2}
    };

    int size = 5;
    int searchID = 104;

    SearchStats linearStats;
    SearchStats binaryStats;

    // Linear Search
    int linearResult =
        linearSearch(records, size, searchID, linearStats);

    // Binary Search
    int binaryResult =
        binarySearch(records, size, searchID, binaryStats);

    cout << "Searching for Intruder ID: " << searchID << endl;

    cout << "\nLinear Search";
    cout << "\nIndex: " << linearResult;
    cout << "\nComparisons: " << linearStats.comparisons;
    cout << "\nExecution Time: "
         << linearStats.executionTime << " ms\n";

    cout << "\nBinary Search";
    cout << "\nIndex: " << binaryResult;
    cout << "\nComparisons: " << binaryStats.comparisons;
    cout << "\nExecution Time: "
         << binaryStats.executionTime << " ms\n";

    return 0;
}