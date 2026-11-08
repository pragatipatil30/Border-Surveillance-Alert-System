#include <iostream>
#include "Sorting.h"

using namespace std;

int main()
{
    // Sample intruder records
    Intruder records[5] = {
        {105, "Rahul", "North", 2},
        {102, "Amit", "East", 4},
        {104, "Sneha", "West", 1},
        {101, "Priya", "South", 5},
        {103, "Karan", "North", 3}
    };

    int size = 5;

    // Statistics
    SortStats stats;

    // Display before sorting
    cout << "Before Sorting:\n";

    for (int i = 0; i < size; i++)
    {
        cout << records[i].id << " "
             << records[i].name << " "
             << records[i].threatlevel << endl;
    }

    // Apply Bubble Sort by ID
    insertionSortIntruders(records, size, BY_ID, stats);

    // Display after sorting
    cout << "\nAfter Bubble Sort:\n";

    for (int i = 0; i < size; i++)
    {
        cout << records[i].id << " "
             << records[i].name << " "
             << records[i].threatlevel << endl;
    }

    // Display statistics
    cout << "\nComparisons: " << stats.comparisons;
    cout << "\nPasses: " << stats.passes;
    cout << "\nExecution Time: " << stats.executionTime << " ms\n";

    return 0;
}