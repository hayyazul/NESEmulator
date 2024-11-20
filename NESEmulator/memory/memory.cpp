#include "memory.h"

Memory::Memory() : Memory(BYTES_OF_MEMORY) {}
Memory::Memory(unsigned int size) {
	for (unsigned int i = 0; i < size; ++i) {
		this->data.push_back(0);
	}
}
Memory::~Memory() {}

uint8_t Memory::getByte(uint16_t address) const {
	if (address >= this->data.size()) {  // Check if we are indexing the memory in a valid way.
		// TODO: Add warning message.
		return 0;
	}
	return this->data.at(address);
}

uint8_t Memory::setByte(uint16_t address, uint8_t value) {
	if (address >= this->data.size()) {  // Check if we are indexing the memory in a valid way.
		// TODO: Add warning message.
		return 0;
	}
	this->data.at(address) = value;
	return this->data.at(address);
}
