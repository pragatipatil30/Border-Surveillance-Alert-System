// ============================================================================
//  person3/GraphAlgorithms.h
//  Algorithms that run on the border-map Graph.
//
//  Syllabus (Unit 5): BFS and DFS, connected graph, bipartite graph, detecting
//  a cycle in a graph, minimum spanning tree (Prim's and Kruskal's),
//  shortest path (Dijkstra), Union-Find.
//
//  Rules for every function here
//  -----------------------------
//  * BLOCKED edges are treated as closed and are ignored.
//  * Connectivity, bipartite, cycle detection and the spanning-tree functions
//    are defined for UNDIRECTED graphs (the border map default).
//    BFS, DFS, hop count and Dijkstra also work on directed graphs.
//  * Every function takes an optional EventCallback. When given, it receives
//    one text line per step, which the Qt Structure Lab can show.
//
//  Project role
//      BFS / DFS ............ reachability, patrol coverage order
//      isConnected .......... "can every post reach every other post?"
//      isBipartite .......... can posts be split into two patrol zones
//      hasCycle ............. find redundant / looping routes
//      Dijkstra ............. fastest route from a patrol unit to a sighting
//      nearestCandidate ..... pick the closest free unit for dispatch
//      Prim / Kruskal ....... cheapest communication cable network
//
//  Complexity (V = vertices, E = edges)
//      BFS, DFS, components, bipartite, cycle ... O(V + E)
//      Dijkstra (array version) ................. O(V^2)
//      Prim (array version) ..................... O(V^2)
//      Kruskal (merge sort + Union-Find) ........ O(E log E)
// ============================================================================
#ifndef GRAPH_ALGORITHMS_H
#define GRAPH_ALGORITHMS_H

#include <vector>

#include "Graph.h"
#include "Helpers.h"
#include "UnionFind.h"

namespace GraphAlgo
{
    const int INF = 1000000000;     // "unreachable" distance

    struct PathResult
    {
        bool             found;
        int              distance;   // total km (Dijkstra) or number of hops (fewestHops)
        std::vector<int> path;       // vertex indices from source to target
        PathResult() : found(false), distance(0) {}
    };

    struct MSTEdge
    {
        int u;
        int v;
        int weight;
    };

    struct MSTResult
    {
        std::vector<MSTEdge> edges;
        int                  totalWeight;
        bool                 spansAll;   // false if the graph is disconnected
        MSTResult() : totalWeight(0), spansAll(false) {}
    };

    // ----- traversals (visit order is written into 'order') -----
    void bfs(const Graph& g, int start, std::vector<int>& order,
             const EventCallback& log = nullptr);
    void dfsRecursive(const Graph& g, int start, std::vector<int>& order,
                      const EventCallback& log = nullptr);
    void dfsIterative(const Graph& g, int start, std::vector<int>& order,
                      const EventCallback& log = nullptr);   // same order as the recursive one

    // ----- connectivity -----
    bool isConnected(const Graph& g);                                  // true for an empty graph
    int  connectedComponents(const Graph& g, std::vector<int>& label); // label[v] = component number
    PathResult fewestHops(const Graph& g, int source, int target,
                          const EventCallback& log = nullptr);         // BFS shortest path by edge count

    // ----- structure checks -----
    bool isBipartite(const Graph& g, std::vector<int>& color,
                     const EventCallback& log = nullptr);              // color[v] is 0 or 1
    bool hasCycle(const Graph& g, std::vector<int>* cycle = nullptr,
                  const EventCallback& log = nullptr);                 // optional: vertices of one cycle

    // ----- shortest paths (weighted) -----
    void dijkstra(const Graph& g, int source, std::vector<int>& dist,
                  std::vector<int>& prev, const EventCallback& log = nullptr);
    PathResult shortestPath(const Graph& g, int source, int target,
                            const EventCallback& log = nullptr);

    // Dispatch helper: which candidate vertex (e.g. a free patrol unit's post) is
    // closest to 'target'? Returns the index INTO 'candidates', or -1 if none can
    // reach it. 'route' receives the winning path (candidate -> target).
    int nearestCandidate(const Graph& g, int target,
                         const std::vector<int>& candidates, PathResult& route,
                         const EventCallback& log = nullptr);

    // ----- minimum spanning tree -----
    MSTResult prim(const Graph& g, int start = 0, const EventCallback& log = nullptr);
    MSTResult kruskal(const Graph& g, const EventCallback& log = nullptr);
}

#endif // GRAPH_ALGORITHMS_H
