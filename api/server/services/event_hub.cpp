#include "api/services/event_hub.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace border_api::services {
namespace {

std::string isoTimestamp()
{
    const auto now = std::chrono::system_clock::now();
    const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm utc{};
#if defined(_WIN32)
    gmtime_s(&utc, &time);
#else
    gmtime_r(&time, &utc);
#endif
    std::ostringstream value;
    value << std::put_time(&utc, "%Y-%m-%dT%H:%M:%S") << '.'
          << std::setfill('0') << std::setw(3) << milliseconds.count() << 'Z';
    return value.str();
}

} // namespace

EventHub& EventHub::instance()
{
    static EventHub hub;
    return hub;
}

std::string EventHub::serialize(const std::string& event, crow::json::wvalue data)
{
    crow::json::wvalue message;
    message["event"] = event;
    message["timestamp"] = isoTimestamp();
    message["data"] = std::move(data);
    return message.dump();
}

void EventHub::add(crow::websocket::connection& connection)
{
    std::lock_guard<std::mutex> lock(mutex_);
    connections_.insert(&connection);
}

void EventHub::remove(crow::websocket::connection& connection)
{
    std::lock_guard<std::mutex> lock(mutex_);
    connections_.erase(&connection);
}

void EventHub::send(crow::websocket::connection& connection, const std::string& event, crow::json::wvalue data)
{
    connection.send_text(serialize(event, std::move(data)));
}

void EventHub::publish(const std::string& event, crow::json::wvalue data)
{
    publishSerialized(serialize(event, std::move(data)));
}

void EventHub::publishSerialized(const std::string& message)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto* connection : connections_) connection->send_text(message);
}

} // namespace border_api::services
