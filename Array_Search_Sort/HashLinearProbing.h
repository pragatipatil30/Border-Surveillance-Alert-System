#ifndef HASH_LINEAR_PROBING_H
#define HASH_LINEAR_PROBING_H

#include "../common/Intruder.h"

struct HashEntry
{
    Intruder data;
    bool occupied;
    bool deleted;
};

class HashLinearProbing
{
private:
    HashEntry *table;
    int capacity;
    int count;

    int hashFunction(int id) const;

public:
    explicit HashLinearProbing(int tableSize);
    ~HashLinearProbing();
    HashLinearProbing(const HashLinearProbing &) = delete;
    HashLinearProbing &operator=(const HashLinearProbing &) = delete;

    bool insert(Intruder record);
    bool remove(int id);
    const Intruder *search(int id) const;

    int size() const;
    bool isEmpty() const;
    int getCapacity() const;

    HashEntry *exportState() const;
};

#endif