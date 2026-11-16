#include "api/routes/health_routes.hpp"

#include "api/responses.hpp"
#include "api/services/app_state.hpp"
#include "api/services/event_hub.hpp"

#include <chrono>

namespace border_api::routes {

void registerHealthRoutes(HttpApp& app)
{
    CROW_ROUTE(app, "/api/health")
        .methods(crow::HTTPMethod::Get, crow::HTTPMethod::Options)
    ([](const crow::request& request) {
        if (request.method == crow::HTTPMethod::Options)
        {
            return crow::response{204};
        }

        crow::json::wvalue data;
        data["status"] = "ok";
        data["service"] = "border-command-center-api";
        auto& state = services::AppState::instance();
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            data["dataset"] = state.demoDataset ? "demo" : "real";
            data["recordCount"] = state.records.size();
            data["alertCount"] = state.alerts.size();
            data["unitCount"] = state.units.size();
        }
        data["uptimeSeconds"] = static_cast<std::int64_t>(std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - state.startedClock).count());
        data["websocketUp"] = services::EventHub::instance().hasConnections();
        return successResponse(std::move(data), "API is healthy");
    });
}

} // namespace border_api::routes
