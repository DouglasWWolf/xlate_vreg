#include <cstdlib>
#include <string.h>
#include "amap_parser.h"

using std::string;
using std::map;


//=============================================================================
// halt() - Display an error messager and exit
//=============================================================================
static void halt()
{
    fprintf(stderr, "xlate_vreg: address map file is malformed\n");
    exit(1);
}
//=============================================================================


//=============================================================================
// chopped() - Returns everything prior to the final "/" in a string
//=============================================================================
static string chopped(string input)
{
    // Get a pointer to the start of the input string
    const char* start = input.c_str();

    // Find the nul-terminator of the string
    const char* end = strchr(start, 0);

    // Walk backwards, looking for a '/' separator
    while (--end > start)
    {
        if (*end == '/') break;
    }

    // Determine how long the remaining string is
    int length = end - start;

    // Hand the caller the string with the last part chopped off
    return input.substr(0, length);
}
//=============================================================================



//=============================================================================
// get_key_type() - Fetches the token after the last "." in the key
//=============================================================================
static string get_key_type(const char* start)
{
    char token[1000];

    // Find an equal sign
    const char* in = strchr(start, '=');

    // If there was no '=', give up
    if (in == nullptr) halt();

    // From the equal sign, back up until we find text
    while (in > start)
    {
        if (*--in != ' ') break;
    }

    // If we never found text, quit
    if (in == start) halt();

    // This points to the last character of the token we're fetching
    const char* last = in;

    // Keep backing up, looking for a '.'
    while (in > start)
    {
        if (*--in == '.') break;
    }

    // If we never found the '.', quit
    if (in == start) halt();

    // This is how long the token is
    int length = last - in;

    // Copy the token into our token buffer
    strncpy(token, in+1, length);
    token[length] = 0;

    // Hand the caller the key-type they're looking for
    return token;
}
//=============================================================================


//=============================================================================
// get_key_value() - Fetches the quoted token after the =
//=============================================================================
static string get_key_value(const char* start)
{
    char token[1000], *out = token;

    // Find an equal sign
    const char* in = strchr(start, '=');

    // If there was no '=', give up
    if (in == nullptr) halt();

    // Now find the opening quotation mark
    in = strchr(in+1, 34);

    // If there was no double-quote, give up
    if (in == nullptr) halt();

    // Skip over the opening double-quote
    ++in;

    while (true)
    {
        // Fetch the next input character
        int c = *in;
        
        // If we hit the end of line, this is malformed
        if (c == 0) halt();

        // If we hit the closing quote, we're done
        if (c == 34) break;

        // Stuff the character into our token buffer
        *out++ = *in++;
    }

    // Nul-terminate the token
    *out = 0;

    // Hand the caller the value they're looking for
    return token;
}
//=============================================================================




//=============================================================================
// parse_address_map() - Parses the output of "parse_xbd" to build a list of
//                       AXI connection names and their AXI addresses
//=============================================================================
void parse_address_map(string filename, map<uint64_t, addr_entry_t>* addrmap)
{
    char buffer[10000];
    addr_entry_t entry;
    
    // Open the input file and complain if we can't
    FILE* ifile = fopen(filename.c_str(), "r");
    if (ifile == nullptr)
    {
        fprintf(stderr, "xlate_vreg: can't open %s\n", filename.c_str());
        exit(1);
    }

    // Loop through every line of the input file
    while (fgets(buffer, sizeof buffer, ifile))
    {
        // Get a pointer to the input line
        const char* p = buffer;

        // Skip over whitespace
        while (*p == 32 || *p == 9 || *p == 10 || *p == 13) ++p;

        // If the line is blank, skip it
        if (*p == 0) continue;

        // The "key_type" is everything after the last "." in the key
        string key_type = get_key_type(p);

        // Fetch the value from this key/value pair
        string key_value = get_key_value(p);

        // On an "address_block", we just memorize the name of the connection
        if (key_type == "address_block")
        {
            entry.name = chopped(key_value);
            continue;
        }

        // On an "offset" block, we save this entry
        if (key_type == "offset")
        {
            entry.address = strtoull(key_value.c_str(), nullptr, 0);
            (*addrmap)[entry.address] = entry;
        }
    }

    // Close the input file, we're done
    fclose(ifile);
}
//=============================================================================