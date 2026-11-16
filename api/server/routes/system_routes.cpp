#include "api/routes/system_routes.hpp"

#include "api/responses.hpp"
#include "api/services/app_state.hpp"
#include "api/services/event_hub.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <random>
#include <sstream>

namespace border_api::routes {
namespace {
std::string currentTimestamp()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t raw = std::chrono::system_clock::to_time_t(now);
    std::tm utc{};
#if defined(_WIN32)
    gmtime_s(&utc, &raw);
#else
    gmtime_r(&raw, &utc);
#endif
    std::ostringstream output;
    output << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return output.str();
}
}

void registerSystemRoutes(HttpApp& app)
{
    CROW_ROUTE(app, "/api")
        .methods(crow::HTTPMethod::Get, crow::HTTPMethod::Options)
    ([](const crow::request& request) {
        if (request.method == crow::HTTPMethod::Options)
        {
            return crow::response{204};
        }

        crow::json::wvalue data;
        data["service"] = "border-command-center-api";
        data["phase"] = "live-demo";
        data["implementedEndpoints"] = crow::json::wvalue::list({
            "/api/health", "/api/dashboard", "/api/records", "/api/demo/reset",
            "/api/demo/seed", "/api/dsa/*", "/ws/events"
        });
        data["notImplementedEndpoints"] = crow::json::wvalue::list({
            "/api/auth/login", "/api/auth/logout", "/api/dsa/bst/*",
            "/api/dsa/linked-lists/*", "/api/dsa/sorting/*", "/api/dsa/searching/*",
            "/api/dsa/sparse-matrix/*", "/api/dsa/expression/*"
        });
        return successResponse(std::move(data), "API endpoint status");
    });

    CROW_ROUTE(app, "/api/demo/reset").methods(crow::HTTPMethod::Post)
    ([] {
        auto& state = services::AppState::instance();
        crow::json::wvalue data;
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.resetDemo();
            data["dataset"] = "demo";
            data["records"] = state.records.size();
            data["alerts"] = state.alerts.size();
            data["units"] = state.units.size();
        }
        crow::json::wvalue eventData;
        eventData["dataset"] = "demo";
        eventData["records"] = 15;
        eventData["alerts"] = 12;
        eventData["units"] = 8;
        services::EventHub::instance().publish("demo.reset", std::move(eventData));
        return successResponse(std::move(data), "Demo dataset reset");
    });

    CROW_ROUTE(app, "/api/demo/seed").methods(crow::HTTPMethod::Post)
    ([] {
        auto& state = services::AppState::instance();
        crow::json::wvalue data;
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            if (!state.records.empty()) return errorResponse(409, "DATASET_NOT_EMPTY", "Seed only runs when the application state is empty");
            state.resetDemo();
            data["dataset"] = "demo";
            data["records"] = state.records.size();
            data["alerts"] = state.alerts.size();
            data["units"] = state.units.size();
        }
        crow::json::wvalue eventData;
        eventData["dataset"] = "demo";
        eventData["records"] = 15;
        eventData["alerts"] = 12;
        eventData["units"] = 8;
        services::EventHub::instance().publish("demo.reset", std::move(eventData));
        return successResponse(std::move(data), "Demo dataset seeded");
    });

    CROW_ROUTE(app, "/api/demo/event").methods(crow::HTTPMethod::Post)
    ([] {
        auto& state = services::AppState::instance();
        std::lock_guard<std::mutex> lock(state.mutex);
        if (!state.demoDataset) return errorResponse(409, "DEMO_MODE_REQUIRED", "Test events are available only for the demo dataset");
        static const std::array<const char*, 8> names{{
            "Nisha Verma", "Imran Sheikh", "Pooja Reddy", "Sameer Gill",
            "Aditi Rao", "Farhan Ali", "Ritu Sharma", "Unknown Subject",
        }};
        static const std::array<const char*, 10> posts{{
            "CP-01", "CP-02", "CP-03", "CP-04", "CP-05", "CP-06", "CP-07", "CP-08", "CP-09", "CP-10",
        }};
        std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<std::size_t> namePick(0, names.size() - 1);
        std::uniform_int_distribution<std::size_t> postPick(0, posts.size() - 1);
        std::uniform_int_distribution<int> threatPick(3, 5);
        int id = 1000;
        for (const auto& item : state.records) id = std::max(id, item.record.id);
        ++id;
        const std::string name = names[namePick(generator)];
        const std::string location = posts[postPick(generator)];
        const int threat = threatPick(generator);
        services::RecordEntry entry{Intruder(id, name, location, threat, "Active"), "Active"};
        state.records.push_back(entry);
        state.rebuildRecordIndexes();
        if (!state.saveRecords()) {
            state.records.pop_back(); state.rebuildRecordIndexes();
            return errorResponse(500, "SAVE_FAILED", "Test event could not be saved");
        }
        const std::string alertId = "A-" + std::to_string(101 + state.nextAlertSequence);
        services::AlertEntry alert{alertId, id, threat, location, "Queued", "", {}, state.nextAlertSequence++};
        state.incomingAlerts.enqueue(static_cast<int>(alert.arrival));
        state.waitingPriority.insert(static_cast<int>(alert.arrival), threat);
        state.alerts.push_back(alert);
        const std::string timestamp = currentTimestamp();
        state.activity.insert(state.activity.begin(), {"EV-TEST-" + std::to_string(id), "intruder", "New test event: " + name + " at " + location, timestamp});
        crow::json::wvalue recordData; recordData["id"] = std::to_string(id); recordData["name"] = name;
        recordData["location"] = location; recordData["threatLevel"] = threat; recordData["status"] = "Active";
        crow::json::wvalue alertData; alertData["id"] = alertId; alertData["intruderId"] = id;
        alertData["checkpoint"] = location; alertData["threatLevel"] = threat; alertData["status"] = "Queued";
        services::EventHub::instance().publish("record.created", std::move(recordData));
        services::EventHub::instance().publish("alert.created", std::move(alertData));
        crow::json::wvalue eventData; eventData["id"] = id; eventData["message"] = "New test event created";
        services::EventHub::instance().publish("activity.added", std::move(eventData));
        crow::json::wvalue data; data["record"]["id"] = std::to_string(id); data["record"]["name"] = name;
        data["record"]["location"] = location; data["record"]["threatLevel"] = threat;
        data["alert"]["id"] = alertId; data["alert"]["status"] = "Queued";
        return successResponse(std::move(data), "Test intruder event generated");
    });
}

} // namespace border_api::routes
