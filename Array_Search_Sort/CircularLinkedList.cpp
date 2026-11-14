#include "CircularLinkedList.h"

// Constructor
CircularLinkedList::CircularLinkedList()
{
    tail = nullptr;
    count = 0;
}

// Destructor
CircularLinkedList::~CircularLinkedList()
{
    if (tail == nullptr)
        return;

    CircularNode *head = tail->next;

    tail->next = nullptr;

    while (head != nullptr)
    {
        CircularNode *temp = head;
        head = head->next;
        delete temp;
    }
}

// Insert an intruder record at the end
void CircularLinkedList::insert(Intruder record)
{
    if (search(record.id) != nullptr)
        return;
    CircularNode *newNode = new CircularNode;

    newNode->data = record;

    if (tail == nullptr)
    {
        tail = newNode;
        newNode->next = newNode;
    }
    else
    {
        newNode->next = tail->next;
        tail->next = newNode;
        tail = newNode;
    }

    count++;
}

// Remove an intruder record using ID
bool CircularLinkedList::remove(int id)
{
    if (tail == nullptr)
        return false;

    CircularNode *current = tail->next;
    CircularNode *previous = tail;

    do
    {
        if (current->data.id == id)
        {
            // Only one node
            if (current == tail && current == tail->next)
            {
                tail = nullptr;
            }
            else
            {
                previous->next = current->next;

                if (current == tail)
                    tail = previous;
            }

            delete current;
            count--;

            return true;
        }

        previous = current;
        current = current->next;

    } while (current != tail->next);

    return false;
}

// Search for an intruder using ID
const Intruder *CircularLinkedList::search(int id) const
{
    if (tail == nullptr)
        return nullptr;

    CircularNode *current = tail->next;

    do
    {
        if (current->data.id == id)
            return &current->data;

        current = current->next;

    } while (current != tail->next);

    return nullptr;
}

// Return number of records
int CircularLinkedList::size() const
{
    return count;
}

// Check whether list is empty
bool CircularLinkedList::isEmpty() const
{
    return tail == nullptr;
}

// Export records for Qt visualization
Intruder *CircularLinkedList::exportState() const
{
    if (count == 0)
        return nullptr;

    Intruder *copy = new Intruder[count];

    CircularNode *current = tail->next;

    for (int i = 0; i < count; i++)
    {
        copy[i] = current->data;
        current = current->next;
    }

    return copy;
}