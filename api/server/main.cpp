#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

#include <crow.h>
#include <crow/middlewares/cors.h>

#include "api/http_app.hpp"
#include "api/responses.hpp"
#include "api/routes/health_routes.hpp"
#include "api/routes/system_routes.hpp"
#include "api/routes/dashboard_routes.hpp"
#include "api/routes/records_routes.hpp"
#include "api/routes/operational_routes.hpp"

namespace {

int readPort()
{
    const char* configuredPort = std::getenv("API_PORT");
    if (configuredPort == nullptr || *configuredPort == '\0') return 18080;

    try
    {
        std::size_t parsed = 0;
        const int port = std::stoi(configuredPort, &parsed);
        if (parsed != std::string(configuredPort).size() || port < 1 || port > 65535)
            throw std::invalid_argument("port out of range");
        return port;
    }
    catch (...)
    {
        throw std::runtime_error("API_PORT must be an integer between 1 and 65535");
    }
}

std::string readCorsOrigin()
{
    const char* configuredOrigin = std::getenv("API_CORS_ORIGIN");
    return configuredOrigin != nullptr && *configuredOrigin != '\0'
        ? configuredOrigin
        : "http://localhost:5173";
}

} // namespace

int main()
{
    try
    {
        border_api::HttpApp app;
        const std::string allowedOrigin = readCorsOrigin();
        const int port = readPort();

        app.get_middleware<crow::CORSHandler>()
            .global()
            .origin(allowedOrigin)
            .methods(crow::HTTPMethod::Get, crow::HTTPMethod::Post,
                     crow::HTTPMethod::Put, crow::HTTPMethod::Delete,
                     crow::HTTPMethod::Options)
            .headers("Content-Type", "Authorization")
            .max_age(600);

        border_api::routes::registerSystemRoutes(app);
        border_api::routes::registerHealthRoutes(app);
        border_api::routes::registerDashboardRoutes(app);
        border_api::routes::registerRecordsRoutes(app);
        border_api::routes::registerOperationalRoutes(app);

        CROW_ROUTE(app, "/api/<path>")
            .methods(crow::HTTPMethod::Get, crow::HTTPMethod::Post,
                     crow::HTTPMethod::Put, crow::HTTPMethod::Delete,
                     crow::HTTPMethod::Options)
        ([](const crow::request&, const std::string&) {
            return border_api::errorResponse(404, "NOT_FOUND", "API route not found");
        });

        app.bindaddr("127.0.0.1").port(static_cast<std::uint16_t>(port)).multithreaded().run();
    }
    catch (const std::exception& error)
    {
        std::cerr << "API server could not start: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
