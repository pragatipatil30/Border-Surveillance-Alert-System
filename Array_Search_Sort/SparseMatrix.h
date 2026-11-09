#ifndef SPARSE_MATRIX_H
#define SPARSE_MATRIX_H

struct SparseEntry
{
    int row;
    int col;
    int value;
};

class SparseMatrix
{
private:
    SparseEntry *entries;
    int capacity;
    int count;
    int rows;
    int cols;

public:
    SparseMatrix(int totalRows, int totalCols);
    ~SparseMatrix();
    SparseMatrix(const SparseMatrix &) = delete;
    SparseMatrix &operator=(const SparseMatrix &) = delete;
    void insert(int row, int col, int value);
    int getValue(int row, int col) const;
    bool remove(int row, int col);

    int getRows() const;
    int getCols() const;
    int size() const;
    bool isEmpty() const;
    // Caller must release the returned array using delete[].
    SparseEntry *exportState() const;
};

#endif