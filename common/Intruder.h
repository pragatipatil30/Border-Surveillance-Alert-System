#ifndef INTRUDER_H
#define INTRUDER_H

#include <string>

struct Intruder
{
    int id;
    std::string name;
    std::string location;
    union { int threatlevel; int threatLevel; };
    std::string status;

    Intruder() : id(0), threatlevel(1), status("Active") {}
    Intruder(int recordId, const std::string& recordName, const std::string& recordLocation,
             int level, const std::string& recordStatus = "Active")
        : id(recordId), name(recordName), location(recordLocation), threatlevel(level), status(recordStatus) {}
};

#endif
