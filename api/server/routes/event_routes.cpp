#include "api/routes/event_routes.hpp"

#include <optional>
#include <string>

#include "api/services/event_hub.hpp"

namespace border_api::routes {

void registerEventRoutes(HttpApp& app, std::string allowedOrigin)
{
    auto& hub = services::EventHub::instance();
    CROW_WEBSOCKET_ROUTE(app, "/ws/events")
        .max_payload(1024)
        .onaccept([allowedOrigin = std::move(allowedOrigin)](const crow::request& request,
                                                              std::optional<crow::response>& response,
                                                              void**) {
            const std::string origin = request.get_header_value("Origin");
            if (!origin.empty() && origin != allowedOrigin) response = crow::response{403, "WebSocket origin not allowed"};
        })
        .onopen([&hub](crow::websocket::connection& connection) {
            hub.add(connection);
            crow::json::wvalue data;
            data["status"] = "connected";
            hub.send(connection, "system.connected", std::move(data));
        })
        .onmessage([&hub](crow::websocket::connection& connection, const std::string& message, bool isBinary) {
            if (isBinary) {
                connection.close("Text frames only", 1003);
                return;
            }
            const auto payload = crow::json::load(message);
            if (payload.error() || payload.t() != crow::json::type::Object || !payload.has("type") ||
                payload["type"].t() != crow::json::type::String || payload["type"].s() != "ping") {
                crow::json::wvalue data;
                data["code"] = "INVALID_MESSAGE";
                data["message"] = "Send a JSON ping message to keep the event connection active";
                hub.send(connection, "system.error", std::move(data));
                return;
            }
            crow::json::wvalue data;
            data["status"] = "ok";
            if (payload.has("timestamp") && payload["timestamp"].t() == crow::json::type::String)
                data["clientTimestamp"] = payload["timestamp"].s();
            hub.send(connection, "system.pong", std::move(data));
        })
        .onclose([&hub](crow::websocket::connection& connection, const std::string&, std::uint16_t) {
            hub.remove(connection);
        })
        .onerror([&hub](crow::websocket::connection& connection, const std::string&) {
            hub.remove(connection);
        });
}

} // namespace border_api::routes
