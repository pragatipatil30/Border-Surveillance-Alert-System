#ifndef SINGLY_LINKED_LIST_H
#define SINGLY_LINKED_LIST_H

#include "../common/Intruder.h"

struct SinglyNode
{
    Intruder data;
    SinglyNode *next;
};

class SinglyLinkedList
{
private:
    SinglyNode *head;
    int count;

public:
    SinglyLinkedList();
    ~SinglyLinkedList();
    SinglyLinkedList(const SinglyLinkedList &) = delete;
    SinglyLinkedList &operator=(const SinglyLinkedList &) = delete;


    void insert(Intruder record);
    bool remove(int id);
    const Intruder *search(int id) const;

    int size() const;
    bool isEmpty() const;

    Intruder *exportState() const;
};

#endif