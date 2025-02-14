#include "memory.h"

Memory::Memory() : Memory(0) {}
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

Memory& Memory::operator=(const Memory& memory) {
	// Copy only as many bytes as we can store UNLESS our size is 0; in which case, increase the size of this
	// module to account for that.
	int bytesToCopy;
	if (this->data.size()) {
		bytesToCopy = this->data.size() > memory.data.size() ? memory.data.size() : this->data.size();
	} else {
		bytesToCopy = memory.data.size();
		for (int i = 0; i < bytesToCopy; ++i) {
			this->data.push_back(0);
		}
	}

	for (int i = 0; i < bytesToCopy; ++i) {
		this->data.at(i) = memory.data.at(i);
	}

	return *this;
}

uint8_t Memory::setByte(uint16_t address, uint8_t value) {
	// NOTE: experimenting with just using the modulo of the address.
	address %= this->data.size();
	
	//if (address >= this->data.size()) {  // Check if we are indexing the memory in a valid way.
		// TODO: Add warning message.
	//	return 0;
	//}


	uint8_t oldValue = this->data.at(address);
	this->data.at(address) = value;
	return oldValue;
}
