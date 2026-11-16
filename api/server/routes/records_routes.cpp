#include "api/routes/records_routes.hpp"

#include <algorithm>
#include <cctype>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "api/responses.hpp"
#include "api/services/app_state.hpp"
#include "api/services/event_hub.hpp"
#include "api/services/intruder_repository.hpp"
#include "Sorting.h"

namespace border_api::routes {
namespace {
using crow::json::rvalue;
using crow::json::wvalue;

wvalue recordJson(const services::RecordEntry& entry)
{
    wvalue value;
    value["id"] = std::to_string(entry.record.id);
    value["name"] = entry.record.name;
    value["location"] = entry.record.location;
    value["threatLevel"] = entry.record.threatlevel;
    value["status"] = entry.status;
    return value;
}

services::RecordEntry* findRecord(services::AppState& state, int id)
{
    const auto found = std::find_if(state.records.begin(), state.records.end(),
        [id](const auto& entry) { return entry.record.id == id; });
    return found == state.records.end() ? nullptr : &*found;
}

bool readInt(const rvalue& body, const char* key, int& value)
{
    if (!body.has(key) || body[key].t() != crow::json::type::Number) return false;
    const auto raw = body[key].i();
    if (raw < std::numeric_limits<int>::min() || raw > std::numeric_limits<int>::max()) return false;
    value = static_cast<int>(raw);
    return true;
}

bool readText(const rvalue& body, const char* key, std::string& value, std::size_t limit = 120)
{
    if (!body.has(key) || body[key].t() != crow::json::type::String) return false;
    value = body[key].s();
    return !value.empty() && value.size() <= limit;
}

bool parseBody(const crow::request& request, rvalue& body)
{
    body = crow::json::load(request.body);
    return !body.error() && body.t() == crow::json::type::Object;
}

bool persist(const services::AppState& state)
{
    return state.saveRecords();
}

void upsert(services::AppState& state, const services::RecordEntry& entry)
{
    const auto found = std::find_if(state.records.begin(), state.records.end(),
        [&entry](const auto& current) { return current.record.id == entry.record.id; });
    if (found == state.records.end()) state.records.push_back(entry);
    else *found = entry;
    state.rebuildRecordIndexes();
}

void erase(services::AppState& state, int id)
{
    state.records.erase(std::remove_if(state.records.begin(), state.records.end(),
        [id](const auto& entry) { return entry.record.id == id; }), state.records.end());
    state.rebuildRecordIndexes();
}

bool validStatus(const std::string& status)
{
    return status == "Active" || status == "Dispatched" || status == "Resolved";
}

std::vector<Intruder> recordCandidates(services::AppState& state, const std::string& q)
{
    std::vector<Intruder> result;
    if (q.empty()) {
        for (const auto& entry : state.records) result.push_back(entry.record);
        return result;
    }
    bool digits = std::all_of(q.begin(), q.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
    if (digits) {
        try {
            const int id = std::stoi(q);
            if (const auto* found = state.recordHash->search(id)) result.push_back(*found);
        } catch (...) { }
        return result;
    }
    state.nameTree->prefixSearch(q, result);
    if (!result.empty()) return result;
    std::string needle = q;
    std::transform(needle.begin(), needle.end(), needle.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    for (const auto& entry : state.records) {
        std::string text = entry.record.name + " " + entry.record.location;
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (text.find(needle) != std::string::npos) result.push_back(entry.record);
    }
    return result;
}

crow::response mutateRecord(const std::string& action, const int id, const rvalue* body)
{
    auto& state = services::AppState::instance();
    std::lock_guard<std::mutex> lock(state.mutex);
    const auto beforeRecords = state.records;
    services::RecordEntry before{};
    const auto existing = std::find_if(state.records.begin(), state.records.end(),
        [id](const auto& entry) { return entry.record.id == id; });
    const bool hadRecord = existing != state.records.end();
    if (hadRecord) before = *existing;
    if (action == "update" && !hadRecord) return errorResponse(404, "RECORD_NOT_FOUND", "Intruder record was not found");

    if (action == "delete") {
        if (!hadRecord) return errorResponse(404, "RECORD_NOT_FOUND", "Intruder record was not found");
        erase(state, id);
    } else {
        if (!body) return errorResponse(400, "INVALID_REQUEST", "Record fields are required");
        int parsedId = id, threat = 0;
        std::string name, location, status = "Active";
        if ((!hadRecord && !readInt(*body, "id", parsedId)) || parsedId <= 0 ||
            !readText(*body, "name", name) || !readText(*body, "location", location) ||
            !readInt(*body, "threatLevel", threat) || threat < 1 || threat > 5)
            return errorResponse(400, "INVALID_REQUEST", "Record needs a positive id, name, location and threat level from 1 to 5");
        if (parsedId != id) return errorResponse(400, "ID_MISMATCH", "Record id cannot change during update");
        if (body->has("status") && (!readText(*body, "status", status) || !validStatus(status)))
            return errorResponse(400, "INVALID_STATUS", "Status must be Active, Dispatched, or Resolved");
        if (action == "create" && hadRecord) return errorResponse(409, "DUPLICATE_ID", "An intruder with this id already exists");
        services::RecordEntry after{Intruder(id, name, location, threat, status), status};
        upsert(state, after);
        state.undoHistory.push_back({action, before, after});
        state.redoHistory.clear();
    }
    if (action == "delete") {
        state.undoHistory.push_back({"delete", before, {}});
        state.redoHistory.clear();
    }
    if (!persist(state)) {
        state.records = beforeRecords;
        state.rebuildRecordIndexes();
        state.undoHistory.pop_back();
        return errorResponse(500, "SAVE_FAILED", "Records were not saved; no change was kept");
    }
    crow::json::wvalue event;
    event["id"] = id;
    services::EventHub::instance().publish(action == "create" ? "record.created" : action == "update" ? "record.updated" : "record.deleted", std::move(event));
    return successResponse(action == "delete" ? wvalue{} : recordJson(*findRecord(state, id)), "Record " + action + "d");
}

} // namespace

void registerRecordsRoutes(HttpApp& app)
{
    CROW_ROUTE(app, "/api/records").methods(crow::HTTPMethod::Get, crow::HTTPMethod::Post)
    ([](const crow::request& request) -> crow::response {
        auto& state = services::AppState::instance();
        if (request.method == crow::HTTPMethod::Post) {
            rvalue body;
            if (!parseBody(request, body)) return errorResponse(400, "INVALID_REQUEST", "A JSON record is required");
            int id = 0;
            if (!readInt(body, "id", id)) return errorResponse(400, "INVALID_REQUEST", "A positive numeric id is required");
            return mutateRecord("create", id, &body);
        }
        std::lock_guard<std::mutex> lock(state.mutex);
        if (request.url_params.get("rule")) return errorResponse(501, "RULE_FILTER_UNAVAILABLE", "Expression rules are not exposed until the C++ rule evaluator is connected");
        const std::string q = request.url_params.get("q") ? request.url_params.get("q") : "";
        const std::string location = request.url_params.get("location") ? request.url_params.get("location") : "";
        const std::string status = request.url_params.get("status") ? request.url_params.get("status") : "";
        const std::string threatText = request.url_params.get("threat") ? request.url_params.get("threat") : "";
        const std::string sort = request.url_params.get("sort") ? request.url_params.get("sort") : "id";
        const std::string algorithm = request.url_params.get("algorithm") ? request.url_params.get("algorithm") : "merge";
        int threat = 0;
        try { if (!threatText.empty() && threatText != "HIGH" && threatText != "MEDIUM" && threatText != "LOW") threat = std::stoi(threatText); }
        catch (...) { return errorResponse(400, "INVALID_FILTER", "Threat filter must be a category or level from 1 to 5"); }
        if (threat < 0 || threat > 5) return errorResponse(400, "INVALID_FILTER", "Threat filter must be a category or level from 1 to 5");
        std::vector<Intruder> candidates = recordCandidates(state, q);
        std::vector<services::RecordEntry> selected;
        for (const auto& record : candidates) {
            const auto entry = std::find_if(state.records.begin(), state.records.end(), [&record](const auto& value) { return value.record.id == record.id; });
            if (entry == state.records.end()) continue;
            if (!location.empty() && entry->record.location != location) continue;
            if (threat != 0 && entry->record.threatlevel != threat) continue;
            if (threatText == "HIGH" && entry->record.threatlevel < 4) continue;
            if (threatText == "MEDIUM" && entry->record.threatlevel != 3) continue;
            if (threatText == "LOW" && entry->record.threatlevel > 2) continue;
            if (!status.empty() && entry->status != status) continue;
            selected.push_back(*entry);
        }
        if (sort != "id" && sort != "threat" && sort != "name" && sort != "location") return errorResponse(400, "INVALID_SORT", "Sort must be id, threat, name, or location");
        if (algorithm != "bubble" && algorithm != "insertion" && algorithm != "merge" && algorithm != "quick") return errorResponse(400, "INVALID_ALGORITHM", "Algorithm must be bubble, insertion, merge, or quick");
        std::vector<Intruder> sorted;
        for (const auto& entry : selected) sorted.push_back(entry.record);
        SortCriteria criteria = sort == "threat" ? BY_THREAT_LEVEL : sort == "name" ? BY_NAME : sort == "location" ? BY_LOCATION : BY_ID;
        SortStats stats;
        if (!sorted.empty()) {
            if (algorithm == "bubble") bubbleSortIntruders(sorted.data(), static_cast<int>(sorted.size()), criteria, stats);
            else if (algorithm == "insertion") insertionSortIntruders(sorted.data(), static_cast<int>(sorted.size()), criteria, stats);
            else if (algorithm == "quick") quickSortIntruders(sorted.data(), 0, static_cast<int>(sorted.size()) - 1, criteria, stats);
            else mergeSortIntruders(sorted.data(), 0, static_cast<int>(sorted.size()) - 1, criteria, stats);
        }
        wvalue data; wvalue items = wvalue::list();
        for (std::size_t index = 0; index < sorted.size(); ++index) {
            const auto entry = std::find_if(selected.begin(), selected.end(), [&sorted, index](const auto& value) { return value.record.id == sorted[index].id; });
            items[index] = recordJson(*entry);
        }
        data["records"] = std::move(items); data["count"] = sorted.size();
        wvalue sortResult; sortResult["algorithm"] = algorithm; sortResult["criteria"] = sort;
        sortResult["comparisons"] = stats.comparisons; sortResult["passes"] = stats.passes;
        data["sort"] = std::move(sortResult);
        return successResponse(std::move(data), "Records loaded from C++ indexes");
    });

    CROW_ROUTE(app, "/api/records/search").methods(crow::HTTPMethod::Get)
    ([](const crow::request& request) {
        const std::string prefix = request.url_params.get("prefix") ? request.url_params.get("prefix") : "";
        auto& state = services::AppState::instance(); std::lock_guard<std::mutex> lock(state.mutex);
        std::vector<Intruder> matches; state.nameTree->prefixSearch(prefix, matches, 20);
        wvalue data; wvalue items = wvalue::list();
        for (std::size_t index = 0; index < matches.size(); ++index) {
            if (auto* entry = findRecord(state, matches[index].id)) items[index] = recordJson(*entry);
        }
        data["records"] = std::move(items); data["count"] = matches.size(); data["prefix"] = prefix;
        return successResponse(std::move(data), "Name prefix search completed by the C++ BST");
    });

    CROW_ROUTE(app, "/api/records/undo-state").methods(crow::HTTPMethod::Get)
    ([] {
        auto& state = services::AppState::instance(); std::lock_guard<std::mutex> lock(state.mutex);
        wvalue data; data["canUndo"] = !state.undoHistory.empty(); data["canRedo"] = !state.redoHistory.empty();
        data["undoCount"] = state.undoHistory.size(); data["redoCount"] = state.redoHistory.size();
        return successResponse(std::move(data), "Undo state loaded");
    });

    CROW_ROUTE(app, "/api/records/undo").methods(crow::HTTPMethod::Post)
    ([] {
        auto& state = services::AppState::instance(); std::lock_guard<std::mutex> lock(state.mutex);
        if (state.undoHistory.empty()) return errorResponse(409, "UNDO_EMPTY", "There is no record operation to undo");
        const auto history = state.undoHistory.back(); state.undoHistory.pop_back();
        const auto before = state.records;
        if (history.action == "create") erase(state, history.after.record.id);
        else if (history.action == "update" || history.action == "delete") upsert(state, history.before);
        state.redoHistory.push_back(history);
        if (!persist(state)) { state.records = before; state.rebuildRecordIndexes(); state.redoHistory.pop_back(); state.undoHistory.push_back(history); return errorResponse(500, "SAVE_FAILED", "Undo could not be saved"); }
        wvalue data; data["records"] = state.records.size(); data["canUndo"] = !state.undoHistory.empty(); data["canRedo"] = true;
        services::EventHub::instance().publish("record.updated", crow::json::wvalue{});
        return successResponse(std::move(data), "Last record operation undone");
    });

    CROW_ROUTE(app, "/api/records/redo").methods(crow::HTTPMethod::Post)
    ([] {
        auto& state = services::AppState::instance(); std::lock_guard<std::mutex> lock(state.mutex);
        if (state.redoHistory.empty()) return errorResponse(409, "REDO_EMPTY", "There is no record operation to redo");
        const auto history = state.redoHistory.back(); state.redoHistory.pop_back();
        const auto before = state.records;
        if (history.action == "create" || history.action == "update") upsert(state, history.after);
        else if (history.action == "delete") erase(state, history.before.record.id);
        state.undoHistory.push_back(history);
        if (!persist(state)) { state.records = before; state.rebuildRecordIndexes(); state.undoHistory.pop_back(); state.redoHistory.push_back(history); return errorResponse(500, "SAVE_FAILED", "Redo could not be saved"); }
        wvalue data; data["records"] = state.records.size(); data["canUndo"] = true; data["canRedo"] = !state.redoHistory.empty();
        services::EventHub::instance().publish("record.updated", crow::json::wvalue{});
        return successResponse(std::move(data), "Last record operation redone");
    });

    CROW_ROUTE(app, "/api/records/<int>").methods(crow::HTTPMethod::Get, crow::HTTPMethod::Put, crow::HTTPMethod::Delete)
    ([](const crow::request& request, int id) -> crow::response {
        auto& state = services::AppState::instance();
        if (request.method == crow::HTTPMethod::Get) {
            std::lock_guard<std::mutex> lock(state.mutex);
            const Intruder* indexed = state.recordHash->search(id);
            if (!indexed) return errorResponse(404, "RECORD_NOT_FOUND", "Hash table has no record with this id");
            if (auto* entry = findRecord(state, id)) return successResponse(recordJson(*entry), "Record found through the C++ hash table");
            return errorResponse(404, "RECORD_NOT_FOUND", "Record index is inconsistent");
        }
        if (request.method == crow::HTTPMethod::Delete) return mutateRecord("delete", id, nullptr);
        rvalue body;
        if (!parseBody(request, body)) return errorResponse(400, "INVALID_REQUEST", "A JSON record is required");
        return mutateRecord("update", id, &body);
    });
}

} // namespace border_api::routes
