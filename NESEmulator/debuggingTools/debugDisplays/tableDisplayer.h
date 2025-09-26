#pragma once

#include "../../graphics/graphics.h"
#include "../PPUDebug.h"

#include <array>

// TODO: Given a PPU, display a nametable at a specified location w/ patterns (pseudocolor).
// TODO: add a 1px border around a nametable and pattern display.
// NOTE: I might just make these functions as I don't see how them being objects helps.

class PatternTableDisplayer {
public:
	/* Display rules :
	00 - Transparent (gray)
	01 - Red
	10 - Green
	11 - Blue
	*/

	PatternTableDisplayer();
	~PatternTableDisplayer();

	// Displays a pseudocolor pattern table.
	void displayPatternTable(Graphics& graphics, PPUDebug& ppu, bool table = false, unsigned int x = 0, unsigned int y = 0, unsigned int scale = 1);

	// Displays a single pattern, either when given an array of bytes defining a pattern, or a reference to a debug PPU and a pattern ID.
	void displayPattern(Graphics& graphics, std::array<uint8_t, PATTERN_SIZE_IN_BYTES> pattern, unsigned int x, unsigned int y, unsigned int scale);
	void displayPattern(Graphics& graphics, PPUDebug& ppu, uint8_t patternId, bool table, unsigned int x, unsigned int y, unsigned int scale);

private:

};

class NametableDisplayer {
public:
	NametableDisplayer();
	~NametableDisplayer();

	void displayNametable(Graphics& graphics, PPUDebug& ppu, unsigned int table = 0, unsigned int x = 0, unsigned int y = 0, unsigned int scale = 1, bool patternTable = false);

private:

	PatternTableDisplayer PTDisplayer;

	// Displays a tile based on the pattern table.

};
