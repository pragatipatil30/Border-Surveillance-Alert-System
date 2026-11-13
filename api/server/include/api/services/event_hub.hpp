#pragma once

#include <mutex>
#include <string>
#include <unordered_set>

#include <crow.h>

namespace border_api::services {

class EventHub {
public:
    static EventHub& instance();
    void add(crow::websocket::connection& connection);
    void remove(crow::websocket::connection& connection);
    void send(crow::websocket::connection& connection, const std::string& event, crow::json::wvalue data);
    void publish(const std::string& event, crow::json::wvalue data);

private:
    static std::string serialize(const std::string& event, crow::json::wvalue data);
    void publishSerialized(const std::string& message);

    std::mutex mutex_;
    std::unordered_set<crow::websocket::connection*> connections_;
};

} // namespace border_api::services
