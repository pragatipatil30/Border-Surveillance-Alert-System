// ============================================================================
//  person3/BST.cpp
//  Implementation of the Binary Search Tree (see BST.h for the overview).
// ============================================================================
#include "BST.h"

#include <cctype>
#include <sstream>

// ---------------------------------------------------------------------------
//  Small helpers
// ---------------------------------------------------------------------------
std::string BST::toLower(const std::string& s)
{
    std::string result = s;
    for (size_t i = 0; i < result.size(); i++)
        result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
    return result;
}

// Returns -1 if a < b, 0 if equal, +1 if a > b. Key = (lowercase name, id).
int BST::compareKeys(const Intruder& a, const Intruder& b)
{
    std::string x = toLower(a.name);
    std::string y = toLower(b.name);
    if (x < y) return -1;
    if (x > y) return 1;
    if (a.id < b.id) return -1;
    if (a.id > b.id) return 1;
    return 0;
}

bool BST::startsWith(const std::string& text, const std::string& prefix)
{
    if (text.size() < prefix.size()) return false;
    return text.compare(0, prefix.size(), prefix) == 0;
}

void BST::emit(const std::string& message) const
{
    emitEvent(onEvent_, message);
}

// ---------------------------------------------------------------------------
//  Construction / destruction
// ---------------------------------------------------------------------------
BST::BST() : root_(nullptr), size_(0), comparisons_(0) {}

BST::~BST() { destroy(root_); }

void BST::destroy(Node* node)
{
    if (!node) return;
    destroy(node->left);
    destroy(node->right);
    delete node;
}

void BST::clear()
{
    destroy(root_);
    root_ = nullptr;
    size_ = 0;
}

// ---------------------------------------------------------------------------
//  Insert  -  walk down comparing keys until a free child slot is found.
// ---------------------------------------------------------------------------
bool BST::insert(const Intruder& item)
{
    comparisons_ = 0;

    if (!root_)
    {
        root_ = new Node(item);
        size_ = 1;
        emit("BST insert: tree is empty, '" + item.name + "' becomes the root");
        return true;
    }

    Node* current = root_;
    while (true)
    {
        comparisons_++;
        int c = compareKeys(item, current->data);

        if (c == 0)
        {
            emit("BST insert: '" + item.name + "' (id " + std::to_string(item.id) +
                 ") already exists, nothing inserted");
            return false;
        }

        const char* direction = (c < 0) ? "left" : "right";
        emit("BST insert: compare '" + item.name + "' with '" + current->data.name +
             "' -> go " + direction);

        Node*& next = (c < 0) ? current->left : current->right;
        if (!next)
        {
            next = new Node(item);
            size_++;
            emit("BST insert: '" + item.name + "' placed as " + direction +
                 " child of '" + current->data.name + "' (" +
                 std::to_string(comparisons_) + " comparisons)");
            return true;
        }
        current = next;
    }
}

// ---------------------------------------------------------------------------
//  Remove  -  three cases: leaf, one child, two children.
//  Two children: copy the in-order successor (smallest key in the right
//  subtree) into this node, then delete that successor from the right subtree.
// ---------------------------------------------------------------------------
bool BST::remove(const std::string& name, int id)
{
    Intruder key;
    key.name = name;
    key.id   = id;

    comparisons_ = 0;
    bool removed = false;
    root_ = removeNode(root_, key, removed);
    if (removed) size_--;
    else emit("BST remove: '" + name + "' (id " + std::to_string(id) + ") not found");
    return removed;
}

BST::Node* BST::removeNode(Node* node, const Intruder& key, bool& removed)
{
    if (!node) return nullptr;

    comparisons_++;
    int c = compareKeys(key, node->data);

    if (c < 0)
    {
        node->left = removeNode(node->left, key, removed);
    }
    else if (c > 0)
    {
        node->right = removeNode(node->right, key, removed);
    }
    else
    {
        removed = true;

        if (!node->left)                       // case 1 and 2a: no left child
        {
            emit("BST remove: '" + node->data.name + "' has no left child, right subtree moves up");
            Node* replacement = node->right;
            delete node;
            return replacement;
        }
        if (!node->right)                      // case 2b: only a left child
        {
            emit("BST remove: '" + node->data.name + "' has only a left child, it moves up");
            Node* replacement = node->left;
            delete node;
            return replacement;
        }

        // case 3: two children, use the in-order successor
        Node* successor = node->right;
        while (successor->left) successor = successor->left;

        emit("BST remove: '" + node->data.name + "' has two children, replaced by in-order successor '" +
             successor->data.name + "'");

        Intruder successorKey = successor->data;   // copy: the node itself is about to be deleted
        node->data = successorKey;
        bool ignored = false;
        node->right = removeNode(node->right, successorKey, ignored);
    }
    return node;
}

bool BST::removeById(int id)
{
    Intruder found;
    if (!searchById(id, found))
    {
        emit("BST remove: no intruder with id " + std::to_string(id));
        return false;
    }
    return remove(found.name, id);
}

// ---------------------------------------------------------------------------
//  Searching
// ---------------------------------------------------------------------------

// First match in sorted order: keep going left after a hit, so the smallest id
// among equal names is found.
bool BST::searchByName(const std::string& name, Intruder& out) const
{
    comparisons_ = 0;
    std::string target = toLower(name);

    const Node* current = root_;
    const Node* best = nullptr;
    while (current)
    {
        comparisons_++;
        std::string here = toLower(current->data.name);
        if (target < here)      current = current->left;
        else if (target > here) current = current->right;
        else { best = current; current = current->left; }
    }

    if (best)
    {
        out = best->data;
        emit("BST search: found '" + name + "' after " + std::to_string(comparisons_) + " comparisons");
        return true;
    }
    emit("BST search: '" + name + "' not found after " + std::to_string(comparisons_) + " comparisons");
    return false;
}

void BST::collectByName(const Node* node, const std::string& lowerName,
                        std::vector<Intruder>& out) const
{
    if (!node) return;
    comparisons_++;
    std::string here = toLower(node->data.name);

    if (lowerName < here)
    {
        collectByName(node->left, lowerName, out);
    }
    else if (lowerName > here)
    {
        collectByName(node->right, lowerName, out);
    }
    else
    {
        collectByName(node->left, lowerName, out);
        out.push_back(node->data);
        collectByName(node->right, lowerName, out);
    }
}

int BST::searchAllByName(const std::string& name, std::vector<Intruder>& out) const
{
    out.clear();
    comparisons_ = 0;
    collectByName(root_, toLower(name), out);
    return static_cast<int>(out.size());
}

// The tree is ordered by name, so searching by id has to look at every node.
bool BST::searchById(int id, Intruder& out) const
{
    comparisons_ = 0;
    if (!root_) return false;

    SimpleStack<const Node*> stack;
    stack.push(root_);
    while (!stack.empty())
    {
        const Node* node = stack.pop();
        comparisons_++;
        if (node->data.id == id)
        {
            out = node->data;
            return true;
        }
        if (node->right) stack.push(node->right);
        if (node->left)  stack.push(node->left);
    }
    return false;
}

// Autocomplete. Pruning idea: strings that start with P form one contiguous
// block in sorted order (everything >= P up to the end of that block).
//   node < P  -> node and its whole left subtree are too small, go right only
//   node >= P and does not start with P -> node is past the block, skip right
void BST::collectByPrefix(const Node* node, const std::string& lowerPrefix,
                          std::vector<Intruder>& out, int limit) const
{
    if (!node) return;
    if (limit > 0 && static_cast<int>(out.size()) >= limit) return;

    comparisons_++;
    std::string here = toLower(node->data.name);

    if (here < lowerPrefix)
    {
        collectByPrefix(node->right, lowerPrefix, out, limit);
        return;
    }

    collectByPrefix(node->left, lowerPrefix, out, limit);

    if (limit > 0 && static_cast<int>(out.size()) >= limit) return;

    if (startsWith(here, lowerPrefix))
    {
        out.push_back(node->data);
        collectByPrefix(node->right, lowerPrefix, out, limit);
    }
    // else: node is past the prefix block, so nothing to the right can match
}

void BST::prefixSearch(const std::string& prefix, std::vector<Intruder>& out, int limit) const
{
    out.clear();
    comparisons_ = 0;
    collectByPrefix(root_, toLower(prefix), out, limit);
    emit("BST prefix search '" + prefix + "': " + std::to_string(out.size()) +
         " match(es), " + std::to_string(comparisons_) + " nodes visited");
}

bool BST::findMin(Intruder& out) const
{
    if (!root_) return false;
    const Node* n = root_;
    while (n->left) n = n->left;
    out = n->data;
    return true;
}

bool BST::findMax(Intruder& out) const
{
    if (!root_) return false;
    const Node* n = root_;
    while (n->right) n = n->right;
    out = n->data;
    return true;
}

int BST::heightOf(const Node* node)
{
    if (!node) return 0;
    int l = heightOf(node->left);
    int r = heightOf(node->right);
    return 1 + (l > r ? l : r);
}

// ---------------------------------------------------------------------------
//  Traversals: recursive
// ---------------------------------------------------------------------------
void BST::inorderRec(const Node* node, std::vector<Intruder>& out)
{
    if (!node) return;
    inorderRec(node->left, out);
    out.push_back(node->data);
    inorderRec(node->right, out);
}

void BST::preorderRec(const Node* node, std::vector<Intruder>& out)
{
    if (!node) return;
    out.push_back(node->data);
    preorderRec(node->left, out);
    preorderRec(node->right, out);
}

void BST::postorderRec(const Node* node, std::vector<Intruder>& out)
{
    if (!node) return;
    postorderRec(node->left, out);
    postorderRec(node->right, out);
    out.push_back(node->data);
}

void BST::inorderRecursive(std::vector<Intruder>& out) const   { out.clear(); inorderRec(root_, out); }
void BST::preorderRecursive(std::vector<Intruder>& out) const  { out.clear(); preorderRec(root_, out); }
void BST::postorderRecursive(std::vector<Intruder>& out) const { out.clear(); postorderRec(root_, out); }

// ---------------------------------------------------------------------------
//  Traversals: non-recursive (explicit stack / queue, no recursion)
// ---------------------------------------------------------------------------

// Left, Node, Right: slide down the left spine pushing nodes, pop and visit,
// then move to the right child.
void BST::inorderIterative(std::vector<Intruder>& out) const
{
    out.clear();
    SimpleStack<const Node*> stack;
    const Node* current = root_;

    while (current || !stack.empty())
    {
        while (current)
        {
            stack.push(current);
            current = current->left;
        }
        current = stack.pop();
        out.push_back(current->data);
        current = current->right;
    }
}

// Node, Left, Right: pop, visit, push right then left (so left is popped first).
void BST::preorderIterative(std::vector<Intruder>& out) const
{
    out.clear();
    if (!root_) return;

    SimpleStack<const Node*> stack;
    stack.push(root_);
    while (!stack.empty())
    {
        const Node* node = stack.pop();
        out.push_back(node->data);
        if (node->right) stack.push(node->right);
        if (node->left)  stack.push(node->left);
    }
}

// Left, Right, Node: single stack plus a pointer to the last visited node, used
// to tell whether we are coming back up from the right subtree.
void BST::postorderIterative(std::vector<Intruder>& out) const
{
    out.clear();
    SimpleStack<const Node*> stack;
    const Node* current = root_;
    const Node* lastVisited = nullptr;

    while (current || !stack.empty())
    {
        if (current)
        {
            stack.push(current);
            current = current->left;
        }
        else
        {
            const Node* top = stack.peek();
            if (top->right && lastVisited != top->right)
            {
                current = top->right;
            }
            else
            {
                out.push_back(top->data);
                lastVisited = stack.pop();
            }
        }
    }
}

// Breadth-first, level by level, using a queue.
void BST::levelOrder(std::vector<Intruder>& out) const
{
    out.clear();
    if (!root_) return;

    SimpleQueue<const Node*> queue;
    queue.enqueue(root_);
    while (!queue.empty())
    {
        const Node* node = queue.dequeue();
        out.push_back(node->data);
        if (node->left)  queue.enqueue(node->left);
        if (node->right) queue.enqueue(node->right);
    }
}

// ---------------------------------------------------------------------------
//  Display
// ---------------------------------------------------------------------------

// Sideways tree: right subtree on top, root at the left edge, left subtree below.
void BST::drawRec(const Node* node, int depth, std::string& out)
{
    if (!node) return;
    drawRec(node->right, depth + 1, out);

    out += std::string(static_cast<size_t>(depth) * 6, ' ');
    out += node->data.name + " (#" + std::to_string(node->data.id) +
           ", T" + std::to_string(node->data.threatLevel) + ")\n";

    drawRec(node->left, depth + 1, out);
}

std::string BST::drawTree() const
{
    if (!root_) return "(empty tree)\n";
    std::string out;
    drawRec(root_, 0, out);
    return out;
}

// Level-order export: node 0 is the root, children get consecutive indices.
std::vector<BSTNodeInfo> BST::exportState() const
{
    std::vector<BSTNodeInfo> info(static_cast<size_t>(size_));
    if (!root_) return info;

    struct Item { const Node* node; int index; int parent; int depth; };
    SimpleQueue<Item> queue;
    int nextIndex = 1;
    queue.enqueue(Item{root_, 0, -1, 0});

    while (!queue.empty())
    {
        Item item = queue.dequeue();

        BSTNodeInfo& slot = info[static_cast<size_t>(item.index)];
        slot.index       = item.index;
        slot.id          = item.node->data.id;
        slot.name        = item.node->data.name;
        slot.threatLevel = item.node->data.threatLevel;
        slot.parent      = item.parent;
        slot.depth       = item.depth;
        slot.left        = -1;
        slot.right       = -1;

        if (item.node->left)
        {
            slot.left = nextIndex;
            queue.enqueue(Item{item.node->left, nextIndex++, item.index, item.depth + 1});
        }
        if (item.node->right)
        {
            slot.right = nextIndex;
            queue.enqueue(Item{item.node->right, nextIndex++, item.index, item.depth + 1});
        }
    }
    return info;
}
