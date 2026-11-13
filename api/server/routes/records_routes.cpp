#include "api/routes/records_routes.hpp"

#include "api/responses.hpp"
#include "api/services/intruder_repository.hpp"

namespace border_api::routes {

void registerRecordsRoutes(HttpApp& app)
{
    CROW_ROUTE(app, "/api/records")
        .methods(crow::HTTPMethod::Get, crow::HTTPMethod::Options)
    ([](const crow::request& request) {
        if (request.method == crow::HTTPMethod::Options) return crow::response{204};

        const auto result = services::loadIntruders();
        if (!result.ok) {
            return errorResponse(503, "DATA_UNAVAILABLE", "Intruder records could not be loaded");
        }

        crow::json::wvalue::list records;
        records.reserve(result.records.size());
        for (const auto& record : result.records) {
            crow::json::wvalue item;
            item["id"] = std::to_string(record.id);
            item["name"] = record.name;
            item["location"] = record.location;
            item["threatLevel"] = record.threatlevel;
            records.push_back(std::move(item));
        }

        crow::json::wvalue data;
        data["records"] = std::move(records);
        data["count"] = result.records.size();
        return successResponse(std::move(data), "Intruder records loaded");
    });
}

} // namespace border_api::routes
