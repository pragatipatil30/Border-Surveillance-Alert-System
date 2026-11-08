#include "HashLinearProbing.h"

HashLinearProbing::HashLinearProbing(int tableSize)
{
    capacity = (tableSize > 0) ? tableSize : 10;
    count = 0;

    table = new HashEntry[capacity];

    for (int i = 0; i < capacity; i++)
    {
        table[i].occupied = false;
        table[i].deleted = false;
    }
}

HashLinearProbing::~HashLinearProbing()
{
    delete[] table;
}

int HashLinearProbing::hashFunction(int id) const
{
    int index = id % capacity;

    if (index < 0)
        index += capacity;

    return index;
}

bool HashLinearProbing::insert(Intruder record)
{
    if (count == capacity)
        return false;

    int index = hashFunction(record.id);
    int firstDeleted = -1;

    for (int i = 0; i < capacity; i++)
    {
        int current = (index + i) % capacity;

        if (table[current].occupied)
        {
            if (table[current].data.id == record.id)
                return false;
        }
        else if (table[current].deleted)
        {
            if (firstDeleted == -1)
                firstDeleted = current;
        }
        else
        {
            int target = (firstDeleted != -1) ? firstDeleted : current;

            table[target].data = record;
            table[target].occupied = true;
            table[target].deleted = false;
            count++;

            return true;
        }
    }

    if (firstDeleted != -1)
    {
        table[firstDeleted].data = record;
        table[firstDeleted].occupied = true;
        table[firstDeleted].deleted = false;
        count++;

        return true;
    }

    return false;
}

const Intruder *HashLinearProbing::search(int id) const
{
    int index = hashFunction(id);

    for (int i = 0; i < capacity; i++)
    {
        int current = (index + i) % capacity;

        if (!table[current].occupied && !table[current].deleted)
            return nullptr;

        if (table[current].occupied &&
            table[current].data.id == id)
        {
            return &table[current].data;
        }
    }

    return nullptr;
}

bool HashLinearProbing::remove(int id)
{
    int index = hashFunction(id);

    for (int i = 0; i < capacity; i++)
    {
        int current = (index + i) % capacity;

        if (!table[current].occupied && !table[current].deleted)
            return false;

        if (table[current].occupied &&
            table[current].data.id == id)
        {
            table[current].occupied = false;
            table[current].deleted = true;
            count--;

            return true;
        }
    }

    return false;
}

int HashLinearProbing::size() const
{
    return count;
}

bool HashLinearProbing::isEmpty() const
{
    return count == 0;
}

int HashLinearProbing::getCapacity() const
{
    return capacity;
}

HashEntry *HashLinearProbing::exportState() const
{
    HashEntry *copy = new HashEntry[capacity];

    for (int i = 0; i < capacity; i++)
    {
        copy[i] = table[i];
    }

    return copy;
}