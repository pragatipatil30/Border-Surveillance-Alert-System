#ifndef JOSEPHUS_H
#define JOSEPHUS_H

struct JosephusResult
{
    int survivorPosition;
    int eliminationCount;
    int *eliminationOrder;
};

// Positions are numbered from 1 to n.
// Caller must release eliminationOrder using delete[].
JosephusResult solveJosephus(int n, int k);

#endif