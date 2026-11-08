//  person3/GraphAlgorithms.cpp
//  Implementation of the graph algorithms (see GraphAlgorithms.h).
#include "GraphAlgorithms.h"
namespace GraphAlgo
{

//  Private helpers
namespace
{
    std::string label(const Graph& g, int v)
    {
        return g.name(v) + "(" + std::to_string(v) + ")";
    }

    // Walk the 'prev' array backwards from target to source.
    // Returns false if target was never reached.
    bool buildPath(const std::vector<int>& prev, int source, int target,
                   std::vector<int>& path)
    {
        path.clear();
        int cur = target;
        while (cur != -1)
        {
            path.push_back(cur);
            if (cur == source) break;
            cur = prev[static_cast<size_t>(cur)];
        }
        if (path.empty() || path.back() != source) { path.clear(); return false; }

        // reverse in place so the path reads source -> target
        for (size_t i = 0, j = path.size() - 1; i < j; i++, j--)
        {
            int tmp = path[i]; path[i] = path[j]; path[j] = tmp;
        }
        return true;
    }

    // Recursive DFS used by dfsRecursive.
    void dfsVisit(const Graph& g, int u, std::vector<char>& seen,
                  std::vector<int>& order, const EventCallback& log)
    {
        seen[static_cast<size_t>(u)] = 1;
        order.push_back(u);
        emitEvent(log, "DFS: visit " + label(g, u));

        for (const Edge* e = g.neighbors(u); e; e = e->next)
        {
            if (e->blocked) continue;
            if (!seen[static_cast<size_t>(e->to)])
                dfsVisit(g, e->to, seen, order, log);
        }
    }

    // Recursive DFS for cycle detection in an undirected graph.
    // A visited neighbour that is not our parent means we found a back edge.
    bool cycleVisit(const Graph& g, int u, int parent, std::vector<char>& seen,
                    std::vector<int>& par, std::vector<int>* cycle,
                    const EventCallback& log)
    {
        seen[static_cast<size_t>(u)] = 1;
        for (const Edge* e = g.neighbors(u); e; e = e->next)
        {
            if (e->blocked) continue;
            int v = e->to;

            if (!seen[static_cast<size_t>(v)])
            {
                par[static_cast<size_t>(v)] = u;
                if (cycleVisit(g, v, u, seen, par, cycle, log)) return true;
            }
            else if (v != parent)
            {
                emitEvent(log, "Cycle: back edge " + label(g, u) + " -> " + label(g, v));
                if (cycle)
                {
                    cycle->clear();
                    int x = u;
                    while (x != v)
                    {
                        cycle->push_back(x);
                        x = par[static_cast<size_t>(x)];
                    }
                    cycle->push_back(v);
                }
                return true;
            }
        }
        return false;
    }

    // Own merge sort for Kruskal's edge list (sort by weight, then u, then v).
    bool edgeLess(const MSTEdge& a, const MSTEdge& b)
    {
        if (a.weight != b.weight) return a.weight < b.weight;
        if (a.u != b.u) return a.u < b.u;
        return a.v < b.v;
    }

    void mergeSortEdges(MSTEdge* a, MSTEdge* tmp, int lo, int hi)   // sorts [lo, hi)
    {
        if (hi - lo < 2) return;
        int mid = lo + (hi - lo) / 2;
        mergeSortEdges(a, tmp, lo, mid);
        mergeSortEdges(a, tmp, mid, hi);

        int i = lo, j = mid, k = lo;
        while (i < mid && j < hi)
            tmp[k++] = edgeLess(a[j], a[i]) ? a[j++] : a[i++];
        while (i < mid) tmp[k++] = a[i++];
        while (j < hi)  tmp[k++] = a[j++];
        for (int t = lo; t < hi; t++) a[t] = tmp[t];
    }
}


//  Traversals


// BFS: visit the start, then all its neighbours, then their neighbours...
// A queue holds the "frontier". O(V + E).
void bfs(const Graph& g, int start, std::vector<int>& order, const EventCallback& log)
{
    order.clear();
    int n = g.vertexCount();
    if (!g.isValid(start)) return;

    std::vector<char> seen(static_cast<size_t>(n), 0);
    SimpleQueue<int> queue;

    seen[static_cast<size_t>(start)] = 1;
    queue.enqueue(start);

    while (!queue.empty())
    {
        int u = queue.dequeue();
        order.push_back(u);
        emitEvent(log, "BFS: visit " + label(g, u));

        for (const Edge* e = g.neighbors(u); e; e = e->next)
        {
            if (e->blocked) continue;
            if (!seen[static_cast<size_t>(e->to)])
            {
                seen[static_cast<size_t>(e->to)] = 1;
                queue.enqueue(e->to);
            }
        }
    }
}

// DFS (recursive): go as deep as possible, then backtrack. O(V + E).
void dfsRecursive(const Graph& g, int start, std::vector<int>& order, const EventCallback& log)
{
    order.clear();
    if (!g.isValid(start)) return;
    std::vector<char> seen(static_cast<size_t>(g.vertexCount()), 0);
    dfsVisit(g, start, seen, order, log);
}

// DFS (non-recursive): the stack stores (vertex, next edge still to try), which
// is exactly what the call stack would hold, so the visit order is identical.
void dfsIterative(const Graph& g, int start, std::vector<int>& order, const EventCallback& log)
{
    order.clear();
    if (!g.isValid(start)) return;

    struct Frame { int vertex; const Edge* next; };

    std::vector<char> seen(static_cast<size_t>(g.vertexCount()), 0);
    SimpleStack<Frame> stack;

    seen[static_cast<size_t>(start)] = 1;
    order.push_back(start);
    emitEvent(log, "DFS: visit " + label(g, start));
    stack.push(Frame{start, g.neighbors(start)});

    while (!stack.empty())
    {
        Frame& top = stack.peek();

        // skip blocked edges and already visited neighbours
        while (top.next && (top.next->blocked || seen[static_cast<size_t>(top.next->to)]))
            top.next = top.next->next;

        if (!top.next)
        {
            stack.pop();                    // nothing left here: backtrack
            continue;
        }

        int v = top.next->to;
        top.next = top.next->next;          // remember where to continue later

        seen[static_cast<size_t>(v)] = 1;
        order.push_back(v);
        emitEvent(log, "DFS: visit " + label(g, v));
        stack.push(Frame{v, g.neighbors(v)});   // 'top' may dangle after this push
    }
}


//  Connectivity


// Labels every vertex with its component number (0, 1, 2, ...). O(V + E).
int connectedComponents(const Graph& g, std::vector<int>& labelOut)
{
    int n = g.vertexCount();
    labelOut.assign(static_cast<size_t>(n), -1);

    int components = 0;
    for (int s = 0; s < n; s++)
    {
        if (labelOut[static_cast<size_t>(s)] != -1) continue;

        SimpleQueue<int> queue;
        labelOut[static_cast<size_t>(s)] = components;
        queue.enqueue(s);

        while (!queue.empty())
        {
            int u = queue.dequeue();
            for (const Edge* e = g.neighbors(u); e; e = e->next)
            {
                if (e->blocked) continue;
                if (labelOut[static_cast<size_t>(e->to)] == -1)
                {
                    labelOut[static_cast<size_t>(e->to)] = components;
                    queue.enqueue(e->to);
                }
            }
        }
        components++;
    }
    return components;
}

bool isConnected(const Graph& g)
{
    if (g.vertexCount() == 0) return true;
    std::vector<int> labels;
    return connectedComponents(g, labels) == 1;
}

// Fewest edges between two checkpoints: BFS that remembers each vertex's parent.
PathResult fewestHops(const Graph& g, int source, int target, const EventCallback& log)
{
    PathResult result;
    if (!g.isValid(source) || !g.isValid(target)) return result;

    int n = g.vertexCount();
    std::vector<int>  prev(static_cast<size_t>(n), -1);
    std::vector<char> seen(static_cast<size_t>(n), 0);
    SimpleQueue<int>  queue;

    seen[static_cast<size_t>(source)] = 1;
    queue.enqueue(source);

    while (!queue.empty() && !seen[static_cast<size_t>(target)])
    {
        int u = queue.dequeue();
        for (const Edge* e = g.neighbors(u); e; e = e->next)
        {
            if (e->blocked || seen[static_cast<size_t>(e->to)]) continue;
            seen[static_cast<size_t>(e->to)] = 1;
            prev[static_cast<size_t>(e->to)] = u;
            queue.enqueue(e->to);
            emitEvent(log, "Hops: reached " + label(g, e->to) + " from " + label(g, u));
        }
    }

    if (buildPath(prev, source, target, result.path))
    {
        result.found    = true;
        result.distance = static_cast<int>(result.path.size()) - 1;
    }
    return result;
}


//  Bipartite check (2-colouring with BFS)


// A graph is bipartite if its vertices can be coloured with 2 colours so that
// every edge joins different colours. Equivalent: it has no odd-length cycle.
// Project use: split checkpoints into two patrol zones with no two adjacent
// posts in the same zone. Handles disconnected graphs. O(V + E).
bool isBipartite(const Graph& g, std::vector<int>& color, const EventCallback& log)
{
    int n = g.vertexCount();
    color.assign(static_cast<size_t>(n), -1);

    for (int s = 0; s < n; s++)
    {
        if (color[static_cast<size_t>(s)] != -1) continue;

        SimpleQueue<int> queue;
        color[static_cast<size_t>(s)] = 0;
        queue.enqueue(s);

        while (!queue.empty())
        {
            int u = queue.dequeue();
            for (const Edge* e = g.neighbors(u); e; e = e->next)
            {
                if (e->blocked) continue;
                int v = e->to;

                if (color[static_cast<size_t>(v)] == -1)
                {
                    color[static_cast<size_t>(v)] = 1 - color[static_cast<size_t>(u)];
                    queue.enqueue(v);
                    emitEvent(log, "Bipartite: " + label(g, v) + " gets zone " +
                                   std::to_string(color[static_cast<size_t>(v)]));
                }
                else if (color[static_cast<size_t>(v)] == color[static_cast<size_t>(u)])
                {
                    emitEvent(log, "Bipartite: conflict, " + label(g, u) + " and " +
                                   label(g, v) + " are adjacent but share zone " +
                                   std::to_string(color[static_cast<size_t>(u)]));
                    return false;
                }
            }
        }
    }
    return true;
}


//  Cycle detection


// Undirected graph, DFS based. Project use: find redundant or looping routes.
bool hasCycle(const Graph& g, std::vector<int>* cycle, const EventCallback& log)
{
    int n = g.vertexCount();
    if (cycle) cycle->clear();

    std::vector<char> seen(static_cast<size_t>(n), 0);
    std::vector<int>  par(static_cast<size_t>(n), -1);

    for (int s = 0; s < n; s++)
    {
        if (seen[static_cast<size_t>(s)]) continue;
        if (cycleVisit(g, s, -1, seen, par, cycle, log)) return true;
    }
    emitEvent(log, "Cycle: none found, the open routes form a forest");
    return false;
}


//  Dijkstra (array version, O(V^2))


// Repeat: pick the unfinished vertex with the smallest known distance, finalize
// it, then try to improve its neighbours ("relaxation"). Edge weights are > 0,
// which is what makes this greedy choice correct.
void dijkstra(const Graph& g, int source, std::vector<int>& dist,
              std::vector<int>& prev, const EventCallback& log)
{
    int n = g.vertexCount();
    dist.assign(static_cast<size_t>(n), INF);
    prev.assign(static_cast<size_t>(n), -1);
    if (!g.isValid(source)) return;

    std::vector<char> done(static_cast<size_t>(n), 0);
    dist[static_cast<size_t>(source)] = 0;

    for (int round = 0; round < n; round++)
    {
        int u = -1;
        for (int v = 0; v < n; v++)
            if (!done[static_cast<size_t>(v)] && dist[static_cast<size_t>(v)] < INF &&
                (u == -1 || dist[static_cast<size_t>(v)] < dist[static_cast<size_t>(u)]))
                u = v;

        if (u == -1) break;                 // everything left is unreachable
        done[static_cast<size_t>(u)] = 1;
        emitEvent(log, "Dijkstra: finalize " + label(g, u) + " at " +
                       std::to_string(dist[static_cast<size_t>(u)]) + " km");

        for (const Edge* e = g.neighbors(u); e; e = e->next)
        {
            if (e->blocked || done[static_cast<size_t>(e->to)]) continue;

            long long candidate = static_cast<long long>(dist[static_cast<size_t>(u)]) + e->weight;
            if (candidate < dist[static_cast<size_t>(e->to)])
            {
                dist[static_cast<size_t>(e->to)] = static_cast<int>(candidate);
                prev[static_cast<size_t>(e->to)] = u;
                emitEvent(log, "Dijkstra: shorter way to " + label(g, e->to) + " via " +
                               label(g, u) + " = " + std::to_string(candidate) + " km");
            }
        }
    }
}

PathResult shortestPath(const Graph& g, int source, int target, const EventCallback& log)
{
    PathResult result;
    if (!g.isValid(source) || !g.isValid(target)) return result;

    std::vector<int> dist, prev;
    dijkstra(g, source, dist, prev, log);

    if (dist[static_cast<size_t>(target)] >= INF) return result;   // unreachable
    if (buildPath(prev, source, target, result.path))
    {
        result.found    = true;
        result.distance = dist[static_cast<size_t>(target)];
    }
    return result;
}

// Dispatch: run Dijkstra from each candidate post and keep the closest.
int nearestCandidate(const Graph& g, int target, const std::vector<int>& candidates,
                     PathResult& route, const EventCallback& log)
{
    route = PathResult();
    int bestIndex = -1;

    for (size_t i = 0; i < candidates.size(); i++)
    {
        PathResult attempt = shortestPath(g, candidates[i], target);
        if (!attempt.found) continue;

        emitEvent(log, "Dispatch: unit at " + label(g, candidates[i]) + " is " +
                       std::to_string(attempt.distance) + " km away");

        if (bestIndex == -1 || attempt.distance < route.distance)
        {
            bestIndex = static_cast<int>(i);
            route = attempt;
        }
    }

    if (bestIndex != -1)
        emitEvent(log, "Dispatch: nearest unit is at " + label(g, candidates[static_cast<size_t>(bestIndex)]));
    else
        emitEvent(log, "Dispatch: no unit can reach " + label(g, target));
    return bestIndex;
}


//  Prim's algorithm (array version, O(V^2))


// Grow one tree from 'start'. Each round, add the vertex that is cheapest to
// connect to the tree. key[v] = cheapest edge linking v to the tree so far.
MSTResult prim(const Graph& g, int start, const EventCallback& log)
{
    MSTResult result;
    int n = g.vertexCount();
    if (n == 0) { result.spansAll = true; return result; }
    if (!g.isValid(start)) return result;

    std::vector<int>  key(static_cast<size_t>(n), INF);
    std::vector<int>  parent(static_cast<size_t>(n), -1);
    std::vector<char> inTree(static_cast<size_t>(n), 0);
    key[static_cast<size_t>(start)] = 0;

    int added = 0;
    for (int round = 0; round < n; round++)
    {
        int u = -1;
        for (int v = 0; v < n; v++)
            if (!inTree[static_cast<size_t>(v)] && key[static_cast<size_t>(v)] < INF &&
                (u == -1 || key[static_cast<size_t>(v)] < key[static_cast<size_t>(u)]))
                u = v;

        if (u == -1) break;                 // the rest is a different component
        inTree[static_cast<size_t>(u)] = 1;
        added++;

        if (parent[static_cast<size_t>(u)] != -1)
        {
            MSTEdge edge = { parent[static_cast<size_t>(u)], u, key[static_cast<size_t>(u)] };
            result.edges.push_back(edge);
            result.totalWeight += edge.weight;
            emitEvent(log, "Prim: add " + label(g, edge.u) + " - " + label(g, edge.v) +
                           " (" + std::to_string(edge.weight) + " km)");
        }

        for (const Edge* e = g.neighbors(u); e; e = e->next)
        {
            if (e->blocked || inTree[static_cast<size_t>(e->to)]) continue;
            if (e->weight < key[static_cast<size_t>(e->to)])
            {
                key[static_cast<size_t>(e->to)]    = e->weight;
                parent[static_cast<size_t>(e->to)] = u;
            }
        }
    }

    result.spansAll = (added == n);
    return result;
}


//  Kruskal's algorithm (O(E log E))


// Sort all edges by weight, then take each edge unless it would close a cycle.
// Union-Find answers "are these two ends already connected?" almost instantly.
MSTResult kruskal(const Graph& g, const EventCallback& log)
{
    MSTResult result;
    int n = g.vertexCount();
    if (n == 0) { result.spansAll = true; return result; }

    // collect every open edge (once per undirected edge)
    std::vector<MSTEdge> edges;
    for (int u = 0; u < n; u++)
    {
        for (const Edge* e = g.neighbors(u); e; e = e->next)
        {
            if (e->blocked) continue;
            if (!g.isDirected() && e->to < u) continue;
            MSTEdge edge = { u, e->to, e->weight };
            edges.push_back(edge);
        }
    }

    int m = static_cast<int>(edges.size());
    if (m > 1)
    {
        std::vector<MSTEdge> scratch(edges.size());
        mergeSortEdges(&edges[0], &scratch[0], 0, m);
    }

    UnionFind sets(n);
    for (int i = 0; i < m; i++)
    {
        const MSTEdge& edge = edges[static_cast<size_t>(i)];
        if (sets.unite(edge.u, edge.v))
        {
            result.edges.push_back(edge);
            result.totalWeight += edge.weight;
            emitEvent(log, "Kruskal: take " + label(g, edge.u) + " - " + label(g, edge.v) +
                           " (" + std::to_string(edge.weight) + " km)");
        }
        else
        {
            emitEvent(log, "Kruskal: skip " + label(g, edge.u) + " - " + label(g, edge.v) +
                           " (" + std::to_string(edge.weight) + " km), would form a cycle");
        }
    }

    result.spansAll = (sets.componentCount() == 1);
    return result;
}

} // namespace GraphAlgo
