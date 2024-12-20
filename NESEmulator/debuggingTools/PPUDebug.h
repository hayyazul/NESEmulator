#pragma once

#include "../ppu/ppu.h"

#include <array>

struct PPUActions {
	// TODO
};

// Size and dimensions of a single nametable.
constexpr uint16_t TABLE_SIZE_IN_BYTES = 0x400;
constexpr unsigned int TABLE_WIDTH = 32;  // In tiles.
constexpr unsigned int TABLE_HEIGHT = 30;  // In tiles.

class PPUDebug : public PPU {
public:
	PPUDebug();
	PPUDebug(Memory* VRAM, Memory* CHRDATA);
	~PPUDebug();

	// Debug Methods

	// Displays the nametable and its attribute table from VRAM using the given table id; displays nothing upon
	// invalid input.
	void displayNametable(int table = 0);

	std::array<uint8_t, TABLE_SIZE_IN_BYTES> getNametable(int table = 0) const;

private:
};