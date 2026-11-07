// ============================================================================
//  person3/BST.h
//  Binary Search Tree of Intruder records, keyed by NAME.
//
//  Syllabus (Unit 4): binary tree, BST, traversals (recursive and
//  non-recursive), operations on a binary tree.
//
//  Ordering rule
//  -------------
//  Names are compared case-insensitively. Two intruders may share a name, so
//  the real key is the pair (lowercase name, id). This keeps every key unique
//  and keeps duplicate names next to each other in sorted order.
//
//  Project role: "search by name" and "autocomplete suggestions" on the
//  Records screen, plus the BST view in the Structure Lab.
//
//  Complexity (h = tree height, n = number of nodes, k = matches returned)
//      insert / remove / search by name ..... O(h)   (O(log n) average, O(n) worst)
//      search by id (tree is ordered by name) O(n)
//      prefix search ........................ O(h + k)
//      every traversal ...................... O(n)
//
//  Design rule: no cin / cout in here. Text is returned as std::string, and
//  step-by-step messages go to the optional event callback.
// ============================================================================
#ifndef BST_H
#define BST_H

#include <string>
#include <vector>

#include "Intruder.h"
#include "Helpers.h"

// One node as the visualizer needs it (indices point into the exported array).
struct BSTNodeInfo
{
    int         index;
    int         id;
    std::string name;
    int         threatLevel;
    int         left;     // index of left child, or -1
    int         right;    // index of right child, or -1
    int         parent;   // index of parent, or -1 for the root
    int         depth;    // root = 0
};

class BST
{
private:
    struct Node
    {
        Intruder data;
        Node*    left;
        Node*    right;
        explicit Node(const Intruder& d) : data(d), left(nullptr), right(nullptr) {}
    };

    Node*         root_;
    int           size_;
    mutable int   comparisons_;      // key comparisons made by the last operation
    EventCallback onEvent_;

    // ----- helpers -----
    static std::string toLower(const std::string& s);
    static int         compareKeys(const Intruder& a, const Intruder& b);
    static bool        startsWith(const std::string& text, const std::string& prefix);
    void               emit(const std::string& message) const;

    static void destroy(Node* node);
    Node*       removeNode(Node* node, const Intruder& key, bool& removed);
    static int  heightOf(const Node* node);

    void collectByName(const Node* node, const std::string& lowerName,
                       std::vector<Intruder>& out) const;
    void collectByPrefix(const Node* node, const std::string& lowerPrefix,
                         std::vector<Intruder>& out, int limit) const;

    static void inorderRec(const Node* node, std::vector<Intruder>& out);
    static void preorderRec(const Node* node, std::vector<Intruder>& out);
    static void postorderRec(const Node* node, std::vector<Intruder>& out);
    static void drawRec(const Node* node, int depth, std::string& out);

public:
    BST();
    ~BST();

    // The tree owns raw nodes, so copying is disabled to avoid double deletes.
    BST(const BST&)            = delete;
    BST& operator=(const BST&) = delete;

    // ----- hook for the Structure Lab -----
    void setEventCallback(const EventCallback& cb) { onEvent_ = cb; }
    int  lastComparisons() const { return comparisons_; }

    // ----- core operations -----
    bool insert(const Intruder& item);                       // false if (name,id) already exists
    bool remove(const std::string& name, int id);            // false if not found
    bool removeById(int id);                                 // O(n): finds the name first
    void clear();

    bool searchByName(const std::string& name, Intruder& out) const;   // first match in sorted order
    int  searchAllByName(const std::string& name, std::vector<Intruder>& out) const;
    bool searchById(int id, Intruder& out) const;                      // O(n)
    void prefixSearch(const std::string& prefix, std::vector<Intruder>& out,
                      int limit = -1) const;                           // autocomplete

    // ----- queries -----
    bool isEmpty() const { return root_ == nullptr; }
    int  size()    const { return size_; }
    int  height()  const { return heightOf(root_); }   // empty = 0, single node = 1
    bool findMin(Intruder& out) const;
    bool findMax(Intruder& out) const;

    // ----- traversals: recursive versions -----
    void inorderRecursive(std::vector<Intruder>& out) const;
    void preorderRecursive(std::vector<Intruder>& out) const;
    void postorderRecursive(std::vector<Intruder>& out) const;

    // ----- traversals: non-recursive versions (explicit stack / queue) -----
    void inorderIterative(std::vector<Intruder>& out) const;
    void preorderIterative(std::vector<Intruder>& out) const;
    void postorderIterative(std::vector<Intruder>& out) const;
    void levelOrder(std::vector<Intruder>& out) const;

    // ----- display / visualizer -----
    std::string drawTree() const;                    // sideways ASCII picture
    std::vector<BSTNodeInfo> exportState() const;    // for the Qt BST view
};

#endif // BST_H
