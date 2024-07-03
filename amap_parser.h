#pragma once
#include <cstdint>
#include <string>
#include <map>

struct connection_t
{
    std::string name;
    uint64_t    address;
    std::string filename;
    std::string prefix;
};

void parse_address_map(std::string filename, std::map<std::string, connection_t>* addrmap);


