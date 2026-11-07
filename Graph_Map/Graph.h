// ============================================================================
//  person3/Graph.h
//  Weighted graph that models the BORDER MAP.
//
//  Vertices = checkpoints / patrol posts.   Edges = patrol routes (weight = km).
//
//  Syllabus (Unit 5): graph terminology, representation using an ADJACENCY LIST
//  (the real storage) and an ADJACENCY MATRIX (built on request for display and
//  comparison).
//
//  Notes
//  -----
//  * The map is UNDIRECTED by default (a route can be driven both ways).
//    Pass directed = true to the constructor for a directed graph.
//  * It is a simple graph: no self loops, no duplicate edges, weights > 0.
//  * An edge can be BLOCKED (flooded road, closed route). Every algorithm in
//    GraphAlgorithms.h ignores blocked edges, so "block a route and reroute" is
//    just: block the edge, run the algorithm again.
//  * Each vertex also stores x / y coordinates so the Qt map can draw it.
//
//  Complexity (V = vertices, E = edges, d = degree of a vertex)
//      addVertex ............ O(1) amortized
//      addEdge / removeEdge . O(d)  (checks for duplicates / walks the list)
//      hasEdge / getWeight .. O(d)
//      adjacencyMatrix ...... O(V^2) time and space
//      memory (list) ........ O(V + E)
// ============================================================================
#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>

#include "Helpers.h"

// One node of a vertex's adjacency list (singly linked list of outgoing edges).
struct Edge
{
    int   to;
    int   weight;
    bool  blocked;
    Edge* next;
};

// ----- plain data handed to the Qt visualizer -----
struct GraphVertexInfo { int index; std::string name; double x; double y; };
struct GraphEdgeInfo   { int from; int to; int weight; bool blocked; };
struct GraphState
{
    bool directed;
    std::vector<GraphVertexInfo> vertices;
    std::vector<GraphEdgeInfo>   edges;      // undirected edges appear once (from < to)
};

class Graph
{
private:
    struct Vertex
    {
        std::string name;
        double      x;
        double      y;
        Edge*       head;      // head of this vertex's adjacency list
        Vertex() : name(""), x(0), y(0), head(nullptr) {}
    };

    Vertex*       vertices_;
    int           count_;
    int           capacity_;
    int           edgeCount_;
    bool          directed_;
    EventCallback onEvent_;

    void  grow();
    Edge* findEdge(int from, int to) const;
    bool  removeFromList(int from, int to);
    void  emit(const std::string& message) const;

public:
    explicit Graph(bool directed = false, int initialCapacity = 8);
    ~Graph();

    // The graph owns linked lists of raw pointers, so copying is disabled.
    Graph(const Graph&)            = delete;
    Graph& operator=(const Graph&) = delete;

    void setEventCallback(const EventCallback& cb) { onEvent_ = cb; }

    // ----- building the map -----
    int  addVertex(const std::string& name, double x = 0.0, double y = 0.0);  // returns its index
    bool addEdge(int u, int v, int weight);       // false if invalid, duplicate, self loop or weight <= 0
    bool removeEdge(int u, int v);
    bool setBlocked(int u, int v, bool blocked);  // block / reopen a route (both directions if undirected)

    // ----- queries -----
    int  vertexCount() const { return count_; }
    int  edgeCount()   const { return edgeCount_; }
    bool isDirected()  const { return directed_; }
    bool isValid(int v) const { return v >= 0 && v < count_; }

    bool hasEdge(int u, int v) const;
    bool isBlocked(int u, int v) const;
    int  getWeight(int u, int v) const;           // -1 if there is no such edge

    const std::string& name(int v) const;
    double x(int v) const;
    double y(int v) const;
    int    findVertex(const std::string& name) const;   // -1 if not found
    const Edge* neighbors(int v) const;                 // head of the adjacency list (or nullptr)
    int    degree(int v) const;

    // ----- representations -----
    std::vector<std::vector<int> > adjacencyMatrix(int noEdgeValue = 0) const;
    std::string adjacencyListString() const;
    std::string adjacencyMatrixString() const;          // blocked edges shown as X, no edge as .

    // ----- visualizer -----
    GraphState exportState() const;
};

#endif // GRAPH_H
