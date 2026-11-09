#ifndef HASH_CHAINING_H
#define HASH_CHAINING_H

#include "../common/Intruder.h"

struct HashNode
{
    Intruder data;
    HashNode *next;
};

class HashChaining
{
private:
    HashNode **table;
    int capacity;
    int count;

    int hashFunction(int id) const;

public:
    explicit HashChaining(int tableSize);
    ~HashChaining();
    HashChaining(const HashChaining &) = delete;
    HashChaining &operator=(const HashChaining &) = delete;

    bool insert(Intruder record);
    bool remove(int id);
    const Intruder *search(int id) const;

    int size() const;
    bool isEmpty() const;
    int getCapacity() const;

    HashNode **exportState() const;
};

#endif