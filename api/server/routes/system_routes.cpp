#include "api/routes/system_routes.hpp"

#include "api/responses.hpp"

namespace border_api::routes {

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
        data["phase"] = "4";
        data["implementedEndpoints"] = crow::json::wvalue::list({
            "/api/health", "/api/dashboard", "/api/records", "/api/dsa/*"
        });
        data["notImplementedEndpoints"] = crow::json::wvalue::list({
            "/api/graph", "/api/alerts", "/api/patrol-units"
        });
        return successResponse(std::move(data), "API endpoint status");
    });
}

} // namespace border_api::routes
