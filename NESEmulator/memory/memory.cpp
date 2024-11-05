#include "memory.h"

Memory::Memory() {
	this->data = new std::array<uint8_t, BYTES_OF_MEMORY>;
}

Memory::~Memory() {
	delete this->data;
}

uint8_t Memory::getByte(uint16_t address) const {
	return this->data->at(address);
}

uint8_t Memory::setByte(uint16_t address, uint8_t value) {
	this->data->at(address) = value;
	return this->data->at(address);
}
