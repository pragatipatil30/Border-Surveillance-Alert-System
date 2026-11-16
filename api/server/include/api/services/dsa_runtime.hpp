#pragma once

#include <memory>
#include <mutex>

#include "Graph.h"
#include "GraphAlgorithms.h"
#include "HashChaining.h"
#include "api/dsa/priority_queue.hpp"

namespace border_api::services {

struct DsaRuntime {
    std::mutex mutex;
    SimpleStack<int> stack;
    SimpleQueue<int> queue;
    dsa::PriorityQueue priorityQueue;
    HashChaining hashMap{17};
    std::unique_ptr<Graph> graph;

    static DsaRuntime& instance();
};

} // namespace border_api::services
