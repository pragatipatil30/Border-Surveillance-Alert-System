#include "api/routes/health_routes.hpp"

#include "api/responses.hpp"

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
        return successResponse(std::move(data), "API is healthy");
    });
}

} // namespace border_api::routes
