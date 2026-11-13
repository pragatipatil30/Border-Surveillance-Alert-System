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
        data["phase"] = "2";
        data["availableEndpoints"] = crow::json::wvalue::list({
            "/api/health", "/api/dashboard", "/api/records"
        });
        return successResponse(std::move(data), "API data endpoints are running");
    });
}

} // namespace border_api::routes
