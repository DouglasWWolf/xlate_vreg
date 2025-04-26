#include <vector>
#include <string>
#include <string.h>
#include "vreg_parser.h"

// Allow the convenient usage of STL containers 
using std::vector;
using std::string;


// We will keep a vector of various kinds of definitions.
// Each entry in the vector is one of these
struct entry_t
{
    string  key;
    string  name;
    string  width;
    string  pos; 
    string  type;
    string  reset;
    string  desc;
};

vector<entry_t> definition;


//=============================================================================
// chomp() - Chops the end-of-line character off the end of a buffer
//=============================================================================
static void chomp(char* p)
{
    char* eol;

    eol = strchr(p, '\n');
    if (eol) *eol = 0;
    eol = strchr(p, '\r');
    if (eol) *eol = 0;
}
//=============================================================================


//=============================================================================
// decode_int() - Decodes a string to a 32-bit integer
//=============================================================================
static uint32_t decode_int(string input)
{
    char    buffer[100], *out = buffer;

    // Get a pointer to the input string
    const char* in = input.c_str();

    // Copy the input string to the buffer, removing underscores as we go
    while ((out - buffer) < sizeof(buffer - 1))
    {
        if (*in == 0) break;
        if (*in == '_')
        {
            in++;
            continue;
        }
        *out++ = *in++;
    }

    // Nul-terminate the buffer
    *out = 0;

    // If there is a 'h in the string, it's in hex
    in = strstr(buffer, "'h");
    if (in) return strtoul(in+2, nullptr, 16);

    // If there is a 'd in the string, it's in decimal
    in = strstr(buffer, "'d");
    if (in) return strtoul(in+2, nullptr, 10);

    // In all other cases, we won't assume we know the radix
    return strtoul(buffer, nullptr, 0);
}
//=============================================================================



//=============================================================================
// skip_whitespace() - Skips over whitespace
//=============================================================================
static const char* skip_whitespace(const char* p)
{
    while (*p == ' ' || *p == 9) ++p;
    return p;
}
//=============================================================================

//=============================================================================
// remaining_text() - Returns a string containing the remaining text in the
//                    buffer
//=============================================================================
static string remaining_text(const char* p)
{
    p = skip_whitespace(p);
    return p;
}
//=============================================================================

//=============================================================================
// get_next_token() - Skips over any leading whitespace and returns the
//                    next token
//=============================================================================
static const char* get_next_token(const char* in, string* output)
{
    char buffer[5000], *out = buffer;

    // Clear the caller's output field
    output->clear();

    // Skip over any leading whitespace
    in = skip_whitespace(in);

    while (true)
    {
        int c = *in;
        if (c == 32 || c == 9 || c == 0) break;
        *out++ = *in++;
    }

    // Nul-terminate the buffer
    *out = 0;

    // Fill in the caller's return field
    *output = buffer;

    // Skip over whitespace
    in = skip_whitespace(in);

    // If there's a trailing comma, skip it
    if (*in == ',') ++in;

    // Hand the caller a pointer to the character after the input token
    return in;
}
//=============================================================================


//=============================================================================
// dump_definition() - This prints out the current set of definitions for
//                     debugging
//=============================================================================
#if 0
static void dump_definition()
{
    for (auto& e : definition)
    {
        printf("%s: %s, %s, %s, %s, %s, \"%s\"\n",
           e.key.c_str(),
           e.name.c_str(),
           e.width.c_str(),
           e.pos.c_str(),
           e.type.c_str(),
           e.reset.c_str(),
           e.desc.c_str() 
        );       
    }
}
#endif
//=============================================================================

//=============================================================================
// pos_str() - Returns a string representation of a bit position
//=============================================================================
static string pos_string(uint32_t pos, uint32_t width)
{
    char buffer[100];
    if (width < 2)
        sprintf(buffer, "%u", pos);
    else
        sprintf(buffer, "%02u:%02u", pos+width-1, pos);
    return buffer;
}
//=============================================================================


//=============================================================================
// write_register_documentation() - Outputs "//" comments that describe
//                                  the register
//=============================================================================
static void write_register_documentation(FILE* ofile, string register_name)
{
    int field_count = 0;

    fprintf(ofile, "//\n");
    fprintf(ofile, "// Register:    %s\n", register_name.c_str());

    // Loop through every entry in the defintion
    for (auto& e : definition)
    {

        if (e.key == "@register")
        {
            fprintf(ofile,  "// Description: %s\n", e.desc.c_str());
            continue;
        }

        if (e.key == "@rdesc")
        {
            fprintf(ofile, "//                                         ");
            fprintf(ofile, "                      %s\n", e.desc.c_str());
            continue;            
        }

        if (e.key == "@field")
        {
            uint32_t width = decode_int(e.width);
            uint32_t pos   = decode_int(e.pos);

            if (field_count++ == 0)
            {
                fprintf(ofile, "// Fields:\n");
                fprintf(ofile, "//     NAME                           WID   POS TYPE RESET       DESCRIPTION\n");
            }            

            fprintf
            (
                ofile,
                "//     %-30s %-3u %5s %-4s %-11s %s\n",
                e.name.c_str(),
                width,
                pos_string(pos, width).c_str(),
                e.type.c_str(),
                e.reset.c_str(),
                e.desc.c_str()
            );

            continue;
        }

        if (e.key == "@fdesc")
        {
            fprintf(ofile, "//                                         ");
            fprintf(ofile, "                      %s\n", e.desc.c_str());
            continue;            
        }

    }

    // Leave a blank line at the end to visually offset it
    fprintf(ofile, "//\n");    
}
//=============================================================================


//=============================================================================
// write_c_constants() - Output the #define statements that C/C++ require
//=============================================================================
static void write_c_constants(FILE* ofile, string reg_name, uint32_t reg_addr)
{
    fprintf(ofile, "#define %-60s 0x%016xULL\n", reg_name.c_str(), reg_addr);

    // Loop through every entry in the defintion
    for (auto& e : definition)
    {
        if (e.key == "@field")
        {
            uint32_t width = decode_int(e.width);
            uint32_t pos   = decode_int(e.pos);
            uint32_t spec  = (width << 24) | (pos << 16);
            string   field = reg_name + "_" + e.name;
            fprintf(ofile, "#define %-60s 0x%08x%08xULL\n", field.c_str(), spec, reg_addr);
        }
    }

    // Leave a couple of blank lines after every set of constants
    fprintf(ofile, "\n\n");
}
//=============================================================================



//=============================================================================
// parse_localparam_name() - Fetches the name of the localparam constant
//=============================================================================
static string parse_localparam_name(const char* in)
{
    char buffer[1000], *out = buffer;

    // Skip over whitespace
    skip_whitespace(in);

    // Copy the localparam name into the buffer
    while (true)
    {
        int c = *in;
        if (c == 32 || c == 9 || c == 0 || c == '=') break;
        *out++ = *in++;
    }

    // nul-terminate the buffer
    *out = 0;

    // The result string is in our buffer
    const char* result = buffer;

    // Hand the resulting name to the caller
    return (result[0]) ? result : "no_localparam_name";
}
//=============================================================================


//=============================================================================
// parse_localparam_value() - Fetches the value of the localparam constant
//=============================================================================
static uint32_t parse_localparam_value(const char* in)
{
    // Find the '=' character
    in = strchr(in, '=');

    // If there's no "=", there's no value
    if (in == nullptr) return 0;

    // Skip over any whitespace
    in = skip_whitespace(in+1);

    // And hand the caller the decoded integer value
    return decode_int(in);
}
//=============================================================================


//=============================================================================
// make_reg_name() - Make a register name from a localparam name
//=============================================================================
static string make_reg_name(string lparam, string prefix)
{
    // If the localparam name doesn't start with "REG_", its not a register
    if (lparam.substr(0, 4) != "REG_") return "";
    
    // Strip "REG_" from the front of the name
    string reg_name = lparam.substr(4, 1000);

    // Do we need to prepend a prefix?
    if (!prefix.empty())
    {
        reg_name = prefix + "_" + reg_name;
    }

    // Hand the register name to the caller
    return reg_name;
}
//=============================================================================

//=============================================================================
// parse_verilog_regs() - Reads in a Verilog file containing register
//                        definitions and outputs the corresponding C/C++
//                        header file.
//=============================================================================
void parse_verilog_regs(FILE* ifile, uint32_t base_addr, string prefix, FILE* ofile)
{
    entry_t entry;
    char    buffer[1000];

    // Loop through each line of the input
    while (fgets(buffer, sizeof buffer, ifile))
    {
        // Remove the CR or LF from the end of the line
        chomp(buffer);

        // Skip past any leading whitespace
        const char* in = skip_whitespace(buffer);

        // Skip any line that is blank, a comment, /* or */
        if (*in == 0)                     continue;
        if (in[0] == '/' && in[1] == '/') continue;
        if (in[0] == '/' && in[1] == '*') continue;
        if (in[0] == '*' && in[1] == '/') continue;

        // Fetch the first token on the line
        in = get_next_token(in, &entry.key);
        
        // Are we defining a new register?
        if (entry.key == "@register")
        {
            definition.clear();
            entry.desc = remaining_text(in);
            definition.push_back(entry);
            continue;
        }


        // Are we adding a comment line to a register or field description?
        if (entry.key == "@fdesc" || entry.key == "@rdesc")
        {
            entry.desc = remaining_text(in);
            definition.push_back(entry);
            continue;
        }

        // Was this a "@field" definition?
        if (entry.key == "@field")
        {
            in = get_next_token(in, &entry.name);
            in = get_next_token(in, &entry.width);
            in = get_next_token(in, &entry.pos);
            in = get_next_token(in, &entry.type);
            in = get_next_token(in, &entry.reset);
            entry.desc = remaining_text(in);
            definition.push_back(entry);
            continue;
        }


        // Is it time to output some documentation and definitions?
        if (entry.key == "localparam" && definition.size())
        {
            string   lparam_name  = parse_localparam_name(in);
            uint32_t lparam_value = parse_localparam_value(in);
            string   reg_name = make_reg_name(lparam_name, prefix);
            uint32_t reg_addr = base_addr + (lparam_value * 4);
 
            if (!reg_name.empty())
            {
                write_register_documentation(ofile, reg_name);
                write_c_constants(ofile, reg_name, reg_addr);
                definition.clear();
            }
        }

    }

}
//=============================================================================

