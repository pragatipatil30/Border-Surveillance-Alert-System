#pragma once

#include <mutex>
#include <string>
#include <vector>
#include <memory>
#include <chrono>

#include "BST.h"
#include "Graph.h"
#include "HashChaining.h"
#include "SinglyLinkedList.h"
#include "api/dsa/priority_queue.hpp"

namespace border_api::services {

struct RecordEntry {
    Intruder record{};
    std::string status;
};

struct AlertEntry {
    std::string id;
    int intruderId = 0;
    int threat = 0;
    std::string checkpoint;
    std::string status;
    std::string assignedUnit;
    std::vector<std::string> route;
    std::size_t arrival = 0;
    int distanceKm = 0;
};

struct UnitEntry {
    std::string id;
    std::string checkpoint;
    std::string status;
    std::string assignment;
    std::string updated;
};

struct ActivityEntry {
    std::string id;
    std::string category;
    std::string message;
    std::string timestamp;
};

struct RecordHistory {
    std::string action;
    RecordEntry before;
    RecordEntry after;
};

// Shared process state. Every route must hold mutex while reading or changing
// these fields. The real user record file is never used as the demo seed file.
struct AppState {
    static AppState& instance();
    AppState();

    void initialize();
    void resetDemo();
    void rebuildRecordIndexes();
    bool saveRecords() const;
    bool demoDataset = true;
    std::mutex mutex;
    std::vector<RecordEntry> records;
    std::vector<AlertEntry> alerts;
    std::vector<UnitEntry> units;
    std::vector<ActivityEntry> activity;
    SimpleQueue<int> incomingAlerts;
    std::vector<RecordHistory> undoHistory;
    std::vector<RecordHistory> redoHistory;
    std::unique_ptr<SinglyLinkedList> recordList;
    std::unique_ptr<HashChaining> recordHash;
    std::unique_ptr<BST> nameTree;
    std::unique_ptr<Graph> productionGraph;
    dsa::PriorityQueue waitingPriority;
    std::size_t nextAlertSequence = 0;
    std::string alertPolicy = "PRIORITY";
    std::string startedAt;
    std::chrono::steady_clock::time_point startedClock = std::chrono::steady_clock::now();
};

} // namespace border_api::services
