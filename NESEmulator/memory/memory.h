// Container for the data the NES will access. 
#pragma once

#include <cstdint>
#include <array>

constexpr int BYTES_OF_MEMORY = 0xFFFF;

/*
Basic memory class. Right now it is no different from just an array,
but I may use it as a parent class for future memory modules when I 
implement paging, etc.

I will also implement the quirks of memory addressing on the NES either
here or the databus, depending on its exact functionality.
*/
class Memory {
public:
	Memory();
	~Memory();

	uint8_t getByte(uint16_t address) const;
	uint8_t setByte(uint16_t address, uint8_t value);

private:
	std::array<uint8_t, BYTES_OF_MEMORY>* data;  // Allocated on the heap.
};