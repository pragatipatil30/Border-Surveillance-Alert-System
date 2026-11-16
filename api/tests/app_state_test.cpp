#include <cassert>
#include <cstdlib>
#include <string>
#include <vector>

#include "api/services/app_state.hpp"
#include "api/services/intruder_repository.hpp"

int main()
{
    _putenv_s("API_DEMO", "1");
    auto& state = border_api::services::AppState::instance();
    state.initialize();

    assert(state.demoDataset);
    assert(state.records.size() == 15);
    assert(state.alerts.size() == 12);
    assert(state.units.size() == 8);
    assert(state.recordList->size() == 15);
    assert(state.recordHash->size() == 15);
    assert(state.nameTree->size() == 15);
    assert(state.productionGraph->vertexCount() == 10);
    assert(state.productionGraph->edgeCount() == 15);
    assert(state.activity.size() >= 12);

    const auto* first = state.recordHash->search(1047);
    const auto* second = state.recordHash->search(1057);
    assert(first && second);
    assert(1047 % state.recordHash->getCapacity() == 1057 % state.recordHash->getCapacity());
    assert(state.recordHash->getCapacity() == 10);

    std::vector<Intruder> prefix;
    state.nameTree->prefixSearch("ka", prefix);
    assert(prefix.size() == 2);
    assert(prefix[0].name == "kabir Das" || prefix[1].name == "kabir Das");

    auto waiting = state.waitingPriority;
    border_api::dsa::PriorityItem item{};
    assert(waiting.extract(item) && item.value == 0 && item.priority == 5);
    assert(waiting.extract(item) && item.value == 1 && item.priority == 5);
    assert(waiting.extract(item) && item.value == 2 && item.priority == 5);

    assert(state.units[3].status == "Dispatched" && state.units[3].assignment == "A-106");
    assert(state.units[5].status == "Dispatched" && state.units[5].assignment == "A-107");
    assert(state.records[7].status == "Dispatched");
    assert(state.records[9].status == "Dispatched");

    const auto loaded = border_api::services::loadIntruders();
    assert(loaded.ok && loaded.records.size() == 15);
    assert(loaded.records[2].name == "Unknown Male 1");
}
