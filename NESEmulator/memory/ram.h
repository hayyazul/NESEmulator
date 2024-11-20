// ram.h - The internal RAM of the NES. Note that while there is only 0x800 bytes of RAM, the first 0x2000 bytes are taken up by it (RAM is mirrored).
#pragma once

#include "memory.h"
#include <array>

constexpr int SIZE_OF_RAM = 0x800;

class RAM : public Memory {
public:
	RAM();
	~RAM();

	uint8_t getByte(uint16_t address) const override;
	uint8_t setByte(uint16_t address, uint8_t value) override;
};