#pragma once

#include <vector>

#include "Intruder.h"

namespace border_api::services {

struct IntruderLoadResult {
    bool ok;
    std::vector<Intruder> records;
};

// Reads the same whitespace-delimited file used by the existing console app.
// The file is re-read per request so changes saved by that app become visible.
IntruderLoadResult loadIntruders();

} // namespace border_api::services
