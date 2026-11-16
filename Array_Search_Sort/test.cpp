#include <iostream>

#include "Searching.h"
#include "Sorting.h"
#include "SparseMatrix.h"
#include "SinglyLinkedList.h"
#include "DoublyLinkedList.h"
#include "CircularLinkedList.h"
#include "HashChaining.h"
#include "HashLinearProbing.h"
#include "Josephus.h"

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

    // ==========================================
    // 1. SEARCHING TESTS
    // ==========================================

    SearchStats linearStats;
    SearchStats binaryStats;

    cout << "\n===== SEARCHING TESTS =====\n";

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

    // Additional search tests
    cout << "\n===== ADDITIONAL SEARCH TESTS =====\n";

    SearchStats extraStats;

    int missingResult =
        linearSearch(records, size, 999, extraStats);

    cout << "\nMissing ID result: " << missingResult;
    cout << "\nExpected: -1\n";

    int invalidResult =
        binarySearch(records, 0, 104, extraStats);

    cout << "\nInvalid size result: " << invalidResult;
    cout << "\nExpected: -1\n";

    int nullResult =
        linearSearch(nullptr, size, 104, extraStats);

    cout << "\nNull array result: " << nullResult;
    cout << "\nExpected: -1\n";


    // ==========================================
    // 2. SORTING TESTS
    // ==========================================

    SortStats sortStats;

    cout << "\n===== SORTING TESTS =====\n";

    // Bubble Sort
    Intruder bubbleRecords[5];

    for (int i = 0; i < size; i++)
        bubbleRecords[i] = records[i];

    bubbleSortIntruders(
        bubbleRecords, size, BY_THREAT_LEVEL, sortStats);

    cout << "\nBubble Sort - Threat Level\n";
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

    cout << "\nInsertion Sort - Name\n";
    displayRecords(insertionRecords, size);

    cout << "Comparisons: " << sortStats.comparisons;
    cout << "\nPasses: " << sortStats.passes;
    cout << "\nTime: " << sortStats.executionTime << " ms\n";


    // Merge Sort
    Intruder mergeRecords[5];

    for (int i = 0; i < size; i++)
        mergeRecords[i] = records[i];

    mergeSortIntruders(
        mergeRecords, size, 0, size - 1, BY_LOCATION, sortStats);

    cout << "\nMerge Sort - Location\n";
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

    cout << "\nQuick Sort - ID\n";
    displayRecords(quickRecords, size);

    cout << "Comparisons: " << sortStats.comparisons;
    cout << "\nPasses: " << sortStats.passes;
    cout << "\nTime: " << sortStats.executionTime << " ms\n";


    // Duplicate-value sorting test
    cout << "\n===== DUPLICATE SORTING TEST =====\n";

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

    displayRecords(duplicateRecords, 5);


    // ==========================================
    // 3. SPARSE MATRIX TESTS
    // ==========================================

    cout << "\n===== SPARSE MATRIX TESTS =====\n";

    SparseMatrix matrix(4, 4);

    matrix.insert(0, 1, 10);
    matrix.insert(1, 2, 20);
    matrix.insert(2, 3, 30);

    cout << "\nMatrix dimensions: "
         << matrix.getRows() << " x " << matrix.getCols();

    cout << "\nStored elements: " << matrix.size();

    cout << "\nValue at (1, 2): " << matrix.getValue(1, 2);

    cout << "\nValue at (0, 0): " << matrix.getValue(0, 0);

    cout << "\nIs empty? "
         << (matrix.isEmpty() ? "Yes" : "No") << endl;


    // Update and zero-value test
    cout << "\n===== MATRIX UPDATE TEST =====\n";

    matrix.insert(1, 2, 50);
    matrix.insert(3, 3, 0);

    cout << "\nUpdated value: " << matrix.getValue(1, 2);
    cout << "\nZero position value: " << matrix.getValue(3, 3);
    cout << "\nStored elements: " << matrix.size() << endl;


    // Deletion test
    cout << "\n===== MATRIX DELETION TEST =====\n";

    bool matrixRemoved = matrix.remove(1, 2);

    cout << "\nExisting element removed: "
         << (matrixRemoved ? "Yes" : "No");

    cout << "\nValue after deletion: "
         << matrix.getValue(1, 2);

    bool matrixRemovedAgain = matrix.remove(1, 2);

    cout << "\nRepeated deletion: "
         << (matrixRemovedAgain ? "Yes" : "No") << endl;


    // Invalid coordinate test
    cout << "\n===== MATRIX INVALID INPUT TEST =====\n";

    matrix.insert(5, 5, 100);

    cout << "\nSize after invalid insertion: " << matrix.size();

    cout << "\nInvalid position value: " << matrix.getValue(5, 5);

    bool invalidMatrixRemoval = matrix.remove(5, 5);

    cout << "\nInvalid removal: "
         << (invalidMatrixRemoval ? "Yes" : "No") << endl;


    // Export state test
    cout << "\n===== MATRIX EXPORT TEST =====\n";

    SparseEntry *exportedEntries = matrix.exportState();

    if (exportedEntries != nullptr)
    {
        for (int i = 0; i < matrix.size(); i++)
        {
            cout << "Row: " << exportedEntries[i].row
                 << ", Column: " << exportedEntries[i].col
                 << ", Value: " << exportedEntries[i].value << endl;
        }
    }
    else
    {
        cout << "No entries to export.\n";
    }

    delete[] exportedEntries;


    // Empty and invalid dimensions
    cout << "\n===== MATRIX EMPTY TEST =====\n";

    SparseMatrix emptyMatrix(3, 3);

    cout << "\nInitially empty: "
         << (emptyMatrix.isEmpty() ? "Yes" : "No");

    SparseMatrix invalidMatrix(-2, 4);

    cout << "\nInvalid matrix rows: " << invalidMatrix.getRows();
    cout << "\nInvalid matrix columns: " << invalidMatrix.getCols() << endl;


    // Capacity growth
    cout << "\n===== MATRIX CAPACITY GROWTH TEST =====\n";

    SparseMatrix largeMatrix(5, 5);

    for (int i = 0; i < 12; i++)
    {
        largeMatrix.insert(i / 5, i % 5, (i + 1) * 10);
    }

    cout << "\nStored elements: " << largeMatrix.size();
    cout << "\nValue at (2, 1): "
         << largeMatrix.getValue(2, 1) << endl;


    // ==========================================
    // 4. SINGLY LINKED LIST TESTS
    // ==========================================

    cout << "\n===== SINGLY LINKED LIST TESTS =====\n";

    SinglyLinkedList singlyList;

    singlyList.insert(records[0]);
    singlyList.insert(records[1]);
    singlyList.insert(records[2]);

    cout << "\nInserted records: " << singlyList.size();

    const Intruder *singlyFound = singlyList.search(102);

    cout << "\nSearch ID 102: "
         << (singlyFound != nullptr ? singlyFound->name : "Not found");

    const Intruder *singlyMissing = singlyList.search(999);

    cout << "\nSearch ID 999: "
         << (singlyMissing != nullptr ? "Found" : "Not found");

    singlyList.insert(records[0]);

    cout << "\nSize after duplicate insertion: "
         << singlyList.size();

    bool singlyRemoved = singlyList.remove(102);

    cout << "\nRemove ID 102: "
         << (singlyRemoved ? "Yes" : "No");

    cout << "\nFinal size: " << singlyList.size();

    Intruder *singlyExported = singlyList.exportState();

    cout << "\nExported records:\n";

    if (singlyExported != nullptr)
    {
        for (int i = 0; i < singlyList.size(); i++)
        {
            cout << singlyExported[i].id << " | "
                 << singlyExported[i].name << endl;
        }
    }

    delete[] singlyExported;


    // ==========================================
    // 5. DOUBLY LINKED LIST TESTS
    // ==========================================

    cout << "\n===== DOUBLY LINKED LIST TESTS =====\n";

    DoublyLinkedList doublyList;

    doublyList.insert(records[0]);
    doublyList.insert(records[1]);
    doublyList.insert(records[2]);

    cout << "\nInserted records: " << doublyList.size();

    const Intruder *doublyFound = doublyList.search(103);

    cout << "\nSearch ID 103: "
         << (doublyFound != nullptr ? doublyFound->name : "Not found");

    const Intruder *doublyMissing = doublyList.search(999);

    cout << "\nSearch ID 999: "
         << (doublyMissing != nullptr ? "Found" : "Not found");

    doublyList.insert(records[0]);

    cout << "\nSize after duplicate insertion: "
         << doublyList.size();

    bool doublyRemoved = doublyList.remove(102);

    cout << "\nRemove ID 102: "
         << (doublyRemoved ? "Yes" : "No");

    cout << "\nFinal size: " << doublyList.size();

    Intruder *doublyExported = doublyList.exportState();

    cout << "\nExported records:\n";

    if (doublyExported != nullptr)
    {
        for (int i = 0; i < doublyList.size(); i++)
        {
            cout << doublyExported[i].id << " | "
                 << doublyExported[i].name << endl;
        }
    }

    delete[] doublyExported;


    // ==========================================
    // 6. CIRCULAR LINKED LIST TESTS
    // ==========================================

    cout << "\n===== CIRCULAR LINKED LIST TESTS =====\n";

    CircularLinkedList circularList;

    circularList.insert(records[0]);
    circularList.insert(records[1]);
    circularList.insert(records[2]);

    cout << "\nInserted records: " << circularList.size();

    const Intruder *circularFound = circularList.search(101);

    cout << "\nSearch ID 101: "
         << (circularFound != nullptr ? circularFound->name : "Not found");

    const Intruder *circularMissing = circularList.search(999);

    cout << "\nSearch ID 999: "
         << (circularMissing != nullptr ? "Found" : "Not found");

    circularList.insert(records[0]);

    cout << "\nSize after duplicate insertion: "
         << circularList.size();

    bool circularRemoved = circularList.remove(102);

    cout << "\nRemove ID 102: "
         << (circularRemoved ? "Yes" : "No");

    cout << "\nFinal size: " << circularList.size();

    Intruder *circularExported = circularList.exportState();

    cout << "\nExported records:\n";

    if (circularExported != nullptr)
    {
        for (int i = 0; i < circularList.size(); i++)
        {
            cout << circularExported[i].id << " | "
                 << circularExported[i].name << endl;
        }
    }

    delete[] circularExported;


    // ==========================================
    // 7. HASH TABLE - CHAINING TESTS
    // ==========================================

    cout << "\n===== HASH CHAINING TESTS =====\n";

    HashChaining chainingTable(10);

    bool chainInsert1 = chainingTable.insert(records[0]);
    bool chainInsert2 = chainingTable.insert(records[1]);
    bool chainInsert3 = chainingTable.insert(records[2]);

    cout << "\nInsert results: "
         << chainInsert1 << ", "
         << chainInsert2 << ", "
         << chainInsert3;

    cout << "\nTable size: " << chainingTable.size();

    cout << "\nTable capacity: " << chainingTable.getCapacity();

    const Intruder *chainFound = chainingTable.search(102);

    cout << "\nSearch ID 102: "
         << (chainFound != nullptr ? chainFound->name : "Not found");

    const Intruder *chainMissing = chainingTable.search(999);

    cout << "\nSearch ID 999: "
         << (chainMissing != nullptr ? "Found" : "Not found");

    bool duplicateChainInsert = chainingTable.insert(records[0]);

    cout << "\nDuplicate insertion accepted: "
         << (duplicateChainInsert ? "Yes" : "No");

    bool chainRemoved = chainingTable.remove(102);

    cout << "\nRemove ID 102: "
         << (chainRemoved ? "Yes" : "No");

    cout << "\nFinal table size: " << chainingTable.size();


    // Export chaining table
    HashNode **chainExported = chainingTable.exportState();

    cout << "\nExported chaining table:\n";

    if (chainExported != nullptr)
    {
        for (int i = 0; i < chainingTable.getCapacity(); i++)
        {
            HashNode *current = chainExported[i];

            while (current != nullptr)
            {
                cout << "Bucket " << i << ": "
                     << current->data.id << " | "
                     << current->data.name << endl;

                HashNode *nextNode = current->next;
                delete current;
                current = nextNode;
            }
        }

        delete[] chainExported;
    }


    // ==========================================
    // 8. HASH TABLE - LINEAR PROBING TESTS
    // ==========================================

    cout << "\n===== HASH LINEAR PROBING TESTS =====\n";

    HashLinearProbing probingTable(10);

    bool probeInsert1 = probingTable.insert(records[0]);
    bool probeInsert2 = probingTable.insert(records[1]);
    bool probeInsert3 = probingTable.insert(records[2]);

    cout << "\nInsert results: "
         << probeInsert1 << ", "
         << probeInsert2 << ", "
         << probeInsert3;

    cout << "\nTable size: " << probingTable.size();

    cout << "\nTable capacity: " << probingTable.getCapacity();

    const Intruder *probeFound = probingTable.search(103);

    cout << "\nSearch ID 103: "
         << (probeFound != nullptr ? probeFound->name : "Not found");

    const Intruder *probeMissing = probingTable.search(999);

    cout << "\nSearch ID 999: "
         << (probeMissing != nullptr ? "Found" : "Not found");

    bool duplicateProbeInsert = probingTable.insert(records[0]);

    cout << "\nDuplicate insertion accepted: "
         << (duplicateProbeInsert ? "Yes" : "No");

    bool probeRemoved = probingTable.remove(102);

    cout << "\nRemove ID 102: "
         << (probeRemoved ? "Yes" : "No");

    cout << "\nFinal table size: " << probingTable.size();


    // Export probing table
    HashEntry *probeExported = probingTable.exportState();

    cout << "\nExported probing table:\n";

    if (probeExported != nullptr)
    {
        for (int i = 0; i < probingTable.getCapacity(); i++)
        {
            if (probeExported[i].occupied)
            {
                cout << "Index " << i << ": "
                     << probeExported[i].data.id << " | "
                     << probeExported[i].data.name << endl;
            }
        }

        delete[] probeExported;
    }


    // ==========================================
    // 9. JOSEPHUS PROBLEM TESTS
    // ==========================================

    cout << "\n===== JOSEPHUS PROBLEM TESTS =====\n";

    int totalUnits = 7;
    int eliminationStep = 3;

    JosephusResult josephusResult =
        solveJosephus(totalUnits, eliminationStep);

    cout << "\nTotal units: " << totalUnits;
    cout << "\nElimination step: " << eliminationStep;

    cout << "\nElimination order: ";

    for (int i = 0; i < josephusResult.eliminationCount; i++)
    {
        cout << josephusResult.eliminationOrder[i] << " ";
    }

    cout << "\nSurvivor position: "
         << josephusResult.survivorPosition;

    cout << "\nTotal eliminations: "
         << josephusResult.eliminationCount << endl;

    delete[] josephusResult.eliminationOrder;


    // Invalid Josephus input
    cout << "\n===== JOSEPHUS INVALID INPUT TEST =====\n";

    JosephusResult invalidJosephus = solveJosephus(0, 3);

    cout << "\nInvalid input survivor: "
         << invalidJosephus.survivorPosition;

    cout << "\nInvalid input elimination count: "
         << invalidJosephus.eliminationCount << endl;

    delete[] invalidJosephus.eliminationOrder;


    cout << "\n===== ALL TEST SECTIONS COMPLETED =====\n";

    return 0;
}
