#include "memory.h"
#include <sstream>

Memory::Memory() : Memory(0) {}
Memory::Memory(unsigned int size) {
	for (unsigned int i = 0; i < size; ++i) {
		this->data.push_back(0);
	}
}
Memory::~Memory() {}

uint8_t Memory::getByte(uint16_t address) const {
	if (address >= this->data.size()) {  // Check if we are indexing the memory in a valid way.
		return 0;
	}
	return this->data.at(address);
}

Memory& Memory::operator=(const Memory& memory) {
	// Copy only as many bytes as we can store UNLESS our size is 0; in which case, increase the size of this
	// module to account for that.
	size_t bytesToCopy;
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

std::string Memory::getDataAsStr() const {
	std::stringstream serialStr;
	// We will iterate through the data, appending it to the string as we go along.
	if (this->data.size() == 0) return serialStr.str();  // Edge case: if we have no data, then return an empty string.
	
	// Append the first byte w/o a comma since it is the first element. 
	serialStr << (int)this->data.at(0);
	for (int i = 1; i < this->data.size(); ++i) {
		serialStr << " " << (int)this->data.at(i);
	}

	return serialStr.str();
}

uint8_t Memory::setByte(uint16_t address, uint8_t value) {
	// NOTE: experimenting with just using the modulo of the address.
	address %= this->data.size();
	uint8_t oldValue = this->data.at(address);
	this->data.at(address) = value;
	return oldValue;
}
