#include "ram.h"

RAM::RAM() : Memory(SIZE_OF_RAM) {}

RAM::~RAM() {}

uint8_t RAM::getByte(uint16_t address) const {
	// We want to mirror addresses 0x0000 to ox2000 to mirror addresses 0x0000 to 0x0800.
	if (address >= SIZE_OF_RAM) {
		return 0;  // If the address is out of bounds, default to 0.
	}
	uint16_t adjustedAddress = address % SIZE_OF_RAM;  // Mirror all addresses > 0x0800.
	return Memory::getByte(adjustedAddress);
}

uint8_t RAM::setByte(uint16_t address, uint8_t value) {	
	// We want to mirror addresses 0x0000 to ox2000 to mirror addresses 0x0000 to 0x0800.
	if (address >= SIZE_OF_RAM) {
		return 0;  // If the address is out of bounds, default to 0.
	}
	uint16_t adjustedAddress = address % SIZE_OF_RAM;  // Mirror all addresses > 0x0800.
	return Memory::setByte(adjustedAddress, value);
}


