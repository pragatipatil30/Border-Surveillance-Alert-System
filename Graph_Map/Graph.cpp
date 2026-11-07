// ============================================================================
//  person3/Graph.cpp
//  Implementation of the border-map graph (see Graph.h for the overview).
// ============================================================================
#include "Graph.h"

#include <sstream>

namespace
{
    const std::string kEmptyName = "";
}

// ---------------------------------------------------------------------------
//  Construction / destruction
// ---------------------------------------------------------------------------
Graph::Graph(bool directed, int initialCapacity)
    : vertices_(nullptr), count_(0),
      capacity_(initialCapacity > 0 ? initialCapacity : 8),
      edgeCount_(0), directed_(directed)
{
    vertices_ = new Vertex[capacity_];
}

Graph::~Graph()
{
    for (int i = 0; i < count_; i++)
    {
        Edge* e = vertices_[i].head;
        while (e)
        {
            Edge* next = e->next;
            delete e;
            e = next;
        }
    }
    delete[] vertices_;
}

void Graph::emit(const std::string& message) const
{
    emitEvent(onEvent_, message);
}

// Double the vertex array. Adjacency lists are not copied: only the head
// pointers move, so the old array can be freed without touching the edges.
void Graph::grow()
{
    int newCapacity = capacity_ * 2;
    Vertex* bigger = new Vertex[newCapacity];
    for (int i = 0; i < count_; i++)
        bigger[i] = vertices_[i];
    delete[] vertices_;
    vertices_ = bigger;
    capacity_ = newCapacity;
}

// ---------------------------------------------------------------------------
//  Building the map
// ---------------------------------------------------------------------------
int Graph::addVertex(const std::string& name, double x, double y)
{
    if (count_ == capacity_) grow();

    vertices_[count_].name = name;
    vertices_[count_].x    = x;
    vertices_[count_].y    = y;
    vertices_[count_].head = nullptr;
    count_++;

    emit("Graph: added checkpoint " + std::to_string(count_ - 1) + " '" + name + "'");
    return count_ - 1;
}

Edge* Graph::findEdge(int from, int to) const
{
    if (!isValid(from) || !isValid(to)) return nullptr;
    for (Edge* e = vertices_[from].head; e; e = e->next)
        if (e->to == to) return e;
    return nullptr;
}

bool Graph::addEdge(int u, int v, int weight)
{
    if (!isValid(u) || !isValid(v)) return false;
    if (u == v) return false;                 // no self loops
    if (weight <= 0) return false;            // distances must be positive
    if (findEdge(u, v)) return false;         // no duplicate routes

    // insert at the head of the list: O(1)
    vertices_[u].head = new Edge{v, weight, false, vertices_[u].head};
    if (!directed_)
        vertices_[v].head = new Edge{u, weight, false, vertices_[v].head};

    edgeCount_++;
    emit("Graph: route " + vertices_[u].name + (directed_ ? " -> " : " <-> ") +
         vertices_[v].name + " added (" + std::to_string(weight) + " km)");
    return true;
}

bool Graph::removeFromList(int from, int to)
{
    Edge* prev = nullptr;
    Edge* cur  = vertices_[from].head;
    while (cur)
    {
        if (cur->to == to)
        {
            if (prev) prev->next = cur->next;
            else      vertices_[from].head = cur->next;
            delete cur;
            return true;
        }
        prev = cur;
        cur  = cur->next;
    }
    return false;
}

bool Graph::removeEdge(int u, int v)
{
    if (!isValid(u) || !isValid(v)) return false;
    if (!removeFromList(u, v)) return false;
    if (!directed_) removeFromList(v, u);

    edgeCount_--;
    emit("Graph: route " + vertices_[u].name + " - " + vertices_[v].name + " removed");
    return true;
}

bool Graph::setBlocked(int u, int v, bool blocked)
{
    Edge* forward = findEdge(u, v);
    if (!forward) return false;
    forward->blocked = blocked;

    if (!directed_)
    {
        Edge* backward = findEdge(v, u);
        if (backward) backward->blocked = blocked;
    }

    emit("Graph: route " + vertices_[u].name + " - " + vertices_[v].name +
         (blocked ? " is now BLOCKED" : " reopened"));
    return true;
}

// ---------------------------------------------------------------------------
//  Queries
// ---------------------------------------------------------------------------
bool Graph::hasEdge(int u, int v) const { return findEdge(u, v) != nullptr; }

bool Graph::isBlocked(int u, int v) const
{
    Edge* e = findEdge(u, v);
    return e ? e->blocked : false;
}

int Graph::getWeight(int u, int v) const
{
    Edge* e = findEdge(u, v);
    return e ? e->weight : -1;
}

const std::string& Graph::name(int v) const
{
    return isValid(v) ? vertices_[v].name : kEmptyName;
}

double Graph::x(int v) const { return isValid(v) ? vertices_[v].x : 0.0; }
double Graph::y(int v) const { return isValid(v) ? vertices_[v].y : 0.0; }

int Graph::findVertex(const std::string& name) const
{
    for (int i = 0; i < count_; i++)
        if (vertices_[i].name == name) return i;
    return -1;
}

const Edge* Graph::neighbors(int v) const
{
    return isValid(v) ? vertices_[v].head : nullptr;
}

int Graph::degree(int v) const
{
    if (!isValid(v)) return 0;
    int d = 0;
    for (const Edge* e = vertices_[v].head; e; e = e->next) d++;
    return d;
}

// ---------------------------------------------------------------------------
//  Representations
// ---------------------------------------------------------------------------

// Adjacency MATRIX: matrix[i][j] = weight of edge i->j, or noEdgeValue.
std::vector<std::vector<int> > Graph::adjacencyMatrix(int noEdgeValue) const
{
    std::vector<std::vector<int> > matrix(
        static_cast<size_t>(count_),
        std::vector<int>(static_cast<size_t>(count_), noEdgeValue));

    for (int u = 0; u < count_; u++)
        for (const Edge* e = vertices_[u].head; e; e = e->next)
            matrix[static_cast<size_t>(u)][static_cast<size_t>(e->to)] = e->weight;

    return matrix;
}

std::string Graph::adjacencyListString() const
{
    std::ostringstream out;
    for (int u = 0; u < count_; u++)
    {
        out << u << " [" << vertices_[u].name << "] :";
        if (!vertices_[u].head) out << " (no routes)";
        for (const Edge* e = vertices_[u].head; e; e = e->next)
        {
            out << "  -> " << e->to << "(" << e->weight << "km"
                << (e->blocked ? ",BLOCKED" : "") << ")";
        }
        out << "\n";
    }
    return out.str();
}

std::string Graph::adjacencyMatrixString() const
{
    std::ostringstream out;
    out << "     ";
    for (int j = 0; j < count_; j++)
    {
        out.width(4);
        out << j;
    }
    out << "\n";

    for (int i = 0; i < count_; i++)
    {
        out.width(4);
        out << i << " ";
        for (int j = 0; j < count_; j++)
        {
            out.width(4);
            const Edge* e = findEdge(i, j);
            if (!e)               out << ".";
            else if (e->blocked)  out << "X";
            else                  out << e->weight;
        }
        out << "\n";
    }
    return out.str();
}

// ---------------------------------------------------------------------------
//  Visualizer export
// ---------------------------------------------------------------------------
GraphState Graph::exportState() const
{
    GraphState state;
    state.directed = directed_;

    for (int i = 0; i < count_; i++)
    {
        GraphVertexInfo info;
        info.index = i;
        info.name  = vertices_[i].name;
        info.x     = vertices_[i].x;
        info.y     = vertices_[i].y;
        state.vertices.push_back(info);
    }

    for (int u = 0; u < count_; u++)
    {
        for (const Edge* e = vertices_[u].head; e; e = e->next)
        {
            if (!directed_ && e->to < u) continue;   // list each undirected edge once
            GraphEdgeInfo info;
            info.from    = u;
            info.to      = e->to;
            info.weight  = e->weight;
            info.blocked = e->blocked;
            state.edges.push_back(info);
        }
    }
    return state;
}
