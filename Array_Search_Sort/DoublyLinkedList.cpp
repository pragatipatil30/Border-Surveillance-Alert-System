#include "DoublyLinkedList.h"

// Constructor
DoublyLinkedList::DoublyLinkedList()
{
    head = nullptr;
    tail = nullptr;
    count = 0;
}

// Destructor
DoublyLinkedList::~DoublyLinkedList()
{
    DoublyNode *current = head;

    while (current != nullptr)
    {
        DoublyNode *temp = current;
        current = current->next;
        delete temp;
    }
}

// Insert an intruder record at the end
void DoublyLinkedList::insert(Intruder record)
{
    if (search(record.id) != nullptr)
        return;

    DoublyNode *newNode = new DoublyNode;

    newNode->data = record;
    newNode->next = nullptr;
    newNode->prev = tail;

    if (head == nullptr)
    {
        head = newNode;
        tail = newNode;
    }
    else
    {
        tail->next = newNode;
        tail = newNode;
    }

    count++;
}

// Remove an intruder record using ID
bool DoublyLinkedList::remove(int id)
{
    DoublyNode *current = head;

    while (current != nullptr)
    {
        if (current->data.id == id)
        {
            if (current->prev != nullptr)
                current->prev->next = current->next;
            else
                head = current->next;

            if (current->next != nullptr)
                current->next->prev = current->prev;
            else
                tail = current->prev;

            delete current;
            count--;

            return true;
        }

        current = current->next;
    }

    return false;
}

// Search for an intruder using ID
const Intruder *DoublyLinkedList::search(int id) const
{
    DoublyNode *current = head;

    while (current != nullptr)
    {
        if (current->data.id == id)
            return &current->data;

        current = current->next;
    }

    return nullptr;
}

// Return number of records
int DoublyLinkedList::size() const
{
    return count;
}

// Check whether list is empty
bool DoublyLinkedList::isEmpty() const
{
    return head == nullptr;
}

// Export records for Qt visualization
Intruder *DoublyLinkedList::exportState() const
{
    if (count == 0)
        return nullptr;

    Intruder *copy = new Intruder[count];

    DoublyNode *current = head;

    for (int i = 0; i < count; i++)
    {
        copy[i] = current->data;
        current = current->next;
    }

    return copy;
}
