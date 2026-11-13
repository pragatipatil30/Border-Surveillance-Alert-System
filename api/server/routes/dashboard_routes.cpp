#include "api/routes/dashboard_routes.hpp"

#include <array>

#include "api/responses.hpp"
#include "api/services/intruder_repository.hpp"

namespace border_api::routes {

void registerDashboardRoutes(HttpApp& app)
{
    CROW_ROUTE(app, "/api/dashboard")
        .methods(crow::HTTPMethod::Get, crow::HTTPMethod::Options)
    ([](const crow::request& request) {
        if (request.method == crow::HTTPMethod::Options) return crow::response{204};

        const auto result = services::loadIntruders();
        if (!result.ok) {
            return errorResponse(503, "DATA_UNAVAILABLE", "Dashboard records could not be loaded");
        }

        std::array<std::size_t, 5> counts{};
        for (const auto& record : result.records) {
            ++counts[static_cast<std::size_t>(record.threatlevel - 1)];
        }

        crow::json::wvalue distribution = crow::json::wvalue::list();
        for (std::size_t i = 0; i < counts.size(); ++i) distribution[i] = counts[i];

        crow::json::wvalue data;
        data["totalIntruders"] = result.records.size();
        data["highThreatIntruders"] = counts[3] + counts[4];
        data["threatLevelCounts"] = std::move(distribution);
        data["activeAlerts"] = nullptr;
        data["availablePatrolUnits"] = nullptr;
        return successResponse(std::move(data), "Dashboard summary loaded from intruder records");
    });
}

} // namespace border_api::routes
