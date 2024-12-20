#pragma once

#include "../../graphics/graphics.h"
#include "../PPUDebug.h"

// TODO: Given a PPU, display a nametable at a specified location

class NametableDisplayer {
public:
	NametableDisplayer();
	~NametableDisplayer();

	void displayNametable(Graphics& graphics, PPUDebug& ppu, unsigned int table = 0, unsigned int x = 0, unsigned int y = 0, unsigned int scale = 1);

private:

	// Displays a tile based on the pattern table.
	void displayTile(Graphics& graphics, PPUDebug& ppu, uint8_t tileId, unsigned int x, unsigned int y, unsigned int scale);

};

