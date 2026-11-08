#include "SinglyLinkedList.h"

// Constructor
SinglyLinkedList::SinglyLinkedList()
{
    head = nullptr;
    count = 0;
}

// Destructor
SinglyLinkedList::~SinglyLinkedList()
{
    SinglyNode *current = head;

    while (current != nullptr)
    {
        SinglyNode *temp = current;
        current = current->next;
        delete temp;
    }
}

// Insert an intruder record at the end
void SinglyLinkedList::insert(Intruder record)
{
    SinglyNode *newNode = new SinglyNode;

    newNode->data = record;
    newNode->next = nullptr;

    if (head == nullptr)
    {
        head = newNode;
    }
    else
    {
        SinglyNode *current = head;

        while (current->next != nullptr)
            current = current->next;

        current->next = newNode;
    }

    count++;
}

// Remove an intruder record using ID
bool SinglyLinkedList::remove(int id)
{
    if (head == nullptr)
        return false;

    if (head->data.id == id)
    {
        SinglyNode *temp = head;
        head = head->next;

        delete temp;
        count--;

        return true;
    }

    SinglyNode *current = head;

    while (current->next != nullptr)
    {
        if (current->next->data.id == id)
        {
            SinglyNode *temp = current->next;
            current->next = temp->next;

            delete temp;
            count--;

            return true;
        }

        current = current->next;
    }

    return false;
}

// Search for an intruder using ID
const Intruder *SinglyLinkedList::search(int id) const
{
    SinglyNode *current = head;

    while (current != nullptr)
    {
        if (current->data.id == id)
            return &current->data;

        current = current->next;
    }

    return nullptr;
}

// Return number of records
int SinglyLinkedList::size() const
{
    return count;
}

// Check whether list is empty
bool SinglyLinkedList::isEmpty() const
{
    return head == nullptr;
}

// Export records for Qt visualization
Intruder *SinglyLinkedList::exportState() const
{
    Intruder *copy = new Intruder[count];

    SinglyNode *current = head;

    for (int i = 0; i < count; i++)
    {
        copy[i] = current->data;
        current = current->next;
    }

    return copy;
}