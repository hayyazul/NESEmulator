#include "ppu.h"

#include <iostream>
#include "../globals/helpers.hpp"

PPU::PPU() : VRAM(nullptr), registers(nullptr) {}

PPU::~PPU() {}

void PPU::attach(PPURegisters* registers) {
	this->registers = registers;
}

void PPU::attach(Memory* vram) {
	this->VRAM = vram;
}

void PPU::executePPUCycle() {
}

void PPU::displayNametable(int table) {
	if (table < 0 || table > 3) {
		return;
	}

	const int TABLE_WIDTH = 32;  // In tiles.
	const int TABLE_HEIGHT = 30;  // In tiles.
	const uint16_t TABLE_SIZE_IN_BYTES = 0x400;

	uint16_t startAddr = TABLE_SIZE_IN_BYTES * table;

	for (uint16_t i = startAddr; i < startAddr + TABLE_SIZE_IN_BYTES; ++i) {
		if (i % TABLE_WIDTH == 0) {
			std::cout << std::endl;
		}
		std::cout << displayHex(this->VRAM.getByte(i), 2) << ' ';
	}
}
