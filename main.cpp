
#define REVISION "1.0"

/*
    This program generates a C/C++ header file from a collection of register
    definitions in Verilog/SystemVerilog source files
*/

#include <string>
#include "config_file.h"
#include "vreg_parser.h"
#include "amap_parser.h"
using std::string;
using std::map;

map<uint64_t, addr_entry_t> connection;
string input_file;
bool   show_names;

void execute();
void parse_command_line(const char** argv);


//=============================================================================
// main() - Execution starts here
//=============================================================================
int main(int argc, const char** argv)
{
    parse_command_line(argv);

    try
    {
        execute();
    }
    catch(const std::exception& e)
    {
        fprintf(stderr, "xlate_vreg: %s\n", e.what());
        exit(1);
    }
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


//=============================================================================
// show_help() - Display some minimal help
//=============================================================================
void show_help()
{
    printf("xlate_vreg %s\n", REVISION);
    printf("usage: xlate_vreg [-names] <filename>\n");
    exit(1);
}
//=============================================================================


//=============================================================================
// parse_command_line() - Extracts parameters and switches from command line
//=============================================================================
void parse_command_line(const char** argv)
{
    int idx = 0;

    // Loop through all of the command line parameters
    while (argv[++idx])
    {
        // Fetch this token
        string token = argv[idx];

        // Does the user want to just see a list of connection names?
        if (token == "-names")
        {
            show_names = true;
            continue;
        }

        // If this is an unknown command line switch, complain
        if (argv[idx][0] == '-')
            show_help();

        // A non-switch argument is the name of the input file
        input_file = token;
    }

    // If we don't have the name of an input file, complain
    if (input_file.empty()) show_help();
}
//=============================================================================


//=============================================================================
// read_config_file() - Read the contents of the configuration file
//=============================================================================
void read_config_file(string filename)
{
    CConfigFile config;

    config.read(filename, true);

}
//=============================================================================

//=============================================================================
// execute() - Performs most of the work of this program
//=============================================================================
void execute()
{
    // Build our "connection map" from the input file
    parse_address_map(input_file, &connection);

    // If the user just wants to see the connection names, show them
    if (show_names)
    {
        show_connection_names();
        exit(0);
    }

    // Read our configuration file
    read_config_file("xlate_vreg.conf");
    
    const char* fn = "input.v";
    FILE* ifile = fopen(fn, "r");
    if (ifile == nullptr) exit(1);

    parse_verilog_regs(ifile, 0x3000, "PFX");

}
//=============================================================================