#pragma once

#include "../ppu/ppu.h"

#include <array>

struct PPUActions;

// Struct for some given sprite data.
struct SpriteData {
	uint8_t x, y, pattern, attribute;

	SpriteData();
	~SpriteData();
};

// Size and dimensions of a single nametable.
constexpr uint16_t TABLE_SIZE_IN_BYTES = 0x400;
constexpr unsigned int TABLE_WIDTH = 32;  // In tiles.
constexpr unsigned int TABLE_HEIGHT = 30;  // In tiles.

// Size and dimensions of a single pattern table.
constexpr uint16_t PATTERN_TABLE_SIZE_IN_BYTES = 0x1000;
// Size and dimensions of a single pattern.
constexpr uint8_t PATTERN_SIZE_IN_BYTES = 0x10;
// Size of RAM Palette
constexpr uint8_t PALETTE_RAM_SIZE_IN_BYTES = 0x20;

constexpr uint8_t MAX_SPRITE_COUNT = 0x40;

class PPUDebug : public PPU {
public:
	PPUDebug();
	PPUDebug(Memory* VRAM, Memory* CHRDATA);
	~PPUDebug();

	// Debug Methods

	// Displays the nametable and its attribute table from VRAM using the given table id; displays nothing upon
	// invalid input.
	void displayNametable(int table = 0) const;

	std::array<uint8_t, TABLE_SIZE_IN_BYTES> getNametable(int table = 0) const;

	// There are two pattern tables; one from 0x0000 to 0x0fff, the other from 0x1000 to 0x1fff.
	void displayPattern(uint8_t pattern, bool patternTable = 0) const;

	// MINOR TODO: standardize table-patternId ordering in args.

	std::array<uint8_t, PATTERN_TABLE_SIZE_IN_BYTES> getPatternTable(bool table = 0) const;

	std::array<uint8_t, PATTERN_SIZE_IN_BYTES> getPattern(uint8_t patternId, bool table = 0) const;

	std::array<uint8_t, PALETTE_RAM_SIZE_IN_BYTES> getPalette();

	//std::array<SpriteData, MAX_SPRITE_COUNT> getSprites() const;  // Fetches sprites from primary OAM.
	// Displays a sprite at an arbitrary location in an unemulated fashion. You may specify an x or a y, otherwise it will use the sprite's information to determine its location.
	void displaySprite(int spriteIdx, int x = -1, int y = -1, bool patternTable = 0);

	// Displays sprites where they are supposed to be offset by given x and y inputs (w/ defaults of 0). Only displays secondaryOAM.
	void displayVisibleSprites(int x = 0, int y = 0);

	void dumpOAMData(unsigned int lineSize = 4) const;  // Prints out OAM bytes in a string of bytes.

private:

	void displaySprite(SpriteData spriteData, int x, int y);
};