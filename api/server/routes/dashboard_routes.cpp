#include "api/routes/dashboard_routes.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include "api/responses.hpp"
#include "api/services/app_state.hpp"

namespace border_api::routes {
namespace {
using crow::json::wvalue;

std::string threatName(int threat) { return threat >= 4 ? "HIGH" : threat == 3 ? "MEDIUM" : "LOW"; }

wvalue alertJson(const services::AlertEntry& alert)
{
    wvalue item;
    item["id"] = alert.id;
    item["title"] = "Intruder sighting · " + std::to_string(alert.intruderId);
    item["checkpoint"] = alert.checkpoint;
    item["time"] = "Demo event";
    item["threat"] = threatName(alert.threat);
    item["status"] = alert.status;
    item["assignedUnit"] = alert.assignedUnit;
    return item;
}

std::vector<const services::AlertEntry*> orderedQueued(services::AppState& state)
{
    std::vector<const services::AlertEntry*> ordered;
    auto queue = state.waitingPriority;
    dsa::PriorityItem item{};
    while (queue.extract(item)) {
        const auto found = std::find_if(state.alerts.begin(), state.alerts.end(), [&item](const auto& alert) {
            return alert.status == "Queued" && alert.arrival == static_cast<std::size_t>(item.value);
        });
        if (found != state.alerts.end()) ordered.push_back(&*found);
    }
    if (state.alertPolicy == "FCFS") {
        std::stable_sort(ordered.begin(), ordered.end(), [](const auto* a, const auto* b) { return a->arrival < b->arrival; });
    }
    return ordered;
}

} // namespace

void registerDashboardRoutes(HttpApp& app)
{
    CROW_ROUTE(app, "/api/dashboard").methods(crow::HTTPMethod::Get, crow::HTTPMethod::Options)
    ([](const crow::request& request) {
        if (request.method == crow::HTTPMethod::Options) return crow::response{204};
        auto& state = services::AppState::instance();
        std::lock_guard<std::mutex> lock(state.mutex);

        std::array<std::size_t, 3> counts{};
        std::size_t highThreat = 0;
        std::size_t activeAlerts = 0;
        std::size_t availableUnits = 0;
        for (const auto& record : state.records) {
            const int threat = record.record.threatlevel;
            if (threat <= 2) ++counts[0]; else if (threat == 3) ++counts[1]; else ++counts[2];
            if (threat >= 4) ++highThreat;
        }
        for (const auto& alert : state.alerts) if (alert.status == "Queued" || alert.status == "Dispatched") ++activeAlerts;
        for (const auto& unit : state.units) if (unit.status == "Available") ++availableUnits;
        const std::size_t total = state.records.size();
        const auto percent = [total](std::size_t count) { return total == 0 ? 0 : static_cast<int>((count * 100 + total / 2) / total); };

        wvalue data;
        wvalue metrics = wvalue::list();
        const auto metric = [](const char* label, std::size_t value, const char* icon) {
            wvalue item; item["label"] = label; item["value"] = std::to_string(value); item["icon"] = icon; return item;
        };
        metrics[0] = metric("Total Intruders", total, "users");
        metrics[1] = metric("High Threat", highThreat, "shield");
        metrics[2] = metric("Active Alerts", activeAlerts, "bell");
        metrics[3] = metric("Available Units", availableUnits, "radio");
        data["metrics"] = std::move(metrics);

        wvalue distribution = wvalue::list();
        const char* names[] = {"Low", "Medium", "High"};
        const char* colors[] = {"#67B98D", "#E9B85F", "#DB716B"};
        for (std::size_t i = 0; i < counts.size(); ++i) {
            wvalue item; item["name"] = names[i]; item["value"] = percent(counts[i]); item["count"] = counts[i]; item["color"] = colors[i];
            distribution[i] = std::move(item);
        }
        data["threatDistribution"] = std::move(distribution);

        wvalue alerts = wvalue::list();
        const auto queued = orderedQueued(state);
        for (std::size_t i = 0; i < std::min<std::size_t>(5, queued.size()); ++i) alerts[i] = alertJson(*queued[i]);
        data["alerts"] = std::move(alerts);
        wvalue activities = wvalue::list();
        const std::size_t activityCount = std::min<std::size_t>(5, state.activity.size());
        for (std::size_t i = 0; i < activityCount; ++i) {
            const auto& event = state.activity[i];
            wvalue item; item["id"] = event.id; item["name"] = event.message; item["checkpoint"] = "";
            item["threat"] = "LOW"; item["status"] = event.category; item["time"] = event.timestamp;
            activities[i] = std::move(item);
        }
        data["activities"] = std::move(activities);

        wvalue units = wvalue::list();
        for (std::size_t i = 0; i < state.units.size(); ++i) {
            const auto& unit = state.units[i];
            wvalue item; item["id"] = unit.id; item["checkpoint"] = unit.checkpoint; item["status"] = unit.status;
            item["assignment"] = unit.assignment; item["updated"] = unit.updated; units[i] = std::move(item);
        }
        data["patrolUnits"] = std::move(units);

        wvalue checkpoints = wvalue::list();
        if (state.productionGraph) {
            const auto graph = state.productionGraph->exportState();
            for (std::size_t i = 0; i < graph.vertices.size(); ++i) {
                const auto& vertex = graph.vertices[i];
                bool active = false;
                for (const auto& alert : state.alerts) if (alert.checkpoint == vertex.name && alert.status != "Resolved") active = true;
                wvalue item; item["id"] = vertex.name; item["x"] = vertex.x; item["y"] = vertex.y;
                item["state"] = active ? "alert" : "normal"; checkpoints[i] = std::move(item);
            }
            wvalue routes = wvalue::list();
            for (std::size_t i = 0; i < graph.edges.size(); ++i) {
                const auto& edge = graph.edges[i];
                wvalue item; item["from"] = graph.vertices[static_cast<std::size_t>(edge.from)].name;
                item["to"] = graph.vertices[static_cast<std::size_t>(edge.to)].name;
                item["weight"] = edge.weight; item["blocked"] = edge.blocked; routes[i] = std::move(item);
            }
            data["routes"] = std::move(routes);
        } else data["routes"] = wvalue::list();
        data["checkpoints"] = std::move(checkpoints);
        data["dataset"] = state.demoDataset ? "demo" : "real";
        return successResponse(std::move(data), "Dashboard loaded from shared application state");
    });
}

} // namespace border_api::routes
