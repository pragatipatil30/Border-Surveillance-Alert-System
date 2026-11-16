#include "SparseMatrix.h"

// Constructor
SparseMatrix::SparseMatrix(int totalRows, int totalCols)
{
    rows = (totalRows > 0) ? totalRows : 0;
    cols = (totalCols > 0) ? totalCols : 0;

    capacity = (rows > 0 && cols > 0) ? 10 : 0;
    count = 0;

    entries = (capacity > 0) ? new SparseEntry[capacity] : nullptr;
}
// Destructor
SparseMatrix::~SparseMatrix()
{
    delete[] entries;
}

// Insert or update a matrix element
void SparseMatrix::insert(int row, int col, int value)
{
    if (row < 0 || row >= rows || col < 0 || col >= cols)
        return;

    // Check whether the position already exists
    for (int i = 0; i < count; i++)
    {
        if (entries[i].row == row && entries[i].col == col)
        {
            if (value == 0)
            {
                remove(row, col);
                return;
            }

            entries[i].value = value;
            return;
        }
    }

    // Zero values are not stored
    if (value == 0)
        return;

    // Increase capacity when required
    if (count == capacity)
    {
        capacity *= 2;

        SparseEntry *newEntries = new SparseEntry[capacity];

        for (int i = 0; i < count; i++)
            newEntries[i] = entries[i];

        delete[] entries;
        entries = newEntries;
    }

    entries[count].row = row;
    entries[count].col = col;
    entries[count].value = value;

    count++;
}

// Get the value at a given position
int SparseMatrix::getValue(int row, int col) const
{
    if (row < 0 || row >= rows || col < 0 || col >= cols)
        return 0;

    for (int i = 0; i < count; i++)
    {
        if (entries[i].row == row && entries[i].col == col)
            return entries[i].value;
    }

    return 0;
}

// Remove an element
bool SparseMatrix::remove(int row, int col)
{
    if (row < 0 || row >= rows || col < 0 || col >= cols)
        return false;
    for (int i = 0; i < count; i++)
    {
        if (entries[i].row == row && entries[i].col == col)
        {
            for (int j = i; j < count - 1; j++)
                entries[j] = entries[j + 1];

            count--;
            return true;
        }
    }
    return false;
}
// Get number of rows
int SparseMatrix::getRows() const
{
    return rows;
}

// Get number of columns
int SparseMatrix::getCols() const
{
    return cols;
}

// Get number of stored non-zero entries
int SparseMatrix::size() const
{
    return count;
}

// Check whether matrix is empty
bool SparseMatrix::isEmpty() const
{
    return count == 0;
}
// Export stored entries for Qt visualization
SparseEntry *SparseMatrix::exportState() const
{
    if (count == 0)
        return nullptr;

    SparseEntry *copy = new SparseEntry[count];

    for (int i = 0; i < count; i++)
        copy[i] = entries[i];

    return copy;
}