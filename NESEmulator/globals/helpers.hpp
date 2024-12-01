// helpers.hpp - A list of helper functions and definitions for use throughout this project.
#pragma once
#include <string>

#define displayHex(x, digits) "0x" << std::hex << std::setfill('0') << std::setw(digits) << (int)x

// Bool-to-string
inline const std::string btos(bool b) {
	return b ? "true" : "false";
}

inline const std::string btos(bool b, std::string trueOption, std::string falseOption) {
	return b ? trueOption : falseOption;
}

// 0-indexed.
template <typename T>
inline constexpr bool getBit(T i, int bitIdx) {
	T indexer = 1;
	indexer <<= bitIdx;

	return i & indexer;
}

// 0-indexed.
template <typename T>
inline constexpr void setBit(T& i, int bitIdx) {
	T indexer = 1;
	indexer <<= bitIdx;
	i |= indexer;
}

template <typename T>
inline constexpr void clrBit(T& i, int bitIdx) {
	T indexer = 1;
	indexer <<= bitIdx;  // e.g. bitIdx = 3, indexer = 0b1000
	indexer = ~indexer;  // e.g.   ''      , indexer = 0b0111
	i &= indexer;
}

/* Some terminology and acronyms used in the code:
 - xBI: x-based indexing; e.g. 0BI = 0-based indexing.
*/