
/*
    This program generates a C/C++ header file from a collection of register
    definitions in Verilog/SystemVerilog source files
*/

#include "vreg_parser.h"

//=============================================================================
// 
//=============================================================================
int main(int argc, const char** argv)
{
    const char* fn = "input.v";
    FILE* ifile = fopen(fn, "r");
    if (ifile == nullptr) exit(1);

    parse_verilog_regs(ifile, 0x3000, "PFX");
};
//=============================================================================