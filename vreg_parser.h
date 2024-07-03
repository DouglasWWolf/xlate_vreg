#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <string>
void parse_verilog_regs(FILE* ifile, uint32_t base_addr, std::string prefix, FILE* ofile = stdout);