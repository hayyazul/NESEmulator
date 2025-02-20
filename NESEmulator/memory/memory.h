// Container for the data the NES will access. TODO: Rename because this name
// conflicts with the std memory library.
#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <string>

constexpr int BYTES_OF_MEMORY = 0x10000;

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
	Memory(unsigned int size);
	~Memory();

	virtual uint8_t getByte(uint16_t address) const;
	virtual uint8_t setByte(uint16_t address, uint8_t value);  // Returns the old value at the given address.
    // Copies the data from one memory module to another as much as it can (limit is module w/ fewer allocated bytes).
	Memory& operator=(const Memory& memory);

	// Gets the data contained in this memory module as a comma-seperated string.
	std::string getDataAsStr() const;
private:
	std::vector<uint8_t> data;  // Might change from vector to array if this proves too slow..
	friend Memory;
};