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
	w(0),
	t(0),
	v(0),
	x(0)
{}

PPU::PPU(Memory* VRAM, Memory* CHRDATA) :
	VRAM(VRAM),
	CHRDATA(CHRDATA),
	registers(PPURegisters()),
	cycleCount(0),
	PPUDATABuffer(0),
	w(0),
	t(0),
	v(0),
	x(0)
{}

PPU::~PPU() {}

void PPU::attachVRAM(Memory* vram) {
	this->VRAM = vram;
}

void PPU::attachCHRDATA(Memory* chrdata) {
	this->CHRDATA = chrdata;
}

void PPU::executePPUCycle() {
	this->updatePPUSTATUS();
	++this->cycleCount;
}


uint8_t PPU::writeToRegister(uint16_t address, uint8_t data) {
	uint8_t oldValue = 0;
	switch (address) {
	case(0x2000):
		oldValue = this->registers.PPUCTRL;
		if (this->cycleCount >= PRE_RENDER_LINE * PPU_CYCLES_PER_LINE) {  // Ignore writes to PPUCTRL until pre-render line is reached.
			this->registers.PPUCTRL = data;
		}
		break;
	case(0x2006):  // TODO: there is a quirk regarding internal-t I don't fully understand yet relating to PPUADDR; read nesdev for more info.
		// Here, the internal register 'w' tracks whether we are writing the first (w = 0) or second (w = 1) 
		// byte to PPUADDR. Note that unlike most >8-bit values in the NES, these writes operate on a
		// big-endian basis.
		oldValue = this->registers.PPUADDR & (w * 0xff | !w * 0x3f00);  // Gets first (upper) 6 bits or last (lower) 8 bits, depending on the value of w.
		this->registers.PPUADDR &= w * 0x3f00 | !w * 0xff;  // Clear first (upper) 6 bits or last (lower) 8 bits, depending on the value of w.
		
		this->registers.PPUADDR += static_cast<uint16_t>(data) << (!w * 8);  // NOTE: I think I shouldn't make it add; rather, set the appropriate bits or set those bits to 0 before adding.
		this->registers.PPUADDR &= 0x3fff;  // Clear the last 2 bits because PPUADDR is 14 bits, not 16. 

		w = !w;  // NOTE: This changes 'w' from first to second byte write; I don't know if writes to 0x2006 should change second to first byte write too. For now it does.
		break;
	case(0x2007):
		// Modify VRAM.
		oldValue = this->VRAM->getByte(this->registers.PPUADDR);
		if (data != 0x24 && data != 0x00) {
			int a = 0;
		}
		this->VRAM->setByte(this->registers.PPUADDR, data);
		// Now we increment PPUADDR by 32 if bit 2 of PPUCTRL is set (the nametable is 32 bytes long, so this essentially goes down).
		// Otherwise, we increment PPUADDR by 1 (going right).
		this->registers.PPUADDR += 1 << (5 * getBit(this->registers.PPUCTRL, 2));
		break;
	default:
		oldValue = 0;  // This condition should never occur.
		break;
	}

	return oldValue;
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
	bool a = (getBit(this->registers.PPUCTRL, 7)) && c;
	
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

bool PPU::reachedVblank() const {
	// We reach Vblank when we are on dot 1 of the VBlank line.
	bool vblankStarted = this->getLineOn() == VBLANK_LINE && this->getDotOn() == 1;
	auto a = this->getDotOn();  // DEBUG: Remove when done debugging vblank detection via PPUSTATUS.
	auto b = this->getLineOn();
	if (a == 1 && b == 0xf1) {
		int c = 0;
	}
	return vblankStarted;
}

bool PPU::reachedPrerender() const {
	return false;
}

void PPU::updatePPUSTATUS() {
	if (this->reachedVblank()) {
		setBit(this->registers.PPUSTATUS, 7);
	} else if (this->reachedPrerender()) {
		clrBit(this->registers.PPUSTATUS, 7);
	}
}

int PPU::getLineOn() const {
	int lineOn = (this->cycleCount / PPU_CYCLES_PER_LINE) % TOTAL_LINES;
	return lineOn;
}

int PPU::getDotOn() const {
	return this->cycleCount % PPU_CYCLES_PER_LINE;  // From what I read so far, I think cycles 1-256 (0BI) draw each dot, so I will use that to determine the dot we are on.
}
