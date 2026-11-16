#include "api/services/intruder_repository.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_set>
#if defined(_WIN32)
#include <windows.h>
#endif

namespace border_api::services {
namespace {

std::string dataFilePath()
{
    const char* configuredDemo = std::getenv("API_DEMO");
    if (configuredDemo == nullptr || std::string(configuredDemo) != "0")
        return "data/demo/intruders.txt";
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
        if (!(row >> record.id >> std::quoted(record.name) >> std::quoted(record.location) >> record.threatlevel) ||
            (row >> extra) || record.threatlevel < 1 || record.threatlevel > 5 ||
            !ids.insert(record.id).second) {
            return {false, {}};
        }
        records.push_back(std::move(record));
    }

    if (file.bad()) return {false, {}};
    return {true, std::move(records)};
}

bool saveIntruders(const std::vector<Intruder>& records)
{
    return saveIntruders(records, dataFilePath());
}

bool saveIntruders(const std::vector<Intruder>& records, const std::string& destination)
{
    const std::filesystem::path path(destination);
    std::error_code error;
    if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path(), error);
    if (error) return false;
    const std::filesystem::path temporary = path.string() + ".tmp";
    {
        std::ofstream file(temporary, std::ios::trunc);
        if (!file) return false;
        for (const auto& record : records) {
            file << record.id << ' ' << std::quoted(record.name) << ' ' << std::quoted(record.location) << ' '
                 << record.threatlevel << '\n';
        }
        file.flush();
        if (!file) return false;
    }
#if defined(_WIN32)
    if (!MoveFileExA(temporary.string().c_str(), path.string().c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        std::filesystem::remove(temporary, error);
        return false;
    }
#else
    std::filesystem::rename(temporary, path, error);
    if (error) { std::filesystem::remove(temporary); return false; }
#endif
    return true;
}

} // namespace border_api::services
