#include "api/routes/dsa_routes.hpp"

#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "api/responses.hpp"
#include "api/services/dsa_runtime.hpp"

namespace border_api::routes {
namespace {

using crow::json::rvalue;
using crow::json::wvalue;

bool readInt(const rvalue& object, const char* key, int& value)
{
    if (!object.has(key) || object[key].t() != crow::json::type::Number) return false;
    const std::int64_t raw = object[key].i();
    if (raw < std::numeric_limits<int>::min() || raw > std::numeric_limits<int>::max()) return false;
    value = static_cast<int>(raw);
    return true;
}

bool readDouble(const rvalue& object, const char* key, double& value)
{
    if (!object.has(key) || object[key].t() != crow::json::type::Number) return false;
    value = object[key].d();
    return std::isfinite(value);
}

bool readString(const rvalue& object, const char* key, std::string& value, std::size_t maxLength = 120)
{
    if (!object.has(key) || object[key].t() != crow::json::type::String) return false;
    value = object[key].s();
    return !value.empty() && value.size() <= maxLength;
}

bool parseBody(const crow::request& request, rvalue& body)
{
    body = crow::json::load(request.body);
    return !body.error() && body.t() == crow::json::type::Object;
}

crow::response invalidRequest(const std::string& message)
{
    return errorResponse(400, "INVALID_REQUEST", message);
}

wvalue integerList(const std::vector<int>& values)
{
    wvalue result = wvalue::list();
    for (std::size_t index = 0; index < values.size(); ++index) result[index] = values[index];
    return result;
}

wvalue intState(const std::vector<int>& values)
{
    wvalue data;
    data["items"] = integerList(values);
    data["size"] = values.size();
    return data;
}

wvalue priorityItem(const dsa::PriorityItem& item)
{
    wvalue value;
    value["value"] = item.value;
    value["priority"] = item.priority;
    value["sequence"] = item.sequence;
    return value;
}

wvalue priorityState(const dsa::PriorityQueue& queue)
{
    wvalue data;
    wvalue items = wvalue::list();
    const auto& heap = queue.exportHeap();
    for (std::size_t index = 0; index < heap.size(); ++index) items[index] = priorityItem(heap[index]);
    data["heap"] = std::move(items);
    data["size"] = queue.size();
    if (const auto* item = queue.peek()) data["top"] = priorityItem(*item);
    else data["top"] = nullptr;
    return data;
}

wvalue intruderJson(const Intruder& record)
{
    wvalue value;
    value["id"] = record.id;
    value["name"] = record.name;
    value["location"] = record.location;
    value["threatLevel"] = record.threatlevel;
    return value;
}

wvalue hashState(HashChaining& map)
{
    wvalue data;
    data["size"] = map.size();
    data["capacity"] = map.getCapacity();
    wvalue buckets = wvalue::list();
    HashNode** exported = map.exportState();
    for (int index = 0; index < map.getCapacity(); ++index) {
        wvalue bucket = wvalue::list();
        std::size_t itemIndex = 0;
        HashNode* node = exported[index];
        while (node != nullptr) {
            HashNode* next = node->next;
            bucket[itemIndex++] = intruderJson(node->data);
            delete node;
            node = next;
        }
        buckets[static_cast<std::size_t>(index)] = std::move(bucket);
    }
    delete[] exported;
    data["buckets"] = std::move(buckets);
    return data;
}

wvalue graphState(const Graph* graph)
{
    wvalue data;
    data["configured"] = graph != nullptr;
    if (graph == nullptr) {
        data["directed"] = false;
        data["vertices"] = wvalue::list();
        data["edges"] = wvalue::list();
        return data;
    }

    const GraphState state = graph->exportState();
    data["directed"] = state.directed;
    wvalue vertices = wvalue::list();
    for (std::size_t i = 0; i < state.vertices.size(); ++i) {
        wvalue vertex;
        vertex["id"] = state.vertices[i].index;
        vertex["name"] = state.vertices[i].name;
        vertex["x"] = state.vertices[i].x;
        vertex["y"] = state.vertices[i].y;
        vertices[i] = std::move(vertex);
    }
    wvalue edges = wvalue::list();
    for (std::size_t i = 0; i < state.edges.size(); ++i) {
        wvalue edge;
        edge["from"] = state.edges[i].from;
        edge["to"] = state.edges[i].to;
        edge["weight"] = state.edges[i].weight;
        edge["blocked"] = state.edges[i].blocked;
        edges[i] = std::move(edge);
    }
    data["vertices"] = std::move(vertices);
    data["edges"] = std::move(edges);
    return data;
}

crow::response configureGraph(const crow::request& request)
{
    rvalue body;
    if (!parseBody(request, body) || !body.has("vertices") || !body.has("edges") ||
        body["vertices"].t() != crow::json::type::List || body["edges"].t() != crow::json::type::List) {
        return invalidRequest("A graph requires vertices and edges arrays");
    }
    const auto& vertices = body["vertices"];
    const auto& edges = body["edges"];
    if (vertices.size() > 256 || edges.size() > 4096) return invalidRequest("Graph exceeds the supported size limit");

    auto candidate = std::make_unique<Graph>(false);
    for (std::size_t index = 0; index < vertices.size(); ++index) {
        const auto& item = vertices[index];
        std::string name;
        double x = 0;
        double y = 0;
        if (item.t() != crow::json::type::Object || !readString(item, "name", name, 80) ||
            !readDouble(item, "x", x) || !readDouble(item, "y", y) ||
            std::abs(x) > 1000000 || std::abs(y) > 1000000) {
            return invalidRequest("Each vertex requires a name and finite x/y map coordinates");
        }
        candidate->addVertex(name, x, y);
    }
    for (std::size_t index = 0; index < edges.size(); ++index) {
        const auto& item = edges[index];
        int from = -1;
        int to = -1;
        int weight = 0;
        if (item.t() != crow::json::type::Object || !readInt(item, "from", from) ||
            !readInt(item, "to", to) || !readInt(item, "weight", weight) ||
            !candidate->addEdge(from, to, weight)) {
            return invalidRequest("Each edge requires valid vertex indices and a unique positive weight");
        }
    }

    auto& runtime = services::DsaRuntime::instance();
    std::lock_guard<std::mutex> lock(runtime.mutex);
    runtime.graph = std::move(candidate);
    return successResponse(graphState(runtime.graph.get()), "DSA graph configured");
}

crow::response traverseGraph(const crow::request& request, bool depthFirst)
{
    rvalue body;
    int start = -1;
    if (!parseBody(request, body) || !readInt(body, "start", start)) return invalidRequest("A numeric start vertex is required");
    auto& runtime = services::DsaRuntime::instance();
    std::lock_guard<std::mutex> lock(runtime.mutex);
    if (!runtime.graph) return errorResponse(409, "GRAPH_NOT_CONFIGURED", "Configure a DSA graph before running traversal");
    if (!runtime.graph->isValid(start)) return invalidRequest("Start vertex is outside the graph");

    std::vector<int> order;
    if (depthFirst) GraphAlgo::dfsIterative(*runtime.graph, start, order);
    else GraphAlgo::bfs(*runtime.graph, start, order);
    wvalue data;
    data["algorithm"] = depthFirst ? "DFS" : "BFS";
    data["order"] = integerList(order);
    wvalue names = wvalue::list();
    for (std::size_t i = 0; i < order.size(); ++i) names[i] = runtime.graph->name(order[i]);
    data["names"] = std::move(names);
    return successResponse(std::move(data), depthFirst ? "DFS completed" : "BFS completed");
}

crow::response shortestPath(const crow::request& request)
{
    rvalue body;
    int source = -1;
    int target = -1;
    if (!parseBody(request, body) || !readInt(body, "source", source) || !readInt(body, "target", target)) {
        return invalidRequest("Numeric source and target vertices are required");
    }
    auto& runtime = services::DsaRuntime::instance();
    std::lock_guard<std::mutex> lock(runtime.mutex);
    if (!runtime.graph) return errorResponse(409, "GRAPH_NOT_CONFIGURED", "Configure a DSA graph before running Dijkstra");
    if (!runtime.graph->isValid(source) || !runtime.graph->isValid(target)) return invalidRequest("Source or target is outside the graph");

    const auto result = GraphAlgo::shortestPath(*runtime.graph, source, target);
    wvalue data;
    data["found"] = result.found;
    data["distance"] = result.found ? wvalue(result.distance) : wvalue(nullptr);
    data["path"] = integerList(result.path);
    wvalue names = wvalue::list();
    for (std::size_t i = 0; i < result.path.size(); ++i) names[i] = runtime.graph->name(result.path[i]);
    data["names"] = std::move(names);
    return successResponse(std::move(data), result.found ? "Shortest path found" : "No route connects those vertices");
}

} // namespace

void registerDsaRoutes(HttpApp& app)
{
    CROW_ROUTE(app, "/api/dsa/stack").methods(crow::HTTPMethod::Get)
    ([] {
        auto& runtime = services::DsaRuntime::instance();
        std::lock_guard<std::mutex> lock(runtime.mutex);
        return successResponse(intState(runtime.stack.exportState()), "Stack state loaded");
    });
    CROW_ROUTE(app, "/api/dsa/stack/push").methods(crow::HTTPMethod::Post)
    ([](const crow::request& request) {
        rvalue body;
        int value = 0;
        if (!parseBody(request, body) || !readInt(body, "value", value)) return invalidRequest("A 32-bit integer value is required");
        auto& runtime = services::DsaRuntime::instance();
        std::lock_guard<std::mutex> lock(runtime.mutex);
        runtime.stack.push(value);
        return successResponse(intState(runtime.stack.exportState()), "Value pushed onto stack");
    });
    CROW_ROUTE(app, "/api/dsa/stack/pop").methods(crow::HTTPMethod::Post)
    ([] {
        auto& runtime = services::DsaRuntime::instance();
        std::lock_guard<std::mutex> lock(runtime.mutex);
        if (runtime.stack.empty()) return errorResponse(409, "STACK_EMPTY", "Cannot pop an empty stack");
        const int value = runtime.stack.pop();
        wvalue data = intState(runtime.stack.exportState());
        data["value"] = value;
        return successResponse(std::move(data), "Top value popped from stack");
    });
    CROW_ROUTE(app, "/api/dsa/stack/peek").methods(crow::HTTPMethod::Get)
    ([] {
        auto& runtime = services::DsaRuntime::instance();
        std::lock_guard<std::mutex> lock(runtime.mutex);
        if (runtime.stack.empty()) return errorResponse(409, "STACK_EMPTY", "Cannot peek an empty stack");
        wvalue data;
        data["value"] = runtime.stack.peek();
        return successResponse(std::move(data), "Top stack value loaded");
    });

    CROW_ROUTE(app, "/api/dsa/queue").methods(crow::HTTPMethod::Get)
    ([] {
        auto& runtime = services::DsaRuntime::instance();
        std::lock_guard<std::mutex> lock(runtime.mutex);
        return successResponse(intState(runtime.queue.exportState()), "Queue state loaded");
    });
    CROW_ROUTE(app, "/api/dsa/queue/enqueue").methods(crow::HTTPMethod::Post)
    ([](const crow::request& request) {
        rvalue body;
        int value = 0;
        if (!parseBody(request, body) || !readInt(body, "value", value)) return invalidRequest("A 32-bit integer value is required");
        auto& runtime = services::DsaRuntime::instance();
        std::lock_guard<std::mutex> lock(runtime.mutex);
        runtime.queue.enqueue(value);
        return successResponse(intState(runtime.queue.exportState()), "Value enqueued");
    });
    CROW_ROUTE(app, "/api/dsa/queue/dequeue").methods(crow::HTTPMethod::Post)
    ([] {
        auto& runtime = services::DsaRuntime::instance();
        std::lock_guard<std::mutex> lock(runtime.mutex);
        if (runtime.queue.empty()) return errorResponse(409, "QUEUE_EMPTY", "Cannot dequeue an empty queue");
        const int value = runtime.queue.dequeue();
        wvalue data = intState(runtime.queue.exportState());
        data["value"] = value;
        return successResponse(std::move(data), "Front value dequeued");
    });
    CROW_ROUTE(app, "/api/dsa/queue/front").methods(crow::HTTPMethod::Get)
    ([] {
        auto& runtime = services::DsaRuntime::instance();
        std::lock_guard<std::mutex> lock(runtime.mutex);
        if (runtime.queue.empty()) return errorResponse(409, "QUEUE_EMPTY", "Cannot read the front of an empty queue");
        wvalue data;
        data["value"] = runtime.queue.exportState().front();
        return successResponse(std::move(data), "Queue front loaded");
    });

    CROW_ROUTE(app, "/api/dsa/priority-queue").methods(crow::HTTPMethod::Get)
    ([] {
        auto& runtime = services::DsaRuntime::instance();
        std::lock_guard<std::mutex> lock(runtime.mutex);
        return successResponse(priorityState(runtime.priorityQueue), "Priority queue state loaded");
    });
    CROW_ROUTE(app, "/api/dsa/priority-queue/insert").methods(crow::HTTPMethod::Post)
    ([](const crow::request& request) {
        rvalue body;
        int value = 0;
        int priority = 0;
        if (!parseBody(request, body) || !readInt(body, "value", value) || !readInt(body, "priority", priority)) {
            return invalidRequest("Integer value and priority are required");
        }
        auto& runtime = services::DsaRuntime::instance();
        std::lock_guard<std::mutex> lock(runtime.mutex);
        runtime.priorityQueue.insert(value, priority);
        return successResponse(priorityState(runtime.priorityQueue), "Item inserted into priority queue");
    });
    CROW_ROUTE(app, "/api/dsa/priority-queue/extract").methods(crow::HTTPMethod::Post)
    ([] {
        auto& runtime = services::DsaRuntime::instance();
        std::lock_guard<std::mutex> lock(runtime.mutex);
        dsa::PriorityItem item{};
        if (!runtime.priorityQueue.extract(item)) return errorResponse(409, "PRIORITY_QUEUE_EMPTY", "Cannot extract from an empty priority queue");
        wvalue data = priorityState(runtime.priorityQueue);
        data["extracted"] = priorityItem(item);
        return successResponse(std::move(data), "Highest-priority item extracted");
    });
    CROW_ROUTE(app, "/api/dsa/priority-queue/peek").methods(crow::HTTPMethod::Get)
    ([] {
        auto& runtime = services::DsaRuntime::instance();
        std::lock_guard<std::mutex> lock(runtime.mutex);
        const auto* item = runtime.priorityQueue.peek();
        if (!item) return errorResponse(409, "PRIORITY_QUEUE_EMPTY", "Cannot peek an empty priority queue");
        wvalue data;
        data["item"] = priorityItem(*item);
        return successResponse(std::move(data), "Highest-priority item loaded");
    });

    CROW_ROUTE(app, "/api/dsa/hash-map").methods(crow::HTTPMethod::Get)
    ([] {
        auto& runtime = services::DsaRuntime::instance();
        std::lock_guard<std::mutex> lock(runtime.mutex);
        return successResponse(hashState(runtime.hashMap), "Hash table state loaded");
    });
    CROW_ROUTE(app, "/api/dsa/hash-map/insert").methods(crow::HTTPMethod::Post)
    ([](const crow::request& request) {
        rvalue body;
        int id = 0;
        int threatLevel = 0;
        std::string name;
        std::string location;
        if (!parseBody(request, body) || !readInt(body, "id", id) || id <= 0 ||
            !readString(body, "name", name) || !readString(body, "location", location) ||
            !readInt(body, "threatLevel", threatLevel) || threatLevel < 1 || threatLevel > 5) {
            return invalidRequest("Record requires a positive id, name, location and threatLevel from 1 to 5");
        }
        Intruder record{};
        record.id = id;
        record.name = std::move(name);
        record.location = std::move(location);
        record.threatlevel = threatLevel;
        auto& runtime = services::DsaRuntime::instance();
        std::lock_guard<std::mutex> lock(runtime.mutex);
        if (!runtime.hashMap.insert(record)) return errorResponse(409, "DUPLICATE_ID", "Hash table already contains this id");
        return successResponse(hashState(runtime.hashMap), "Record inserted into hash table");
    });
    CROW_ROUTE(app, "/api/dsa/hash-map/search").methods(crow::HTTPMethod::Post)
    ([](const crow::request& request) {
        rvalue body;
        int id = 0;
        if (!parseBody(request, body) || !readInt(body, "id", id) || id <= 0) return invalidRequest("A positive record id is required");
        auto& runtime = services::DsaRuntime::instance();
        std::lock_guard<std::mutex> lock(runtime.mutex);
        const Intruder* found = runtime.hashMap.search(id);
        if (!found) return errorResponse(404, "RECORD_NOT_FOUND", "Hash table does not contain this id");
        wvalue data;
        data["record"] = intruderJson(*found);
        return successResponse(std::move(data), "Record found in hash table");
    });
    CROW_ROUTE(app, "/api/dsa/hash-map/remove").methods(crow::HTTPMethod::Post)
    ([](const crow::request& request) {
        rvalue body;
        int id = 0;
        if (!parseBody(request, body) || !readInt(body, "id", id) || id <= 0) return invalidRequest("A positive record id is required");
        auto& runtime = services::DsaRuntime::instance();
        std::lock_guard<std::mutex> lock(runtime.mutex);
        if (!runtime.hashMap.remove(id)) return errorResponse(404, "RECORD_NOT_FOUND", "Hash table does not contain this id");
        return successResponse(hashState(runtime.hashMap), "Record removed from hash table");
    });

    CROW_ROUTE(app, "/api/dsa/graph").methods(crow::HTTPMethod::Get)
    ([] {
        auto& runtime = services::DsaRuntime::instance();
        std::lock_guard<std::mutex> lock(runtime.mutex);
        return successResponse(graphState(runtime.graph.get()), "DSA graph state loaded");
    });
    CROW_ROUTE(app, "/api/dsa/graph").methods(crow::HTTPMethod::Post)
    ([](const crow::request& request) { return configureGraph(request); });
    CROW_ROUTE(app, "/api/dsa/graph/bfs").methods(crow::HTTPMethod::Post)
    ([](const crow::request& request) { return traverseGraph(request, false); });
    CROW_ROUTE(app, "/api/dsa/graph/dfs").methods(crow::HTTPMethod::Post)
    ([](const crow::request& request) { return traverseGraph(request, true); });
    CROW_ROUTE(app, "/api/dsa/graph/dijkstra").methods(crow::HTTPMethod::Post)
    ([](const crow::request& request) { return shortestPath(request); });
}

} // namespace border_api::routes
