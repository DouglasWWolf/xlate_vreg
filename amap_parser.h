#pragma once
#include <cstdint>
#include <string>
#include <map>

struct addr_entry_t
{
    std::string name;
    uint64_t    address;
    std::string filename;
    std::string prefix;
};

void parse_address_map(std::string filename, std::map<uint64_t, addr_entry_t>* addrmap);


