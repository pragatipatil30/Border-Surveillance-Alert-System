#include "HashChaining.h"

// Constructor
HashChaining::HashChaining(int tableSize)
{
    capacity = (tableSize > 0) ? tableSize : 10;
    count = 0;

    table = new HashNode *[capacity];

    for (int i = 0; i < capacity; i++)
        table[i] = nullptr;
}

// Destructor
HashChaining::~HashChaining()
{
    for (int i = 0; i < capacity; i++)
    {
        HashNode *current = table[i];

        while (current != nullptr)
        {
            HashNode *temp = current;
            current = current->next;
            delete temp;
        }
    }

    delete[] table;
}

// Hash function
int HashChaining::hashFunction(int id) const
{
    int index = id % capacity;

    if (index < 0)
        index += capacity;

    return index;
}

// Insert an intruder record
bool HashChaining::insert(Intruder record)
{
    int index = hashFunction(record.id);

    // Check for duplicate ID
    HashNode *current = table[index];

    while (current != nullptr)
    {
        if (current->data.id == record.id)
            return false;

        current = current->next;
    }

    // Create a new node
    HashNode *newNode = new HashNode;

    newNode->data = record;
    newNode->next = table[index];

    table[index] = newNode;

    count++;

    return true;
}

// Search for an intruder by ID
const Intruder *HashChaining::search(int id) const
{
    int index = hashFunction(id);

    HashNode *current = table[index];

    while (current != nullptr)
    {
        if (current->data.id == id)
            return &current->data;

        current = current->next;
    }

    return nullptr;
}

// Remove an intruder by ID
bool HashChaining::remove(int id)
{
    int index = hashFunction(id);

    HashNode *current = table[index];
    HashNode *previous = nullptr;

    while (current != nullptr)
    {
        if (current->data.id == id)
        {
            if (previous == nullptr)
                table[index] = current->next;
            else
                previous->next = current->next;

            delete current;
            count--;

            return true;
        }

        previous = current;
        current = current->next;
    }

    return false;
}

// Return number of stored records
int HashChaining::size() const
{
    return count;
}

// Check whether hash table is empty
bool HashChaining::isEmpty() const
{
    return count == 0;
}

// Return table capacity
int HashChaining::getCapacity() const
{
    return capacity;
}

// Export a deep copy of the hash table
HashNode **HashChaining::exportState() const
{
    HashNode **copy = new HashNode *[capacity];

    for (int i = 0; i < capacity; i++)
    {
        copy[i] = nullptr;

        HashNode *current = table[i];
        HashNode *last = nullptr;

        while (current != nullptr)
        {
            HashNode *newNode = new HashNode;

            newNode->data = current->data;
            newNode->next = nullptr;

            if (copy[i] == nullptr)
                copy[i] = newNode;
            else
                last->next = newNode;

            last = newNode;
            current = current->next;
        }
    }

    return copy;
}