#include "Josephus.h"

JosephusResult solveJosephus(int n, int k)
{
    JosephusResult result;
    result.survivorPosition = -1;
    result.eliminationCount = 0;
    result.eliminationOrder = nullptr;

    if (n <= 0 || k <= 0)
        return result;

    // Create a circular linked list of positions.
    struct Node
    {
        int position;
        Node *next;
    };

    Node *head = new Node{1, nullptr};
    Node *tail = head;

    for (int i = 2; i <= n; i++)
    {
        Node *newNode = new Node{i, nullptr};
        tail->next = newNode;
        tail = newNode;
    }

    tail->next = head;

    result.eliminationOrder = new int[n - 1];

    Node *current = tail;
    int remaining = n;

    while (remaining > 1)
    {
        // Move k - 1 steps to reach the person to eliminate.
        for (int step = 1; step < k; step++)
            current = current->next;

        Node *removed = current->next;

        result.eliminationOrder[result.eliminationCount] =
            removed->position;

        result.eliminationCount++;

        current->next = removed->next;

        if (removed == head)
            head = removed->next;

        delete removed;
        remaining--;
    }

    result.survivorPosition = current->position;

    delete current;

    return result;
}