
/*
    This program generates a C/C++ header file from a collection of register
    definitions in Verilog/SystemVerilog source files
*/

#include "vreg_parser.h"
#include "amap_parser.h"

std::map<uint64_t, addr_entry_t> connection;

//=============================================================================
// 
//=============================================================================
int main(int argc, const char** argv)
{
    
    parse_address_map("addr_map.tmp", &connection);
    
    const char* fn = "input.v";
    FILE* ifile = fopen(fn, "r");
    if (ifile == nullptr) exit(1);

    parse_verilog_regs(ifile, 0x3000, "PFX");
};
//=============================================================================


//=============================================================================
// show_connection_names() - Displays a list of connection names and their 
//                           AXI addresses
//=============================================================================
void show_connection_names()
{
    for (auto& e : connection)
    {
        printf("0x%016lx  %s\n", e.first, e.second.name.c_str());
    }
}
//=============================================================================
