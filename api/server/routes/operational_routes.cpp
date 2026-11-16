#include "api/routes/operational_routes.hpp"

#include "api/responses.hpp"

namespace border_api::routes {
namespace {

crow::response unavailable(const char* feature)
{
    return errorResponse(501, "FEATURE_NOT_IMPLEMENTED",
                         std::string(feature) + " is not initialized by the current C++ application");
}

} // namespace

void registerOperationalRoutes(HttpApp& app)
{
    CROW_ROUTE(app, "/api/graph")
        .methods(crow::HTTPMethod::Get, crow::HTTPMethod::Options)
    ([](const crow::request& request) {
        if (request.method == crow::HTTPMethod::Options) return crow::response{204};
        return unavailable("The checkpoint graph");
    });

    CROW_ROUTE(app, "/api/alerts")
        .methods(crow::HTTPMethod::Get, crow::HTTPMethod::Options)
    ([](const crow::request& request) {
        if (request.method == crow::HTTPMethod::Options) return crow::response{204};
        return unavailable("The alert queue");
    });

    CROW_ROUTE(app, "/api/patrol-units")
        .methods(crow::HTTPMethod::Get, crow::HTTPMethod::Options)
    ([](const crow::request& request) {
        if (request.method == crow::HTTPMethod::Options) return crow::response{204};
        return unavailable("Patrol unit data");
    });
}

} // namespace border_api::routes
