#ifndef CIRCULAR_LINKED_LIST_H
#define CIRCULAR_LINKED_LIST_H

#include "../common/Intruder.h"

struct CircularNode
{
    Intruder data;
    CircularNode *next;
};

class CircularLinkedList
{
private:
    CircularNode *tail;
    int count;

public:
    CircularLinkedList();
    ~CircularLinkedList();

    void insert(Intruder record);
    bool remove(int id);
    const Intruder *search(int id) const;

    int size() const;
    bool isEmpty() const;

    Intruder *exportState() const;
};

#endif