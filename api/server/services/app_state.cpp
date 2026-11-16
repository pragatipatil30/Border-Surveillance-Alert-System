#include "api/services/app_state.hpp"

#include <array>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <utility>

#include "api/services/intruder_repository.hpp"

namespace border_api::services {
namespace {

bool demoEnabled()
{
    const char* value = std::getenv("API_DEMO");
    return value == nullptr || std::string(value) != "0";
}

std::string isoAgo(int minutes)
{
    const auto point = std::chrono::system_clock::now() - std::chrono::minutes(minutes);
    const std::time_t raw = std::chrono::system_clock::to_time_t(point);
    std::tm utc{};
#if defined(_WIN32)
    gmtime_s(&utc, &raw);
#else
    gmtime_r(&raw, &utc);
#endif
    std::ostringstream out;
    out << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return out.str();
}

Intruder intruder(int id, const char* name, const char* place, int threat)
{
    Intruder item{};
    item.id = id;
    item.name = name;
    item.location = place;
    item.threatlevel = threat;
    return item;
}

std::unique_ptr<Graph> makeProductionGraph()
{
    auto graph = std::make_unique<Graph>(false, 10);
    const std::pair<const char*, std::pair<double, double>> posts[] = {
        {"CP-01", {9, 55}}, {"CP-02", {20, 34}}, {"CP-03", {29, 66}},
        {"CP-04", {39, 43}}, {"CP-05", {49, 24}}, {"CP-06", {57, 61}},
        {"CP-07", {69, 37}}, {"CP-08", {78, 67}}, {"CP-09", {87, 29}},
        {"CP-10", {94, 53}},
    };
    for (const auto& post : posts) graph->addVertex(post.first, post.second.first, post.second.second);
    const int edges[][3] = {
        {0,1,12},{0,2,9},{1,2,7},{1,3,11},{2,3,8},{2,5,13},{3,4,10},{3,5,6},
        {4,6,12},{5,6,7},{5,7,14},{6,8,9},{7,8,8},{7,9,11},{8,9,6},
    };
    for (const auto& edge : edges) graph->addEdge(edge[0], edge[1], edge[2]);
    return graph;
}

} // namespace

AppState& AppState::instance()
{
    static AppState state;
    return state;
}

AppState::AppState()
    : recordList(std::make_unique<SinglyLinkedList>()),
      recordHash(std::make_unique<HashChaining>(10)),
      nameTree(std::make_unique<BST>()),
      startedAt(isoAgo(0))
{
    demoDataset = demoEnabled();
}

void AppState::initialize()
{
    std::lock_guard<std::mutex> lock(mutex);
    if (demoDataset) {
        resetDemo();
        return;
    }
    const auto loaded = loadIntruders();
    records.clear();
    if (loaded.ok) {
        for (const auto& item : loaded.records) records.push_back({item, "Active"});
    }
    rebuildRecordIndexes();
    productionGraph = makeProductionGraph();
}

void AppState::rebuildRecordIndexes()
{
    recordList = std::make_unique<SinglyLinkedList>();
    recordHash = std::make_unique<HashChaining>(10);
    nameTree = std::make_unique<BST>();
    for (const auto& item : records) {
        Intruder indexed = item.record;
        indexed.status = item.status;
        recordList->insert(indexed);
        recordHash->insert(indexed);
        nameTree->insert(indexed);
    }
}

void AppState::resetDemo()
{
    demoDataset = true;
    records.clear();
    alerts.clear();
    units.clear();
    activity.clear();
    incomingAlerts.clear();
    undoHistory.clear();
    redoHistory.clear();
    waitingPriority = dsa::PriorityQueue{};
    nextAlertSequence = 0;
    alertPolicy = "PRIORITY";

    const std::array<RecordEntry, 15> seededRecords{{
        {intruder(1047, "Ravi Kumar", "CP-03", 5), "Active"},
        {intruder(1048, "Meera Joshi", "CP-09", 4), "Active"},
        {intruder(1049, "Unknown Male 1", "CP-05", 3), "Active"},
        {intruder(1050, "Arjun Rao", "CP-07", 2), "Resolved"},
        {intruder(1051, "Zoya Khan", "CP-02", 3), "Active"},
        {intruder(1052, "Kabir Singh", "CP-10", 5), "Active"},
        {intruder(1053, "kabir Das", "CP-06", 1), "Resolved"},
        {intruder(1054, "Anita Desai", "CP-01", 4), "Dispatched"},
        {intruder(1055, "Mohan Lal", "CP-04", 2), "Active"},
        {intruder(1056, "Sana Iqbal", "CP-08", 3), "Dispatched"},
        {intruder(1057, "Vikram Nair", "CP-03", 5), "Active"},
        {intruder(1058, "Tara Menon", "CP-09", 2), "Resolved"},
        {intruder(1059, "Dev Patil", "CP-05", 4), "Active"},
        {intruder(1060, "Raj Malhotra", "CP-07", 1), "Resolved"},
        {intruder(1061, "Aarav Sharma", "CP-02", 3), "Active"},
    }};
    records.assign(seededRecords.begin(), seededRecords.end());
    for (auto& item : records) item.record.status = item.status;
    rebuildRecordIndexes();

    productionGraph = makeProductionGraph();

    units = {
        {"P-01", "CP-01", "Available", "none", isoAgo(1)},
        {"P-02", "CP-04", "Available", "none", isoAgo(2)},
        {"P-03", "CP-06", "Available", "none", isoAgo(4)},
        {"P-04", "CP-03", "Dispatched", "A-106", isoAgo(8)},
        {"P-05", "CP-07", "On Patrol", "routine", isoAgo(12)},
        {"P-06", "CP-08", "Dispatched", "A-107", isoAgo(16)},
        {"P-07", "CP-10", "Offline", "none", isoAgo(20)},
        {"P-08", "CP-05", "Available", "none", isoAgo(24)},
    };

    auto addAlert = [this](const char* id, int intruderId, int threat, const char* checkpoint,
                           const char* status, const char* unit = "") {
        AlertEntry item{id, intruderId, threat, checkpoint, status, unit, {}, nextAlertSequence++};
        alerts.push_back(item);
        incomingAlerts.enqueue(static_cast<int>(item.arrival));
        if (item.status == "Queued") waitingPriority.insert(static_cast<int>(item.arrival), threat);
    };
    addAlert("A-101",1047,5,"CP-03","Queued");
    addAlert("A-102",1052,5,"CP-10","Queued");
    addAlert("A-103",1057,5,"CP-03","Queued");
    addAlert("A-104",1048,4,"CP-09","Queued");
    addAlert("A-105",1059,4,"CP-05","Queued");
    addAlert("A-106",1054,4,"CP-01","Dispatched","P-04");
    addAlert("A-107",1056,3,"CP-08","Dispatched","P-06");
    addAlert("A-108",1049,3,"CP-05","Queued");
    addAlert("A-109",1051,3,"CP-02","Queued");
    addAlert("A-110",1061,3,"CP-02","Queued");
    addAlert("A-111",1050,2,"CP-07","Resolved");
    addAlert("A-112",1053,1,"CP-06","Resolved");

    const std::pair<const char*, const char*> events[] = {
        {"auth", "Admin login succeeded"}, {"intruder", "Intruder 1047 registered at CP-03"},
        {"alert", "A-101 queued at threat level 5"}, {"intruder", "Intruder 1048 registered at CP-09"},
        {"alert", "A-102 queued at threat level 5"}, {"dispatch", "Unit P-04 dispatched to A-106"},
        {"intruder", "Intruder 1057 registered at CP-03"}, {"alert", "A-103 queued at threat level 5"},
        {"dispatch", "Unit P-06 dispatched to A-107"}, {"resolution", "A-111 marked resolved"},
        {"system", "Demo checkpoint graph loaded"}, {"system", "Demo dataset seeded"},
    };
    for (std::size_t i = 0; i < std::size(events); ++i) {
        activity.push_back({"EV-" + std::to_string(i + 1), events[i].first, events[i].second,
                            isoAgo(static_cast<int>((12 - i) * 11))});
    }
    std::reverse(activity.begin(), activity.end());

    // Keep the demo persistence file separate from the user's live records.
    (void)saveRecords();
}

bool AppState::saveRecords() const
{
    std::vector<Intruder> output;
    output.reserve(records.size());
    for (const auto& entry : records) {
        Intruder item = entry.record;
        item.status = entry.status;
        output.push_back(std::move(item));
    }
    std::string path = "data/demo/intruders.txt";
    if (!demoDataset) {
        const char* configured = std::getenv("INTRUDER_DATA_FILE");
        path = configured != nullptr && *configured != '\0' ? configured : "intruders.txt";
    }
    return saveIntruders(output, path);
}

} // namespace border_api::services
