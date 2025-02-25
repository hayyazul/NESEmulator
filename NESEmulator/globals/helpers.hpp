// helpers.hpp - A list of helper functions and definitions for use throughout this project.
#pragma once
#include <string>
#define ASSERT_PARAMETERS
#ifdef ASSERT_PARAMETERS
#include <iostream>
#include <iomanip>
#include <bitset>
#endif

// If the digit to display is too big, it will truncate the left most digits first (so it will display the lower nibbles first).
#define displayHex(x, digits) "0x" << std::hex << std::setfill('0') << std::setw(digits) << ((int)x & ((1 << (digits * 4)) - 1)) 
#define displayBinary(x, digits) std::bitset<digits>((int)x)

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
inline constexpr T getBits(T i, int startIdx, int endIdx, bool keepPositions=true) {
	T srcIndexer = ((1 << (endIdx + 1)) - 1) ^ ((1 << startIdx) - 1);
	i &= srcIndexer;  // Select only the bits in this range.

	if (!keepPositions) {  // Normally, this function will preserve location, e.g. grabbing bits 2&3 from 0b1101 would give you 0b1100.
		// But if keep positions is disabled, the above would return 0b11 instead.
		i >>= startIdx;
	}

	return i;
}

// 0-indexed.
template <typename T>
inline constexpr void setBit(T& i, int bitIdx) {
	T indexer = 1;
	indexer <<= bitIdx;
	i |= indexer;
}

/* Copies a range of bits from a source value to the same location onto the destination value.
e.g. dst = 0b10110110, src = 0b11110000, startIdx = 0, endIdx = 3; would result in dst = 0b10110000.
Assumes endIdx > startIdx.
*/
template <typename T>
inline constexpr void copyBits(T& dst, T src, int startIdx, int endIdx) {
	// Example: endIdx = 4, startIdx = 2. The left: = 0b100000 - 1 = 0b11111, the right: = 0b11. XORing the results = 0b11100.
	#ifdef ASSERT_PARAMETERS
	if (endIdx < startIdx) {
		std::cout << "Warning: copyBits called w/ invalid start (" << std::dec << startIdx << ") and end (" << endIdx << ") indices; unexpected behavior is likely to follow." << std::endl;
	}
	#endif
	
	T srcIndexer = ((1 << (endIdx + 1)) - 1) ^ ((1 << startIdx) - 1);
	src &= srcIndexer;  // Select only the bits in this range.
	dst &= ~srcIndexer;  // Keep all bits outside of this range--- we don't touch those.
	dst |= src;  // Now we can OR the two together.
}

/* Copies a range from src onto a possibly different range on dst.
Assumes the ranges are equal, but does not perform a check for this condition.
*/
template <typename T>
inline constexpr void copyBits(T& dst, int dstStartIdx, int dstEndIdx, T src, int srcStartIdx, int srcEndIdx) {
	// Example: endIdx = 4, startIdx = 2. The left: = 0b10000 - 1 = 0b1111, the right: = 0b11. XORing the results = 0b1100.
	#ifdef ASSERT_PARAMETERS
	if (dstEndIdx < dstStartIdx) {
		std::cout << "Warning: copyBits called w/ invalid dst start (" << std::dec << dstStartIdx << ") and end (" << dstEndIdx << ") indices; unexpected behavior is likely to follow." << std::endl;
	} else if (srcEndIdx < srcStartIdx) {
		std::cout << "Warning: copyBits called w/ invalid src start (" << std::dec << srcStartIdx << ") and end (" << srcEndIdx << ") indices; unexpected behavior is likely to follow." << std::endl;
	} else if (dstEndIdx - dstStartIdx != srcEndIdx - srcStartIdx) {
		std::cout << "Warning: copyBits called w/ an src range different from the dst range; srcStartIdx = " << std::dec << srcStartIdx << ", srcEndIdx = " << srcEndIdx 
				  << ", dstStartIdx = "<< dstStartIdx << ", dstEndIdx = " << dstEndIdx << "; unexpected behavior is likely to follow." << std::endl;
	}

	#endif
	T indexer = ((1 << (srcEndIdx + 1)) - 1) ^ ((1 << srcStartIdx) - 1);
	src &= indexer;  // Select only the bits in this range.
	// Now, we shift the modified src based on the relative positions.
	// We will also shift the indexer. Instead of indexing src, it will be used to index dst.
	int relativeOffset;
	if (srcStartIdx > dstStartIdx) {  // Shift src to the right.
		relativeOffset = srcStartIdx - dstStartIdx;
		indexer >>= relativeOffset; 
		src >>= relativeOffset;
	} else {  // Shift src to the left.
		relativeOffset = dstStartIdx - srcStartIdx;
		indexer <<= relativeOffset;
		src <<= relativeOffset;
	}
	dst &= ~indexer;  // Keep all bits outside of this range--- we don't touch those.
	dst |= src;  // Now we can OR the two together.
}

template <typename T>
inline constexpr void clrBit(T& i, int bitIdx) {
	T indexer = 1;
	indexer <<= bitIdx;  // e.g. bitIdx = 3, indexer = 0b1000
	indexer = ~indexer;  // e.g.   ''      , indexer = 0b0111
	i &= indexer;
}

template <typename T>
inline constexpr T reverseBits(T bits, int numOfBits) {
	// Reverses the order of bits.
	T lowerBitIndexer = 0b1;
	T upperBitIndexer = 1 << (numOfBits - 1);
	for (int i = 0, j = numOfBits - 1; upperBitIndexer > lowerBitIndexer; ++i, --j) {
		bool upperBitValue = upperBitIndexer & bits;
		if (lowerBitIndexer & bits) {
			setBit(bits, j);
		} else {
			clrBit(bits, j);
		}

		if (upperBitValue) {
			setBit(bits, i);
		} else {
			clrBit(bits, i);
		}
		
		lowerBitIndexer <<= 1;
		upperBitIndexer >>= 1;
	}

	return bits;
};

template <typename T>
inline constexpr T reverseBits(T bits) {
	return reverseBits(bits, sizeof(T) * 8);
}

template <typename T>
inline constexpr void removeElementFromVec(std::vector<T>& vec, T& elementVal) {
	vec.erase(std::remove(vec.begin(),
		vec.end(), elementVal), vec.end());
}

/* Some terminology and acronyms used in the code:
 - xBI: x-based indexing; e.g. 0BI = 0-based indexing.
*/