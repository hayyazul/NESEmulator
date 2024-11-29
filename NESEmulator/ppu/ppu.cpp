#include "ppu.h"

#include <iostream>
#include <iomanip>
#include "../globals/helpers.hpp"

PPU::PPU() : 
	VRAM(nullptr), 
	CHRDATA(nullptr), 
	registers(PPURegisters()), 
	cycleCount(0),
	PPUDATABuffer(0),
	w(0)
{}

PPU::PPU(Memory* VRAM, Memory* CHRDATA) :
	VRAM(VRAM),
	CHRDATA(CHRDATA),
	registers(PPURegisters()),
	cycleCount(0),
	PPUDATABuffer(0),
	w(0)
{}

PPU::~PPU() {}

void PPU::attach(Memory* vram) {
	this->VRAM = vram;
}

void PPU::attachCHRDATA(Memory* chrdata) {
	this->CHRDATA = chrdata;
}

void PPU::executePPUCycle() {
	++this->cycleCount;
}


void PPU::writeToRegister(uint16_t address, uint8_t data) {
	switch (address) {
	case(0x2000):  // TODO: ignore writes to this until first pre-render scanline.
		this->registers.PPUCTRL = data;
		break;
	case(0x2006):  // TODO: there is a quirk regarding internal-t I don't fully understand yet relating to PPUADDR; read nesdev for more info.
		// Here, the internal register 'w' tracks whether we are writing the first (w = 0) or second (w = 1) 
		// byte to PPUADDR. Note that unlike most >8-bit values in the NES, these writes operate on a
		// big-endian basis.
		this->registers.PPUADDR += static_cast<uint16_t>(data) << (~w * 8);
		this->registers.PPUADDR &= 0x3fff;  // Clear the last 2 bits because PPUADDR is 14 bits, not 16. 
		break;
	case(0x2007):
		// Modify VRAM.
		this->VRAM->setByte(this->registers.PPUADDR, data);
		// Now we increment PPUADDR by 32 if bit 2 of PPUCTRL is set (the nametable is 32 bytes long, so this essentially goes down).
		// Otherwise, we increment PPUADDR by 1 (going right).
		this->registers.PPUADDR += 1 << (5 * getBit(this->registers.PPUCTRL, 2));
		break;
	default:
		break;
	}
}

uint8_t PPU::readRegister(uint16_t address) {
	uint8_t returnValue;
	switch (address) {
	case(0x2002):
		this->w = 0;  // w is cleared upon reading PPUSTATUS.
		return this->registers.PPUSTATUS;
		break;
	case(0x2007):
		// Instead of returning the value at the given address, we actually return a value in a buffer;
		// we then update the buffer with the value at the given address. This effectively delays PPUDATA
		// reads by 1.
		returnValue = this->PPUDATABuffer;
		this->PPUDATABuffer = this->VRAM->getByte(this->registers.PPUADDR);
		// Now we increment PPUADDR by 32 if bit 2 of PPUCTRL is set (the nametable is 32 bytes long, so this essentially goes down).
		// Otherwise, we increment PPUADDR by 1 (going right).
		this->registers.PPUADDR += 1 << (5 * getBit(this->registers.PPUCTRL, 2));
		
		return returnValue;
		break;
	default:
		return 0;
		break;
	}
}

bool PPU::requestingNMI() const {
	// Renders 262 scanlines, each scanline takes 341 PPU cycles.
	// Since VBlank happens at scanline 240 (0-indexed), we will request an NMI whenever cycleCount % (262 * 341) == 240 * 341
	// TODO: clean up this code.
	const int PPU_CYCLES_BETWEEN_VBLANKS = 262 * 341;
	const int VBLANK_FIRST_CYCLE_COUNT = 240 * 341;
	bool c = this->cycleCount % PPU_CYCLES_BETWEEN_VBLANKS == VBLANK_FIRST_CYCLE_COUNT;
	if (c) {
		int b = 0;
	}
	bool a = (getBit(this->registers.PPUCTRL, 7)) && c;
	if (a) {
		int b = 0;
	}
	return a;
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
		std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)this->VRAM->getByte(i) << ' ';
	}
}
