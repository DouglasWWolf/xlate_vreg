
#define REVISION "1.0"

/*
    This program generates a C/C++ header file from a collection of register
    definitions in Verilog/SystemVerilog source files
*/

#include <string>
#include <cstdarg>
#include <stdexcept>
#include "config_file.h"
#include "vreg_parser.h"
#include "amap_parser.h"
using std::string;
using std::map;

struct src_entry_t
{
    string name;
    string filename;
    string prefix;
};

// Maps a connection name to a connection entry
map<string, connection_t> connection;

// Maps a connection name to a source file and prefix
map<string, src_entry_t> src_map;

string input_file;
string config_file = "xlate_vreg.conf";

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
// throwRuntime() - Throws a runtime exception
//=============================================================================
static void throwRuntime(const char* fmt, ...)
{
    char buffer[1024];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buffer, fmt, ap);
    va_end(ap);

    throw std::runtime_error(buffer);
}
//=============================================================================



//=============================================================================
// reorder_connections() - Returns a copy of the "connection" map, reordered
//                         by AXI address
//=============================================================================
map<uint64_t, connection_t> reorder_connections()
{
    map<uint64_t, connection_t> result;

    for (auto& c : connection)
    {
        result[c.second.address] = c.second;
    }

    return result;
}
//=============================================================================


//=============================================================================
// show_connection_names() - Displays a list of connection names and their 
//                           AXI addresses
//=============================================================================
void show_connection_names()
{
    auto reordered = reorder_connections();
    for (auto& e : reordered)
    {
        printf("0x%016lx  %s\n", e.second.address, e.second.name.c_str());
    }
}
//=============================================================================


//=============================================================================
// show_help() - Display some minimal help
//=============================================================================
void show_help()
{
    printf("xlate_vreg %s\n", REVISION);
    printf("usage: xlate_vreg [-names] [-config <config_file>] <filename>\n");
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

        // Is the user supplying the name of a config file?
        if (token == "-config" && argv[idx+1])
        {
            config_file = argv[++idx];
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
    src_entry_t   entry;
    CConfigFile   config;
    CConfigScript script;

    // If errors occur, throw exceptions
    config.throw_on_fail(true);

    // Open the configuration file
    if (!config.read(filename, false))
    {
        throwRuntime("can't open %s", filename.c_str());
    }

    // Fetch the "connections" script
    config.get("connections", &script);

    // Loop through each line of that script, and add an entry
    // to the "src_map"
    while (script.get_next_line())
    {
        entry.name     = script.get_next_token();
        entry.filename = script.get_next_token();
        entry.prefix   = script.get_next_token();
        src_map[entry.name] = entry;
    }

}
//=============================================================================


//=============================================================================
// merge_maps() - Fill in missing fields in "connection" from the matching 
//                connection names in "src_map"
//=============================================================================
void merge_maps()
{

    // Loop through every connection in the Xilinx project
    for (auto& c : connection)
    {
        // Fetch the connection name
        string connection_name = c.second.name;

        // Does this connection exist in the src_map?
        auto it = src_map.find(connection_name);

        // If this connection isn't in the src_map, complain
        if (it == src_map.end())
        {
            throwRuntime
            (   "'%s' not defined in %s",
                connection_name.c_str(),
                config_file.c_str()
            );
        }

        // Get a handy reference to the entry that corresponds to this name
        auto& entry = it->second;

        // Fill in the fields in "connection" with their corresponding
        // values from "src_map"        
        c.second.filename = entry.filename;
        c.second.prefix   = entry.prefix;

    }
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
    read_config_file(config_file);

    // Fill in fields in the "addr_map" from matching names in the "src_map"
    merge_maps();

    printf("Done\n"); exit(1);
    
    const char* fn = "input.v";
    FILE* ifile = fopen(fn, "r");
    if (ifile == nullptr) exit(1);

    parse_verilog_regs(ifile, 0x3000, "PFX");

}
//=============================================================================