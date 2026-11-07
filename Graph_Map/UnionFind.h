// ============================================================================
//  person3/UnionFind.h
//  Union-Find (Disjoint Set Union), header-only.
//
//  Syllabus (Unit 5): Union-Find, used inside Kruskal's algorithm and for
//  grouping linked checkpoints.
//
//  Two optimizations:
//      * path compression  - find() flattens the path it walks
//      * union by rank     - the shorter tree is attached under the taller one
//
//  Complexity: find / unite / connected are nearly O(1) amortized
//  (O(alpha(n)), where alpha is the inverse Ackermann function).
// ============================================================================
#ifndef UNION_FIND_H
#define UNION_FIND_H

#include <string>
#include <vector>

#include "Helpers.h"

class UnionFind
{
private:
    int*          parent_;
    int*          rank_;
    int           n_;
    int           components_;
    EventCallback onEvent_;

public:
    explicit UnionFind(int n)
        : parent_(new int[n > 0 ? n : 1]), rank_(new int[n > 0 ? n : 1]),
          n_(n > 0 ? n : 0), components_(n > 0 ? n : 0)
    {
        for (int i = 0; i < n_; i++)
        {
            parent_[i] = i;      // every element starts as its own set
            rank_[i]   = 0;
        }
    }

    ~UnionFind()
    {
        delete[] parent_;
        delete[] rank_;
    }

    UnionFind(const UnionFind&)            = delete;
    UnionFind& operator=(const UnionFind&) = delete;

    void setEventCallback(const EventCallback& cb) { onEvent_ = cb; }

    // Returns the representative (root) of x's set, or -1 if x is out of range.
    int find(int x)
    {
        if (x < 0 || x >= n_) return -1;

        int root = x;
        while (parent_[root] != root) root = parent_[root];

        while (parent_[x] != root)       // second pass: path compression
        {
            int next = parent_[x];
            parent_[x] = root;
            x = next;
        }
        return root;
    }

    // Merges the sets of a and b. Returns false if they were already together
    // (or an index is invalid). Kruskal uses this "false" to detect a cycle.
    bool unite(int a, int b)
    {
        int ra = find(a);
        int rb = find(b);
        if (ra == -1 || rb == -1) return false;
        if (ra == rb)
        {
            emitEvent(onEvent_, "UnionFind: " + std::to_string(a) + " and " +
                                std::to_string(b) + " already in the same set");
            return false;
        }

        if (rank_[ra] < rank_[rb])      parent_[ra] = rb;
        else if (rank_[ra] > rank_[rb]) parent_[rb] = ra;
        else { parent_[rb] = ra; rank_[ra]++; }

        components_--;
        emitEvent(onEvent_, "UnionFind: merged sets of " + std::to_string(a) +
                            " and " + std::to_string(b) + " (" +
                            std::to_string(components_) + " sets left)");
        return true;
    }

    bool connected(int a, int b)
    {
        int ra = find(a);
        int rb = find(b);
        return ra != -1 && ra == rb;
    }

    int componentCount() const { return components_; }
    int size()           const { return n_; }

    // For the visualizer: parent[i] for every element, after full compression.
    std::vector<int> exportState()
    {
        std::vector<int> state;
        for (int i = 0; i < n_; i++) state.push_back(find(i));
        return state;
    }
};

#endif // UNION_FIND_H
