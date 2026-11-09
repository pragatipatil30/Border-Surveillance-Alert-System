#include <iostream>
#include "Searching.h"
#include "Sorting.h"
#include "SparseMatrix.h"

using namespace std;

// Display intruder records
void displayRecords(Intruder arr[], int size)
{
    for (int i = 0; i < size; i++)
    {
        cout << arr[i].id << " | "
             << arr[i].name << " | "
             << arr[i].location << " | "
             << arr[i].threatlevel << endl;
    }
}

int main()
{
    Intruder records[5] = {
        {101, "Priya", "South", 5},
        {102, "Amit", "East", 4},
        {103, "Karan", "North", 3},
        {104, "Sneha", "West", 1},
        {105, "Rahul", "North", 2}
    };

    int size = 5;
    int searchID = 104;

    // SEARCHING TESTS
    SearchStats linearStats;
    SearchStats binaryStats;

    cout << "===== SEARCHING TESTS =====\n";

    int linearResult =
        linearSearch(records, size, searchID, linearStats);

    int binaryResult =
        binarySearch(records, size, searchID, binaryStats);

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

    // SORTING TESTS
    SortStats sortStats;

    cout << "\n===== SORTING TESTS =====\n";

    // Bubble Sort
    Intruder bubbleRecords[5];
    for (int i = 0; i < size; i++)
        bubbleRecords[i] = records[i];

    bubbleSortIntruders(
        bubbleRecords, size, BY_THREAT_LEVEL, sortStats);

    cout << "\nBubble Sort - Threat Level";
    displayRecords(bubbleRecords, size);
    cout << "Comparisons: " << sortStats.comparisons;
    cout << "\nPasses: " << sortStats.passes;
    cout << "\nTime: " << sortStats.executionTime << " ms\n";

    // Insertion Sort
    Intruder insertionRecords[5];
    for (int i = 0; i < size; i++)
        insertionRecords[i] = records[i];

    insertionSortIntruders(
        insertionRecords, size, BY_NAME, sortStats);

    cout << "\nInsertion Sort - Name";
    displayRecords(insertionRecords, size);
    cout << "Comparisons: " << sortStats.comparisons;
    cout << "\nPasses: " << sortStats.passes;
    cout << "\nTime: " << sortStats.executionTime << " ms\n";

    // Merge Sort
    Intruder mergeRecords[5];
    for (int i = 0; i < size; i++)
        mergeRecords[i] = records[i];

    mergeSortIntruders(
        mergeRecords, 0, size - 1, BY_LOCATION, sortStats);

    cout << "\nMerge Sort - Location";
    displayRecords(mergeRecords, size);
    cout << "Comparisons: " << sortStats.comparisons;
    cout << "\nPasses: " << sortStats.passes;
    cout << "\nTime: " << sortStats.executionTime << " ms\n";

    // Quick Sort
    Intruder quickRecords[5];
    for (int i = 0; i < size; i++)
        quickRecords[i] = records[i];
    quickSortIntruders(
        quickRecords, 0, size - 1, BY_ID, sortStats);
    cout << "\nQuick Sort - ID";
    displayRecords(quickRecords, size);
    cout << "Comparisons: " << sortStats.comparisons;
    cout << "\nPasses: " << sortStats.passes;
    cout << "\nTime: " << sortStats.executionTime << " ms\n";
    // ADDITIONAL SEARCH TESTS

    cout << "\n===== ADDITIONAL SEARCH TESTS =====\n";

    SearchStats extraStats;

    // ID not found
    int missingResult =
        linearSearch(records, size, 999, extraStats);

    cout << "\nSearching for missing ID (999)";
    cout << "\nResult: " << missingResult;
    cout << "\nExpected: -1\n";

    // Invalid size
    int invalidResult =
        binarySearch(records, 0, 104, extraStats);

    cout << "\nSearching with invalid size (0)";
    cout << "\nResult: " << invalidResult;
    cout << "\nExpected: -1\n";

    // Null array
    int nullResult =
        linearSearch(nullptr, size, 104, extraStats);

    cout << "\nSearching with null array";
    cout << "\nResult: " << nullResult;
    cout << "\nExpected: -1\n";
        // DUPLICATE-VALUE SORTING TEST

    cout << "\n===== DUPLICATE-VALUE SORTING TEST =====\n";

    Intruder duplicateRecords[5] = {
        {201, "Amit", "East", 3},
        {202, "Priya", "West", 5},
        {203, "Rahul", "North", 3},
        {204, "Sneha", "South", 5},
        {205, "Karan", "East", 1}
    };

    SortStats duplicateStats;

    bubbleSortIntruders(
        duplicateRecords, 5, BY_THREAT_LEVEL, duplicateStats);

    cout << "\nBubble Sort with duplicate threat levels:";
    displayRecords(duplicateRecords, 5);
        // SPARSE MATRIX TESTS

    cout << "\n===== SPARSE MATRIX TESTS =====\n";

    SparseMatrix matrix(4, 4);

    // Insert non-zero elements
    matrix.insert(0, 1, 10);
    matrix.insert(1, 2, 20);
    matrix.insert(2, 3, 30);

    cout << "\nMatrix dimensions: "
         << matrix.getRows() << " x " << matrix.getCols();

    cout << "\nStored non-zero elements: "
         << matrix.size();

    cout << "\nValue at (1, 2): "
         << matrix.getValue(1, 2);

    cout << "\nValue at (0, 0): "
         << matrix.getValue(0, 0);

    cout << "\nIs matrix empty? "
         << (matrix.isEmpty() ? "Yes" : "No") << endl;
        // UPDATE AND ZERO-VALUE TEST

    cout << "\n===== UPDATE AND ZERO-VALUE TEST =====\n";

    // Update an existing position
    matrix.insert(1, 2, 50);

    // Attempt to insert zero
    matrix.insert(3, 3, 0);

    cout << "\nUpdated value at (1, 2): "
         << matrix.getValue(1, 2);

    cout << "\nValue at (3, 3): "
         << matrix.getValue(3, 3);

    cout << "\nStored elements: "
         << matrix.size() << endl;
        // DELETION TEST

    cout << "\n===== DELETION TEST =====\n";

    // Remove an existing element
    bool removed = matrix.remove(1, 2);

    cout << "\nExisting element removed: "
         << (removed ? "Yes" : "No");

    cout << "\nValue after deletion: "
         << matrix.getValue(1, 2);

    cout << "\nStored elements after deletion: "
         << matrix.size();

    // Try removing the same element again
    bool removedAgain = matrix.remove(1, 2);

    cout << "\nRemove same element again: "
         << (removedAgain ? "Yes" : "No") << endl;
        // INVALID COORDINATE TEST

    cout << "\n===== INVALID COORDINATE TEST =====\n";

    // Attempt insertion outside matrix dimensions
    matrix.insert(5, 5, 100);

    cout << "\nSize after invalid insertion: "
         << matrix.size();

    // Attempt to retrieve an invalid position
    cout << "\nValue at invalid position (5, 5): "
         << matrix.getValue(5, 5);

    // Attempt to remove an invalid position
    bool invalidRemoval = matrix.remove(5, 5);

    cout << "\nInvalid removal successful: "
         << (invalidRemoval ? "Yes" : "No") << endl;
        // EXPORT STATE TEST

    cout << "\n===== EXPORT STATE TEST =====\n";

    SparseEntry *exportedEntries = matrix.exportState();

    cout << "\nExported entries: " << matrix.size() << endl;

    for (int i = 0; i < matrix.size(); i++)
    {
        cout << "Row: " << exportedEntries[i].row
             << ", Column: " << exportedEntries[i].col
             << ", Value: " << exportedEntries[i].value << endl;
    }

    // Release dynamically allocated memory
    delete[] exportedEntries;
        // EMPTY MATRIX AND INVALID DIMENSIONS TEST

    cout << "\n===== EMPTY MATRIX TEST =====\n";

    SparseMatrix emptyMatrix(3, 3);

    cout << "\nInitially empty: "
         << (emptyMatrix.isEmpty() ? "Yes" : "No");

    cout << "\nInitial stored elements: "
         << emptyMatrix.size();

    // Test zero dimensions
    SparseMatrix invalidMatrix(-2, 4);

    cout << "\nInvalid matrix rows: "
         << invalidMatrix.getRows();

    cout << "\nInvalid matrix columns: "
         << invalidMatrix.getCols() << endl;
    // SPARSE MATRIX CAPACITY GROWTH TEST

    cout << "\n===== CAPACITY GROWTH TEST =====\n";
    SparseMatrix largeMatrix(5, 5);

    for (int i = 0; i < 12; i++)
    {
      largeMatrix.insert(i / 5, i % 5, (i + 1) * 10);
    }
    cout << "Stored elements after 12 insertions: "
         << largeMatrix.size() << endl;
    cout << "Value at (2, 1): "
         << largeMatrix.getValue(2, 1) << endl;
    return 0;
}
