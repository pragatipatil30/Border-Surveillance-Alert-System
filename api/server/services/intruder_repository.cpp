#include "api/services/intruder_repository.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>

namespace border_api::services {
namespace {

std::string dataFilePath()
{
    const char* configuredPath = std::getenv("INTRUDER_DATA_FILE");
    return configuredPath != nullptr && *configuredPath != '\0'
        ? configuredPath
        : "intruders.txt";
}

} // namespace

IntruderLoadResult loadIntruders()
{
    const std::string path = dataFilePath();
    std::ifstream file(path);
    if (!file) {
        std::error_code error;
        if (!std::filesystem::exists(path, error) && !error) return {true, {}};
        return {false, {}};
    }

    std::vector<Intruder> records;
    std::unordered_set<int> ids;
    std::string line;
    while (std::getline(file, line)) {
        if (line.find_first_not_of(" \t\r\n") == std::string::npos) continue;

        Intruder record{};
        std::istringstream row(line);
        std::string extra;
        if (!(row >> record.id >> record.name >> record.location >> record.threatlevel) ||
            (row >> extra) || record.threatlevel < 1 || record.threatlevel > 5 ||
            !ids.insert(record.id).second) {
            return {false, {}};
        }
        records.push_back(std::move(record));
    }

    if (file.bad()) return {false, {}};
    return {true, std::move(records)};
}

} // namespace border_api::services
