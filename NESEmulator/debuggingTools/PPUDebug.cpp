#include "PPUDebug.h"

#include "../globals/helpers.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

PPUDebug::PPUDebug() : PPU() {
}

PPUDebug::PPUDebug(Memory* VRAM, Memory* CHRDATA) : PPU(VRAM, CHRDATA) {
}

PPUDebug::~PPUDebug()
{
}

PPUInternals PPUDebug::getInternals() const {
	// TODO: Complete implementation.
	PPUInternals ppuInternals;
	ppuInternals.backgroundShiftRegisters = this->backgroundShiftRegisters;
	ppuInternals.latches = this->latches;
	ppuInternals.spriteShiftRegisters = this->spriteShiftRegisters;
	ppuInternals.x = this->x;
	ppuInternals.t = this->t;
	ppuInternals.v = this->v;
	ppuInternals.w = this->w;
	ppuInternals.control = this->control;
	ppuInternals.mask = this->mask;
	ppuInternals.status = this->status;
	ppuInternals.beamPos = this->beamPos;
	ppuInternals.cycleCount = this->cycleCount;
	ppuInternals.frameCount = this->frameCount;
	ppuInternals.paletteControl = this->paletteControl;
	ppuInternals.OAM = this->OAM;
	ppuInternals.secondaryOAM = this->secondaryOAM;
	ppuInternals.spriteEvalCycle = this->spriteEvalCycle;
	ppuInternals.requestingOAMDMA = this->requestingOAMDMA;
	ppuInternals.dmaPage = this->dmaPage;
	ppuInternals.OAMAddr = this->OAMAddr;
	ppuInternals.PPUDATABuffer = this->PPUDATABuffer;
	ppuInternals.ioBus = this->ioBus;
	ppuInternals.VRAM = (*this->VRAM);

	return ppuInternals;
}

bool PPUDebug::loadInternals(PPUInternals ppuInternals) {
	// TODO: Complete implementation.
	this->backgroundShiftRegisters = ppuInternals.backgroundShiftRegisters;
	this->latches = ppuInternals.latches;
	this->spriteShiftRegisters = ppuInternals.spriteShiftRegisters;
	this->x = ppuInternals.x;
	this->t = ppuInternals.t;
	this->v = ppuInternals.v;
	this->w = ppuInternals.w;
	this->control = ppuInternals.control;
	this->mask = ppuInternals.mask;
	this->status = ppuInternals.status;
	this->beamPos = ppuInternals.beamPos;
	this->cycleCount = ppuInternals.cycleCount;
	this->frameCount = ppuInternals.frameCount;
	this->paletteControl = ppuInternals.paletteControl;
	this->OAM = ppuInternals.OAM;
	this->secondaryOAM = ppuInternals.secondaryOAM;
	this->spriteEvalCycle = ppuInternals.spriteEvalCycle;
	this->requestingOAMDMA = ppuInternals.requestingOAMDMA;
	this->dmaPage = ppuInternals.dmaPage;
	this->OAMAddr = ppuInternals.OAMAddr;
	this->PPUDATABuffer = ppuInternals.PPUDATABuffer;
	this->ioBus = ppuInternals.ioBus;
	*this->VRAM = ppuInternals.VRAM;

	return true;
}

PPUPosition PPUDebug::getPosition() const {
	return this->beamPos;
}

void PPUDebug::displayNametable(int table) const {
	if (table < 0 || table > 3) {
		return;
	}

	uint16_t startAddr = TABLE_SIZE_IN_BYTES * table;

	for (uint16_t i = startAddr; i < startAddr + TABLE_WIDTH * TABLE_HEIGHT; ++i) {
		if (i % TABLE_WIDTH == 0) {
			std::cout << std::endl;
		}
		std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)this->VRAM->getByte(i) << ' ';
	}
	std::cout << std::endl << "--- Attribute Table ---" << std::endl;
	for (uint16_t i = startAddr + TABLE_WIDTH * TABLE_HEIGHT; i < startAddr + TABLE_SIZE_IN_BYTES; ++i) {
		if (i % 8 == 0) {
			std::cout << std::endl;
		}
		std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)this->VRAM->getByte(i) << ' ';
	}
}

std::array<uint8_t, TABLE_SIZE_IN_BYTES> PPUDebug::getNametable(int table) const {
	std::array<uint8_t, TABLE_SIZE_IN_BYTES> nametableData{};
	uint16_t startAddr = TABLE_SIZE_IN_BYTES * table;  // First get the starting address of the table.

	for (uint16_t i = 0; i < TABLE_SIZE_IN_BYTES; ++i) {
		nametableData.at(i) = this->VRAM->getByte(startAddr + i);
	}

	return nametableData;
}

void PPUDebug::displayPattern(uint8_t patternId, bool patternTable) const {
	// First, get the pattern.
	std::array<uint8_t, PATTERN_SIZE_IN_BYTES> pattern = this->getPattern(patternId, patternTable);

	unsigned int pixel;  // Can be 0, 1, 2, or 3.
	uint8_t lowerBitPlane, upperBitPlane;
	// Iterate through the 8 byte-pairs.
	for (unsigned int i = 0; i < 8; ++i) {
		lowerBitPlane = pattern.at(i);
		upperBitPlane = pattern.at(i + 0x8);
		// Then, iterate through the 8 bit-pairs in these.
		for (int j = 7; j >= 0; --j) {
			pixel = getBit(lowerBitPlane, j) + (getBit(upperBitPlane, j) << 1);

			// Lastly, display the pixel.
			std::cout << std::dec << pixel << ' ';
		}

		// Go to the next row after finishing the current one.
		std::cout << std::endl;
	}
}

std::array<uint8_t, PATTERN_TABLE_SIZE_IN_BYTES> PPUDebug::getPatternTable(bool table) const {
	// First, get the address of the pattern table.
	uint16_t patternTableAddr = (uint16_t)table << 12;
	uint16_t patternAddr;
	
	std::array<uint8_t, PATTERN_TABLE_SIZE_IN_BYTES> patternTableData{};

	unsigned int pixel;  // Can be 0, 1, 2, or 3.
	uint8_t bitPlane;  // A row of 8 bits indicating the color of a pixel (can either be a low or high bit; two bit planes decide the color).
	// Iterate through the 256 patterns.
	for (unsigned int i = 0; i < 256; ++i) {
		patternAddr = patternTableAddr + (i * PATTERN_SIZE_IN_BYTES);
		for (unsigned int j = 0; j < 16; ++j) {
			bitPlane = this->CHRDATA->getByte(patternAddr + j);
			auto a = i * 16 + j;
			patternTableData.at(i * 16 + j) = bitPlane;
		}
	}

	return patternTableData;
}

std::array<uint8_t, PATTERN_SIZE_IN_BYTES> PPUDebug::getPattern(uint8_t patternId, bool table) const {
	uint16_t patternTableAddr = (uint16_t)table << 12;
	uint16_t patternAddr = patternTableAddr + (patternId * PATTERN_SIZE_IN_BYTES);
	std::array<uint8_t, PATTERN_SIZE_IN_BYTES> pattern;

	// Loop through the rows of the pattern at the given address.
	uint8_t bitPlane;  // A row of 8 bits indicating the color of a pixel (can either be a low or high bit; two bit planes decide the color).
	for (unsigned int i = 0; i < 16; ++i) {
		bitPlane = this->CHRDATA->getByte(patternAddr + i);
		pattern.at(i) = bitPlane;
	}

	return pattern;
}

std::array<uint8_t, PALETTE_RAM_SIZE_IN_BYTES> PPUDebug::getPalette() {
	const uint16_t paletteAddress = 0x3f00;
	std::array<uint8_t, PALETTE_RAM_SIZE_IN_BYTES> paletteData;
	for (uint8_t i = 0; i < PALETTE_RAM_SIZE_IN_BYTES; ++i) {
		paletteData.at(i) = this->databus.read(paletteAddress + i);
	}

	return paletteData;
}

void PPUDebug::displaySprite(int spriteIdx, int x, int y, bool patternTable) {
	// First get the sprite data.
	if (spriteIdx > 0x3f || spriteIdx < 0) {
		spriteIdx = 0;
	}
	SpriteData spriteData;

	spriteData.y = this->OAM.getByte(4 * spriteIdx);
	spriteData.pattern = this->OAM.getByte(4 * spriteIdx + 1);
	spriteData.attribute = this->OAM.getByte(4 * spriteIdx + 2);
	spriteData.x = this->OAM.getByte(4 * spriteIdx + 3);

	x = x < 0 ? spriteData.x : x;
	y = y < 0 ? spriteData.y : y;

	this->displaySprite(spriteData, x, y);

}

void PPUDebug::displayVisibleSprites(int x, int y) {
	for (int i = 0; i < 0x100; i += 4) {
		SpriteData spriteData;
		spriteData.y = this->OAM.getByte(i);
		spriteData.pattern = this->OAM.getByte(i + 1);
		spriteData.attribute = this->OAM.getByte(i + 2);
		spriteData.x = this->OAM.getByte(i + 3);

		if (i == 0 && spriteData.y != 0x7f) {
			int _ = 0;
		}
		
		this->displaySprite(spriteData, spriteData.x + x, spriteData.y + y);
	}
}

void PPUDebug::dumpOAMData(unsigned int lineSize) const {
	const int sizeOfOAMData = 0x100;  // There are 256 bytes in OAM data representing 64 sprites each defined w/ 4 bytes.
	uint8_t OAMByte;
	for (int i = 0; i < sizeOfOAMData; ++i) {
		if (i % lineSize == 0 && i) {
			std::cout << std::endl;
		}
		OAMByte = this->OAM.getByte(i);
		std::cout << displayHex(OAMByte, 2) << ' ';	
	}
}

void PPUDebug::displaySprite(SpriteData spriteData, int x, int y) {
	if (spriteData.y >= 240) {  // Don't attempt to display sprites which are beyond the visible region.
		return;
	}

	// Now to display. Before we can, we must extract the pattern data.
	uint16_t patternAddr = (0x1000 * getBit(this->control, 3));
	patternAddr += 16 * spriteData.pattern;
	std::array<uint8_t, PATTERN_SIZE_IN_BYTES / 2> patternLow;
	std::array<uint8_t, PATTERN_SIZE_IN_BYTES / 2> patternHigh;
	uint8_t attribute = getBits(spriteData.attribute, 0, 1);
	bool flipHoriz = getBit(spriteData.attribute, 6);
	bool flipVert = getBit(spriteData.attribute, 7);

	for (int i = 0, j = 8; i < 8; ++i, ++j) {
		patternLow.at(i) = this->databus.read(patternAddr + i);
		patternHigh.at(i) = this->databus.read(patternAddr + j);
	}

	// Now we display the sprite.
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {

			// Copy and pasted from PPU::drawPixel
			uint16_t colorKey = 0;
			copyBits(colorKey, 6, 8, (uint16_t)this->mask, 5, 7);

			const uint16_t spritePaletteAddress = 0x3f10;  // The starting address for the background palette.
			uint16_t addr = spritePaletteAddress;

			int rowIdx = flipVert ? 7 - i : i;
			int colIdx = flipHoriz ? j : 7 - j;

			uint8_t highBit = getBit(patternHigh.at(rowIdx), colIdx);
			highBit <<= 1;
			uint8_t lowBit = getBit(patternLow.at(rowIdx), colIdx);

			addr += highBit + lowBit;
			// Indexing which palette we want.
			addr += 4 * attribute;

			// Now, using this addr, we will get the color located at that addr.
			auto f = this->databus.read(addr);
			colorKey |= f;

			if (highBit + lowBit == 0) {  // If it is transparent, do not draw.
				continue;
			}
			this->graphics->drawSquare(this->paletteMap.at(colorKey), x + j, y + i, 1);
		}
	}
}

SpriteData::SpriteData() : x(0xff), y(0xff), pattern(0xff), attribute(0xff) {}
SpriteData::~SpriteData() {}

PPUInternals::PPUInternals() : VRAM(VRAM_SIZE) {}

PPUInternals::~PPUInternals() {}

std::string PPUInternals::getSerialFormat() const {
	std::stringstream preSerializedStr;

	preSerializedStr << this->getBGLatchSerialStr();
	preSerializedStr << this->getBGShiftSerialStr();
	preSerializedStr << this->getSPShiftSerialStr();
	preSerializedStr << this->getBeamPosSerialStr();
	preSerializedStr << "PALETTE: " << this->paletteControl.getDataAsStr() << '\n';
	preSerializedStr << "OAM: " << this->OAM.getDataAsStr() << '\n';
	preSerializedStr << "OAM2: " << this->secondaryOAM.getDataAsStr() << '\n';
	preSerializedStr << this->getSpriteEvalSerialStr();
	preSerializedStr << "OAMDMAREQ: " << (unsigned long long)this->requestingOAMDMA << '\n';
	preSerializedStr << "DMAPAGE: " << (unsigned long long)this->dmaPage << '\n';
	preSerializedStr << "W: " << (unsigned long long)this->w << '\n';
	preSerializedStr << "V: " << (unsigned long long)this->v << '\n';
	preSerializedStr << "T: " << (unsigned long long)this->t << '\n';
	preSerializedStr << "X: " << (unsigned long long)this->x << '\n';
	preSerializedStr << "CTRL: " << (unsigned long long)this->control << '\n';
	preSerializedStr << "MASK: " << (unsigned long long)this->mask << '\n';
	preSerializedStr << "STATUS: " << (unsigned long long)this->status << '\n';
	preSerializedStr << "OAMADDR: " << (unsigned long long)this->OAMAddr << '\n';
	preSerializedStr << "PPUDATABUF: " << (unsigned long long)this->PPUDATABuffer << '\n';
	preSerializedStr << "IOBUS: " << (unsigned long long)this->ioBus << '\n';
	preSerializedStr << "VRAM: " << this->VRAM.getDataAsStr() << '\n';
	preSerializedStr << "CYCLES: " << (unsigned long long)this->cycleCount << '\n';
	preSerializedStr << "FRAME: " << (unsigned long long)this->frameCount << '\n';

	return preSerializedStr.str();
}

void PPUInternals::deserializeData(std::stringstream& data) {
	/* Sample input:
BGLATCHES: 0 0 1 0 36
BGSHIFT: 0 0 127 0
SPSHIFT: 65280 65280 2833 65280 65280 2833 65280 65280 2833 65280 65280 2833 65280 65280 2833 65280 65280 2833 65280 65280 2833 65280 65280 2833
BEAMPOS: 156 115
PALETTE: 15 44 56 18 15 39 39 39 15 48 48 48 15 0 0 0 0 37 0 0 0 0 0 0 0 0 0 0 0 0 0 0
OAM: 127 162 0 56 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0
OAM2: 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255
SPEVALCYCLE: 0 1
OAMDMAREQ: 0
DMAPAGE: 2
W: 0
V: 17006
T: 0
X: 0
CTRL: 144
MASK: 30
STATUS: 0
OAMADDR: 100
PPUDATABUF: 0
IOBUS: 144
VRAM: 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 98 98 98 36 36 98 98 98 36 98 36 36 98 36 98 36 36 98 36 98 98 98 36 98 36 98 36 36 36 36 36 36 98 98 36 98 36 98 36 98 36 98 98 36 98 36 98 98 98 36 36 98 36 36 36 98 36 98 36 36 36 36 36 36 98 98 36 98 36 98 36 98 36 98 98 98 98 36 98 98 36 36 36 98 98 98 36 98 98 98 36 36 36 36 36 36 98 98 36 98 36 98 36 98 36 98 36 98 98 36 98 36 98 36 36 98 36 36 36 36 98 36 36 36 36 36 36 36 98 98 98 36 36 98 98 98 36 98 36 36 98 36 98 36 36 98 36 98 98 98 36 36 98 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 98 36 36 98 36 98 98 98 36 98 36 36 98 36 98 98 98 98 36 36 36 36 36 36 36 36 36 36 36 36 36 36 98 98 98 36 36 98 36 98 36 98 98 36 98 36 98 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 98 98 36 36 36 98 36 98 36 98 98 98 98 36 98 36 98 98 36 36 36 36 36 36 36 36 36 36 36 36 36 36 98 36 98 36 36 98 36 98 36 98 36 98 98 36 98 36 36 98 36 36 36 36 36 36 36 36 36 36 36 36 36 36 98 36 36 98 36 98 98 98 36 98 36 36 98 36 98 98 98 98 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 1 36 25 21 10 34 14 27 36 16 10 22 14 36 10 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 1 36 25 21 10 34 14 27 36 16 10 22 14 36 11 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 2 36 25 21 10 34 14 27 36 16 10 22 14 36 10 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 2 36 25 21 10 34 14 27 36 16 10 22 14 36 11 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 211 1 9 8 1 36 23 18 23 29 14 23 13 24 36 12 24 101 21 29 13 100 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 22 10 13 14 36 18 23 36 19 10 25 10 23 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 36 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 85 85 85 85 85 85 85 85 85 85 85 85 85 85 85 85 170 170 170 170 170 170 170 170 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
CYCLES: 500021
FRAME: 5
	*/
	// A map to a component enum; this is used so if I change the labels, I do not need to change the code which checks for the labels much.
	enum Component {
		BGLATCHES,
		BGSHIFT,
		SPSHIFT,
		BEAMPOS,
		PALETTE,
		OAM,
		SPEVALCYCLE,
		OAMDMAREQ,
		DMAPAGE,
		W,
		V,
		T,
		X,
		CTRL,
		MASK,
		STATUS,
		OAMADDR,
		PPUDATABUF,
		IOBUS,
		VRAM,
		CYCLES,
		FRAME
	};
	const std::map<std::string, Component> LABEL_TO_COMPONENT = {
		{"BGLATCHES:", BGLATCHES},
		{"BGSHIFT:", BGSHIFT},
		{"SPSHIFT:", SPSHIFT},
		{"BEAMPOS:", BEAMPOS},
		{"PALETTE:", PALETTE},
		{"OAM:", OAM},
		{"SPEVALCYCLE:", SPEVALCYCLE},
		{"OAMDMAREQ:", OAMDMAREQ},
		{"DMAPAGE:", DMAPAGE},
		{"W:", W},
		{"V:", V},
		{"T:", T},
		{"X:", X},
		{"CTRL:", CTRL},
		{"MASK:", MASK},
		{"STATUS:", STATUS},
		{"OAMADDR:", OAMADDR},
		{"PPUDATABUF:", PPUDATABUF},
		{"IOBUS:", IOBUS},
		{"VRAM:", VRAM},
		{"CYCLES:", CYCLES},
		{"FRAME:", FRAME}
	};

	// First, get all the data into a string vector representing the datapoints.
	std::vector<std::string> directMemberDatapoints;  // Internals which are stored as primitives e.g. internal registers.
	std::vector<std::string> BGShiftDatapoints;
	std::vector<std::string> LatchDatapoints;
	std::vector<std::string> SPShiftDatapoints;

	Component componentOn = BGLATCHES;
	for (std::string datapoint; std::getline(data, datapoint, ' ');) {
		if (LABEL_TO_COMPONENT.contains(datapoint)) {
			componentOn = LABEL_TO_COMPONENT.at(datapoint);
			continue;
		}
		//datapoints.push_back(datapoint);
	}

	componentOn = BGLATCHES;
	for (std::string& datapoint : directMemberDatapoints) {
		if (LABEL_TO_COMPONENT.contains(datapoint)) {
			componentOn = LABEL_TO_COMPONENT.at(datapoint);
			continue;
		}

		unsigned long value = std::stoll(datapoint);
		// Insert the data differently given the component.
		switch (componentOn) {
		case BGLATCHES: {
			break;
		}
		case BGSHIFT: {
			break;
		}
		case SPSHIFT: {
			break;
		}
		case BEAMPOS: {
			break;
		}
		case PALETTE: {
			break;
		}
		case OAM: {
			break;
		}
		case SPEVALCYCLE: {
			break;
		}
		case OAMDMAREQ: {
			break;
		}
		case DMAPAGE: {
			break;
		}
		case W: {
			break;
		}
		case V: {
			break;
		}
		case T: {
			break;
		}
		case X: {
			break;
		}
		case CTRL: {
			break;
		}
		case MASK: {
			break;
		}
		case STATUS: {
			break;
		}
		case OAMADDR: {
			break;
		}
		case PPUDATABUF: {
			break;
		}
		case IOBUS: {
			break;
		}
		case VRAM: {
			break;
		}
		case CYCLES: {
			break;
		}
		case FRAME: {
			break;
		}
		}
	}

}

std::string PPUInternals::getBGLatchSerialStr() const {
	std::stringstream preSerializedStr;
	preSerializedStr << "BGLATCHES: " << (unsigned long long)latches.patternLatchLow;
	preSerializedStr << " " << (unsigned long long)latches.patternLatchHigh;
	preSerializedStr << " " << (unsigned long long)latches.attributeLatchLow;
	preSerializedStr << " " << (unsigned long long)latches.attributeLatchHigh;
	preSerializedStr << " " << (unsigned long long)latches.nametableByteLatch << '\n';
	return preSerializedStr.str();
}

std::string PPUInternals::getBGShiftSerialStr() const {
	std::stringstream preSerializedStr;
	preSerializedStr << "BGSHIFT: " << (unsigned long long)backgroundShiftRegisters.patternShiftRegisterLow;
	preSerializedStr << " " << (unsigned long long)backgroundShiftRegisters.patternShiftRegisterHigh;
	preSerializedStr << " " << (unsigned long long)backgroundShiftRegisters.attributeShiftRegisterLow;
	preSerializedStr << " " << (unsigned long long)backgroundShiftRegisters.attributeShiftRegisterHigh << '\n';
	return preSerializedStr.str();
}

std::string PPUInternals::getSPShiftSerialStr() const {
	std::stringstream preSerializedStr;
	
	// First, we create the string w/ the first sprite shift register.
	preSerializedStr << "SPSHIFT: ";
	preSerializedStr << (unsigned long long)spriteShiftRegisters.shiftRegisters.at(0).patternShiftRegisterLow;
	preSerializedStr << " " << (unsigned long long)spriteShiftRegisters.shiftRegisters.at(0).patternShiftRegisterHigh;
	preSerializedStr << " " << (unsigned long long)spriteShiftRegisters.shiftRegisters.at(0).attributeBits;
	// Then we add in the other 7 values, now w/ the first element seperated by a comma too.
	for (int i = 1; i < 8; ++i) {
		preSerializedStr << " " << (unsigned long long)spriteShiftRegisters.shiftRegisters.at(i).patternShiftRegisterLow;
		preSerializedStr << " " << (unsigned long long)spriteShiftRegisters.shiftRegisters.at(i).patternShiftRegisterHigh;
		preSerializedStr << " " << (unsigned long long)spriteShiftRegisters.shiftRegisters.at(i).attributeBits;
	}
	preSerializedStr << '\n';

	return preSerializedStr.str();
}

std::string PPUInternals::getBeamPosSerialStr() const {
	std::stringstream preSerializedStr;
	preSerializedStr << "BEAMPOS: " << (unsigned long long)this->beamPos.scanline;
	preSerializedStr << " " << (unsigned long long)this->beamPos.dot << '\n';
	return preSerializedStr.str();
}

std::string PPUInternals::getSpriteEvalSerialStr() const {
	std::stringstream preSerializedStr;
	preSerializedStr << "SPEVALCYCLE: " << (unsigned long long)this->spriteEvalCycle.byteType;
	preSerializedStr << " " << (unsigned long long)this->spriteEvalCycle.evalState << '\n';

	return preSerializedStr.str();
}
