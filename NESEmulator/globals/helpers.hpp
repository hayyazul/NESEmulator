// helpers.hpp - A list of helper functions and definitions for use throughout this project.
#pragma once

#define displayHex(x, digits) "0x" << std::hex << std::setfill('0') << std::setw(digits) << (int)x