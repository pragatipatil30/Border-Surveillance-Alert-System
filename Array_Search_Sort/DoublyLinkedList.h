#ifndef DOUBLY_LINKED_LIST_H
#define DOUBLY_LINKED_LIST_H

#include "../common/Intruder.h"

struct DoublyNode
{
    Intruder data;
    DoublyNode *next;
    DoublyNode *prev;
};

class DoublyLinkedList
{
private:
    DoublyNode *head;
    DoublyNode *tail;
    int count;

public:
    DoublyLinkedList();
    ~DoublyLinkedList();

    void insert(Intruder record);
    bool remove(int id);
    const Intruder *search(int id) const;

    int size() const;
    bool isEmpty() const;

    Intruder *exportState() const;
};

#endif