// ============================================================================
//  common/Intruder.h
//  Shared data model for the Border Command Center project.
//
//  PROPOSED VERSION: all three members must agree on these fields on Day 1.
//  Once agreed, nobody changes this file without telling the others.
// ============================================================================
#ifndef INTRUDER_H
#define INTRUDER_H

#include <string>

struct Intruder
{
    int         id;
    std::string name;
    std::string location;
    int         threatLevel;   // 1 (low) .. 5 (critical)
    std::string status;        // "Active", "Dispatched", "Resolved"

    Intruder()
        : id(0), name(""), location(""), threatLevel(1), status("Active") {}

    Intruder(int i, const std::string& n, const std::string& loc,
             int threat, const std::string& st = "Active")
        : id(i), name(n), location(loc), threatLevel(threat), status(st) {}
};

#endif // INTRUDER_H
