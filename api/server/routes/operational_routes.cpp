#include "api/routes/operational_routes.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "api/responses.hpp"
#include "api/services/app_state.hpp"
#include "api/services/event_hub.hpp"
#include "GraphAlgorithms.h"

namespace border_api::routes {
namespace {
using crow::json::rvalue;
using crow::json::wvalue;

std::string nowIso()
{
    const std::time_t raw = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm utc{};
#if defined(_WIN32)
    gmtime_s(&utc, &raw);
#else
    gmtime_r(&raw, &utc);
#endif
    std::ostringstream out;
    out << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return out.str();
}

std::string threatName(int threat)
{
    return threat >= 4 ? "HIGH" : threat == 3 ? "MEDIUM" : "LOW";
}

services::RecordEntry* recordById(services::AppState& state, int id)
{
    const auto found = std::find_if(state.records.begin(), state.records.end(),
        [id](const auto& item) { return item.record.id == id; });
    return found == state.records.end() ? nullptr : &*found;
}

services::AlertEntry* alertById(services::AppState& state, const std::string& id)
{
    const auto found = std::find_if(state.alerts.begin(), state.alerts.end(),
        [&id](const auto& item) { return item.id == id; });
    return found == state.alerts.end() ? nullptr : &*found;
}

services::UnitEntry* unitById(services::AppState& state, const std::string& id)
{
    const auto found = std::find_if(state.units.begin(), state.units.end(),
        [&id](const auto& item) { return item.id == id; });
    return found == state.units.end() ? nullptr : &*found;
}

std::vector<services::AlertEntry*> orderedAlerts(services::AppState& state)
{
    std::vector<services::AlertEntry*> ordered;
    if (state.alertPolicy == "FCFS") {
        for (auto& item : state.alerts) if (item.status == "Queued") ordered.push_back(&item);
    } else {
        auto queue = state.waitingPriority;
        dsa::PriorityItem item{};
        while (queue.extract(item)) {
            const auto found = std::find_if(state.alerts.begin(), state.alerts.end(),
                [&item](const auto& candidate) {
                    return candidate.status == "Queued" && candidate.arrival == static_cast<std::size_t>(item.value);
                });
            if (found != state.alerts.end()) ordered.push_back(&*found);
        }
    }
    for (auto& item : state.alerts) if (item.status != "Queued") ordered.push_back(&item);
    return ordered;
}

wvalue recordJson(const services::RecordEntry& entry)
{
    wvalue item;
    item["id"] = std::to_string(entry.record.id);
    item["name"] = entry.record.name;
    item["location"] = entry.record.location;
    item["threatLevel"] = entry.record.threatlevel;
    item["status"] = entry.status;
    return item;
}

wvalue alertJson(const services::AlertEntry& alert)
{
    wvalue item;
    item["id"] = alert.id;
    item["intruderId"] = alert.intruderId;
    item["title"] = "Intruder sighting · " + std::to_string(alert.intruderId);
    item["checkpoint"] = alert.checkpoint;
    item["threatLevel"] = alert.threat;
    item["threat"] = threatName(alert.threat);
    item["status"] = alert.status;
    item["assignedUnit"] = alert.assignedUnit;
    item["distanceKm"] = alert.distanceKm;
    item["time"] = "Demo event";
    wvalue route = wvalue::list();
    for (std::size_t index = 0; index < alert.route.size(); ++index) route[index] = alert.route[index];
    item["route"] = std::move(route);
    return item;
}

wvalue unitJson(const services::UnitEntry& unit)
{
    wvalue item;
    item["id"] = unit.id;
    item["checkpoint"] = unit.checkpoint;
    item["status"] = unit.status;
    item["assignment"] = unit.assignment;
    item["updated"] = unit.updated;
    return item;
}

wvalue graphJson(const Graph* graph)
{
    wvalue data;
    if (graph == nullptr) {
        data["vertices"] = wvalue::list();
        data["edges"] = wvalue::list();
        return data;
    }
    const auto state = graph->exportState();
    wvalue vertices = wvalue::list();
    for (std::size_t index = 0; index < state.vertices.size(); ++index) {
        wvalue vertex;
        vertex["id"] = state.vertices[index].index;
        vertex["name"] = state.vertices[index].name;
        vertex["x"] = state.vertices[index].x;
        vertex["y"] = state.vertices[index].y;
        vertices[index] = std::move(vertex);
    }
    wvalue edges = wvalue::list();
    for (std::size_t index = 0; index < state.edges.size(); ++index) {
        wvalue edge;
        edge["from"] = state.edges[index].from;
        edge["to"] = state.edges[index].to;
        edge["weight"] = state.edges[index].weight;
        edge["blocked"] = state.edges[index].blocked;
        edges[index] = std::move(edge);
    }
    data["vertices"] = std::move(vertices);
    data["edges"] = std::move(edges);
    data["directed"] = state.directed;
    return data;
}

bool readText(const rvalue& body, const char* key, std::string& value)
{
    if (!body.has(key) || body[key].t() != crow::json::type::String) return false;
    value = body[key].s();
    return !value.empty() && value.size() <= 100;
}

bool readNumber(const rvalue& body, const char* key, int& value)
{
    if (!body.has(key) || body[key].t() != crow::json::type::Number) return false;
    value = static_cast<int>(body[key].i());
    return true;
}

bool parseBody(const crow::request& request, rvalue& body)
{
    body = crow::json::load(request.body);
    return !body.error() && body.t() == crow::json::type::Object;
}

void addActivity(services::AppState& state, const std::string& category, const std::string& message)
{
    state.activity.insert(state.activity.begin(), {
        "EV-" + std::to_string(state.activity.size() + 1), category, message, nowIso()
    });
    if (state.activity.size() > 200) state.activity.pop_back();
}

wvalue eventData(const std::string& id, const std::string& message)
{
    wvalue data;
    data["id"] = id;
    data["message"] = message;
    return data;
}

std::string dispatch(services::AppState& state, services::AlertEntry& alert)
{
    if (alert.status != "Queued") return "Alert is not queued";
    if (!state.productionGraph) return "Checkpoint graph is unavailable";
    std::vector<int> candidates;
    std::vector<services::UnitEntry*> available;
    for (auto& unit : state.units) {
        if (unit.status != "Available") continue;
        const int post = state.productionGraph->findVertex(unit.checkpoint);
        if (post >= 0) { candidates.push_back(post); available.push_back(&unit); }
    }
    const int target = state.productionGraph->findVertex(alert.checkpoint);
    if (target < 0) return "Alert checkpoint is not in the graph";
    GraphAlgo::PathResult path;
    const int candidate = GraphAlgo::nearestCandidate(*state.productionGraph, target, candidates, path);
    if (candidate < 0 || static_cast<std::size_t>(candidate) >= available.size()) return "No available unit can reach this checkpoint";
    auto& unit = *available[static_cast<std::size_t>(candidate)];
    alert.assignedUnit = unit.id;
    alert.status = "Dispatched";
    alert.distanceKm = path.distance;
    alert.route.clear();
    for (const int vertex : path.path) alert.route.push_back(state.productionGraph->name(vertex));
    unit.status = "Dispatched";
    unit.assignment = alert.id;
    unit.updated = nowIso();
    if (auto* record = recordById(state, alert.intruderId)) {
        record->status = "Dispatched";
        record->record.status = "Dispatched";
        state.rebuildRecordIndexes();
        if (!state.saveRecords()) return "Could not persist the record status";
    }
    return {};
}

crow::response changeRoute(const crow::request& request, bool blocked)
{
    rvalue body; std::string from; std::string to;
    if (!parseBody(request, body) || !readText(body, "from", from) || !readText(body, "to", to))
        return errorResponse(400, "INVALID_REQUEST", "Route change requires from and to checkpoint names");
    auto& state = services::AppState::instance(); std::lock_guard<std::mutex> lock(state.mutex);
    const int a = state.productionGraph->findVertex(from), b = state.productionGraph->findVertex(to);
    if (a < 0 || b < 0) return errorResponse(404, "CHECKPOINT_NOT_FOUND", "Route endpoint was not found");
    if (!state.productionGraph->setBlocked(a, b, blocked)) return errorResponse(404, "ROUTE_NOT_FOUND", "Route does not exist");
    addActivity(state, "graph", "Route " + from + " to " + to + (blocked ? " blocked" : " unblocked"));
    services::EventHub::instance().publish("graph.changed", graphJson(state.productionGraph.get()));
    return successResponse(graphJson(state.productionGraph.get()), blocked ? "Route blocked" : "Route reopened");
}

void publishStateChange(const std::string& event, const std::string& id, const std::string& message)
{
    services::EventHub::instance().publish(event, eventData(id, message));
}

} // namespace

void registerOperationalRoutes(HttpApp& app)
{
    CROW_ROUTE(app, "/api/graph").methods(crow::HTTPMethod::Get)
    ([] {
        auto& state = services::AppState::instance();
        std::lock_guard<std::mutex> lock(state.mutex);
        return successResponse(graphJson(state.productionGraph.get()), "Production checkpoint graph loaded");
    });

    CROW_ROUTE(app, "/api/graph/checkpoints").methods(crow::HTTPMethod::Post)
    ([](const crow::request& request) {
        rvalue body; std::string name; int x = 0; int y = 0;
        if (!parseBody(request, body) || !readText(body, "name", name) || !readNumber(body, "x", x) || !readNumber(body, "y", y))
            return errorResponse(400, "INVALID_REQUEST", "Checkpoint requires a name and integer x/y coordinates");
        auto& state = services::AppState::instance();
        std::lock_guard<std::mutex> lock(state.mutex);
        if (!state.productionGraph || state.productionGraph->findVertex(name) >= 0)
            return errorResponse(409, "CHECKPOINT_EXISTS", "Checkpoint already exists or the graph is unavailable");
        state.productionGraph->addVertex(name, x, y);
        addActivity(state, "graph", "Checkpoint " + name + " added");
        auto graph = graphJson(state.productionGraph.get());
        services::EventHub::instance().publish("graph.changed", std::move(graph));
        return successResponse(graphJson(state.productionGraph.get()), "Checkpoint added");
    });

    CROW_ROUTE(app, "/api/graph/routes").methods(crow::HTTPMethod::Post, crow::HTTPMethod::Delete)
    ([](const crow::request& request) -> crow::response {
        rvalue body; std::string from; std::string to; int weight = 0;
        if (!parseBody(request, body) || !readText(body, "from", from) || !readText(body, "to", to) ||
            (request.method == crow::HTTPMethod::Post && !readNumber(body, "weight", weight)))
            return errorResponse(400, "INVALID_REQUEST", "Route requires from, to and, when adding, a positive weight");
        auto& state = services::AppState::instance();
        std::lock_guard<std::mutex> lock(state.mutex);
        const int a = state.productionGraph->findVertex(from), b = state.productionGraph->findVertex(to);
        if (a < 0 || b < 0) return errorResponse(404, "CHECKPOINT_NOT_FOUND", "Route endpoint was not found");
        const bool changed = request.method == crow::HTTPMethod::Post
            ? state.productionGraph->addEdge(a, b, weight)
            : state.productionGraph->removeEdge(a, b);
        if (!changed) return errorResponse(409, "ROUTE_NOT_CHANGED", "Route is invalid, duplicated, or does not exist");
        addActivity(state, "graph", "Route " + from + " to " + to + (request.method == crow::HTTPMethod::Post ? " added" : " removed"));
        services::EventHub::instance().publish("graph.changed", graphJson(state.productionGraph.get()));
        return successResponse(graphJson(state.productionGraph.get()), "Route updated");
    });

    CROW_ROUTE(app, "/api/graph/routes/block").methods(crow::HTTPMethod::Post)
    ([](const crow::request& request) { return changeRoute(request, true); });
    CROW_ROUTE(app, "/api/graph/routes/unblock").methods(crow::HTTPMethod::Post)
    ([](const crow::request& request) { return changeRoute(request, false); });

    CROW_ROUTE(app, "/api/alerts").methods(crow::HTTPMethod::Get, crow::HTTPMethod::Post)
    ([](const crow::request& request) -> crow::response {
        auto& state = services::AppState::instance();
        if (request.method == crow::HTTPMethod::Get) {
            std::lock_guard<std::mutex> lock(state.mutex);
            wvalue data; wvalue items = wvalue::list();
            const auto ordered = orderedAlerts(state);
            for (std::size_t index = 0; index < ordered.size(); ++index) items[index] = alertJson(*ordered[index]);
            data["alerts"] = std::move(items); data["policy"] = state.alertPolicy;
            return successResponse(std::move(data), "Alerts loaded in backend order");
        }
        rvalue body; int id = 0;
        if (!parseBody(request, body) || !readNumber(body, "intruderId", id)) return errorResponse(400, "INVALID_REQUEST", "A numeric intruderId is required");
        std::lock_guard<std::mutex> lock(state.mutex);
        auto* record = recordById(state, id);
        if (!record) return errorResponse(404, "RECORD_NOT_FOUND", "Intruder record does not exist");
        const std::string alertId = "A-" + std::to_string(101 + state.nextAlertSequence);
        services::AlertEntry alert{alertId, id, record->record.threatlevel, record->record.location, "Queued", "", {}, state.nextAlertSequence++};
        state.incomingAlerts.enqueue(static_cast<int>(alert.arrival));
        state.waitingPriority.insert(static_cast<int>(alert.arrival), alert.threat);
        state.alerts.push_back(alert);
        addActivity(state, "alert", alertId + " queued for " + std::to_string(id));
        auto result = alertJson(alert);
        services::EventHub::instance().publish("alert.created", alertJson(alert));
        wvalue activityEvent; activityEvent["category"] = "alert"; activityEvent["message"] = alertId + " queued";
        services::EventHub::instance().publish("activity.added", std::move(activityEvent));
        return successResponse(std::move(result), "Alert queued");
    });

    CROW_ROUTE(app, "/api/alerts/policy").methods(crow::HTTPMethod::Post)
    ([](const crow::request& request) {
        rvalue body; std::string policy;
        if (!parseBody(request, body) || !readText(body, "policy", policy) || (policy != "PRIORITY" && policy != "FCFS"))
            return errorResponse(400, "INVALID_POLICY", "Policy must be PRIORITY or FCFS");
        auto& state = services::AppState::instance(); std::lock_guard<std::mutex> lock(state.mutex);
        state.alertPolicy = policy;
        wvalue data; data["policy"] = state.alertPolicy;
        wvalue queue = wvalue::list();
        const auto ordered = orderedAlerts(state);
        std::size_t index = 0;
        for (const auto* alert : ordered) if (alert->status == "Queued") queue[index++] = alertJson(*alert);
        data["waiting"] = std::move(queue);
        return successResponse(std::move(data), "Alert policy updated");
    });

    CROW_ROUTE(app, "/api/alerts/queues").methods(crow::HTTPMethod::Get)
    ([] {
        auto& state = services::AppState::instance(); std::lock_guard<std::mutex> lock(state.mutex);
        wvalue data; wvalue incoming = wvalue::list();
        const auto slots = state.incomingAlerts.exportSlots();
        const auto occupied = state.incomingAlerts.exportOccupied();
        for (std::size_t i = 0; i < slots.size(); ++i) {
            wvalue slot; slot["index"] = i; slot["occupied"] = occupied[i];
            if (occupied[i]) {
                const auto found = std::find_if(state.alerts.begin(), state.alerts.end(), [&slots, i](const auto& alert) {
                    return alert.arrival == static_cast<std::size_t>(slots[i]);
                });
                if (found != state.alerts.end()) slot["alertId"] = found->id;
            } else slot["alertId"] = nullptr;
            incoming[i] = std::move(slot);
        }
        data["incoming"] = std::move(incoming);
        data["capacity"] = state.incomingAlerts.capacity();
        data["frontIndex"] = state.incomingAlerts.frontIndex();
        data["rearIndex"] = state.incomingAlerts.rearIndex();
        wvalue waiting = wvalue::list();
        const auto ordered = orderedAlerts(state); std::size_t index = 0;
        for (const auto* alert : ordered) if (alert->status == "Queued") waiting[index++] = alertJson(*alert);
        data["waiting"] = std::move(waiting); data["policy"] = state.alertPolicy;
        return successResponse(std::move(data), "Alert queues loaded from C++ state");
    });

    auto dispatchHandler = [](const std::string& id) -> crow::response {
        auto& state = services::AppState::instance();
        std::lock_guard<std::mutex> lock(state.mutex);
        auto* alert = alertById(state, id);
        if (!alert) return errorResponse(404, "ALERT_NOT_FOUND", "Alert was not found");
        const std::string error = dispatch(state, *alert);
        if (!error.empty()) return errorResponse(409, "DISPATCH_FAILED", error);
        auto data = alertJson(*alert);
        if (auto* unit = unitById(state, alert->assignedUnit)) data["unit"] = unitJson(*unit);
        addActivity(state, "dispatch", "Unit " + alert->assignedUnit + " dispatched to " + alert->id);
        services::EventHub::instance().publish("alert.dispatched", alertJson(*alert));
        if (auto* unit = unitById(state, alert->assignedUnit)) services::EventHub::instance().publish("unit.updated", unitJson(*unit));
        wvalue activityEvent; activityEvent["category"] = "dispatch"; activityEvent["message"] = alert->id + " dispatched";
        services::EventHub::instance().publish("activity.added", std::move(activityEvent));
        return successResponse(std::move(data), "Alert dispatched using the nearest available unit");
    };

    CROW_ROUTE(app, "/api/alerts/dispatch-next").methods(crow::HTTPMethod::Post)
    ([dispatchHandler]() mutable -> crow::response {
        auto& state = services::AppState::instance();
        std::string id;
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            const auto ordered = orderedAlerts(state);
            const auto found = std::find_if(ordered.begin(), ordered.end(), [](const auto* alert) { return alert->status == "Queued"; });
            if (found == ordered.end()) return errorResponse(409, "QUEUE_EMPTY", "There are no queued alerts");
            id = (*found)->id;
        }
        return dispatchHandler(id);
    });

    CROW_ROUTE(app, "/api/alerts/<string>/dispatch").methods(crow::HTTPMethod::Post)
    ([dispatchHandler](const std::string& id) mutable { return dispatchHandler(id); });

    CROW_ROUTE(app, "/api/alerts/<string>/resolve").methods(crow::HTTPMethod::Post)
    ([](const std::string& id) {
        auto& state = services::AppState::instance(); std::lock_guard<std::mutex> lock(state.mutex);
        auto* alert = alertById(state, id);
        if (!alert) return errorResponse(404, "ALERT_NOT_FOUND", "Alert was not found");
        if (alert->status == "Resolved") return errorResponse(409, "ALREADY_RESOLVED", "Alert is already resolved");
        alert->status = "Resolved";
        if (auto* record = recordById(state, alert->intruderId)) {
            record->status = "Resolved"; record->record.status = "Resolved";
            state.rebuildRecordIndexes();
            if (!state.saveRecords()) return errorResponse(500, "SAVE_FAILED", "Could not persist the resolved record status");
        }
        if (!alert->assignedUnit.empty()) {
            if (auto* unit = unitById(state, alert->assignedUnit)) {
                unit->status = "Available"; unit->assignment = "none"; unit->updated = nowIso();
            }
            alert->assignedUnit.clear();
        }
        addActivity(state, "resolution", alert->id + " resolved");
        auto data = alertJson(*alert);
        services::EventHub::instance().publish("alert.resolved", alertJson(*alert));
        wvalue activityEvent; activityEvent["category"] = "resolution"; activityEvent["message"] = alert->id + " resolved";
        services::EventHub::instance().publish("activity.added", std::move(activityEvent));
        return successResponse(std::move(data), "Alert resolved");
    });

    CROW_ROUTE(app, "/api/patrol-units").methods(crow::HTTPMethod::Get)
    ([] {
        auto& state = services::AppState::instance(); std::lock_guard<std::mutex> lock(state.mutex);
        wvalue data; wvalue units = wvalue::list();
        for (std::size_t index = 0; index < state.units.size(); ++index) units[index] = unitJson(state.units[index]);
        data["patrolUnits"] = std::move(units);
        return successResponse(std::move(data), "Patrol units loaded");
    });

    CROW_ROUTE(app, "/api/patrol-units/<string>/deploy").methods(crow::HTTPMethod::Post)
    ([](const crow::request& request, const std::string& id) {
        rvalue body; std::string checkpoint;
        if (!parseBody(request, body) || !readText(body, "checkpoint", checkpoint)) return errorResponse(400, "INVALID_REQUEST", "A checkpoint name is required");
        auto& state = services::AppState::instance(); std::lock_guard<std::mutex> lock(state.mutex);
        auto* unit = unitById(state, id);
        if (!unit) return errorResponse(404, "UNIT_NOT_FOUND", "Patrol unit was not found");
        if (!state.productionGraph || state.productionGraph->findVertex(checkpoint) < 0) return errorResponse(404, "CHECKPOINT_NOT_FOUND", "Checkpoint was not found");
        if (unit->status == "Offline" || unit->status == "Dispatched") return errorResponse(409, "UNIT_UNAVAILABLE", "Unit cannot be deployed in its current status");
        unit->checkpoint = checkpoint; unit->status = "On Patrol"; unit->assignment = "routine"; unit->updated = nowIso();
        addActivity(state, "unit", unit->id + " deployed to " + checkpoint);
        auto data = unitJson(*unit); services::EventHub::instance().publish("unit.updated", unitJson(*unit));
        return successResponse(std::move(data), "Patrol unit deployed");
    });

    CROW_ROUTE(app, "/api/patrol-units/<string>/release").methods(crow::HTTPMethod::Post)
    ([](const std::string& id) {
        auto& state = services::AppState::instance(); std::lock_guard<std::mutex> lock(state.mutex);
        auto* unit = unitById(state, id);
        if (!unit) return errorResponse(404, "UNIT_NOT_FOUND", "Patrol unit was not found");
        for (auto& alert : state.alerts) if (alert.assignedUnit == id && alert.status == "Dispatched") {
            alert.status = "Queued"; alert.assignedUnit.clear(); alert.route.clear();
            if (auto* record = recordById(state, alert.intruderId)) {
                record->status = "Active"; record->record.status = "Active";
            }
            state.waitingPriority.insert(static_cast<int>(alert.arrival), alert.threat);
        }
        unit->status = "Available"; unit->assignment = "none"; unit->updated = nowIso();
        state.rebuildRecordIndexes();
        if (!state.saveRecords()) return errorResponse(500, "SAVE_FAILED", "Could not persist the released record status");
        addActivity(state, "unit", unit->id + " released to available status");
        auto data = unitJson(*unit); services::EventHub::instance().publish("unit.updated", unitJson(*unit));
        return successResponse(std::move(data), "Patrol unit released");
    });

    CROW_ROUTE(app, "/api/patrol-units/<string>").methods(crow::HTTPMethod::Patch)
    ([](const crow::request& request, const std::string& id) {
        rvalue body; std::string status;
        if (!parseBody(request, body) || !readText(body, "status", status)) return errorResponse(400, "INVALID_REQUEST", "A unit status is required");
        if (status != "Available" && status != "On Patrol" && status != "Offline") return errorResponse(400, "INVALID_STATUS", "Status must be Available, On Patrol, or Offline");
        auto& state = services::AppState::instance(); std::lock_guard<std::mutex> lock(state.mutex);
        auto* unit = unitById(state, id);
        if (!unit) return errorResponse(404, "UNIT_NOT_FOUND", "Patrol unit was not found");
        if (unit->status == "Dispatched") return errorResponse(409, "UNIT_ASSIGNED", "Release the active alert before changing unit status");
        unit->status = status; unit->updated = nowIso();
        auto data = unitJson(*unit); services::EventHub::instance().publish("unit.updated", unitJson(*unit));
        return successResponse(std::move(data), "Patrol unit status updated");
    });

    CROW_ROUTE(app, "/api/activity").methods(crow::HTTPMethod::Get)
    ([](const crow::request& request) {
        auto& state = services::AppState::instance(); std::lock_guard<std::mutex> lock(state.mutex);
        const std::string category = request.url_params.get("category") ? request.url_params.get("category") : "";
        wvalue data; wvalue events = wvalue::list(); std::size_t index = 0;
        for (const auto& entry : state.activity) {
            if (!category.empty() && entry.category != category) continue;
            wvalue item; item["id"] = entry.id; item["category"] = entry.category; item["message"] = entry.message; item["timestamp"] = entry.timestamp;
            events[index++] = std::move(item);
        }
        data["events"] = std::move(events);
        return successResponse(std::move(data), "Activity loaded");
    });

    CROW_ROUTE(app, "/api/checkpoints/<string>").methods(crow::HTTPMethod::Get)
    ([](const std::string& name) {
        auto& state = services::AppState::instance(); std::lock_guard<std::mutex> lock(state.mutex);
        if (!state.productionGraph) return errorResponse(404, "CHECKPOINT_NOT_FOUND", "Checkpoint graph is unavailable");
        const int vertex = state.productionGraph->findVertex(name);
        if (vertex < 0) return errorResponse(404, "CHECKPOINT_NOT_FOUND", "Checkpoint was not found");
        wvalue data; data["id"] = name; data["x"] = state.productionGraph->x(vertex); data["y"] = state.productionGraph->y(vertex);
        data["status"] = "Operational";
        wvalue connected = wvalue::list(); std::size_t count = 0;
        for (const auto& edge : state.productionGraph->exportState().edges) {
            if (edge.from == vertex || edge.to == vertex) {
                wvalue item; const int other = edge.from == vertex ? edge.to : edge.from;
                item["id"] = state.productionGraph->name(other); item["weight"] = edge.weight; item["blocked"] = edge.blocked;
                connected[count++] = std::move(item);
            }
        }
        data["connectedCheckpoints"] = std::move(connected);
        wvalue incidents = wvalue::list(); count = 0;
        for (const auto& alert : state.alerts) if (alert.checkpoint == name && alert.status != "Resolved") incidents[count++] = alertJson(alert);
        data["recentIncidents"] = std::move(incidents);
        wvalue assigned = wvalue::list(); count = 0;
        for (const auto& unit : state.units) if (unit.checkpoint == name && unit.status != "Offline") assigned[count++] = unitJson(unit);
        data["assignedUnits"] = std::move(assigned);
        return successResponse(std::move(data), "Checkpoint details loaded");
    });
}

} // namespace border_api::routes
