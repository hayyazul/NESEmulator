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
	// Deduces what register the operation should occur on, then performs the appropriate operation.
	
	// All writes, even to read-only registers, change the I/O bus. (NOTE: doublecheck this)
	this->ioBus = data;

	uint8_t oldValue = 0;
	switch (address) {
	case(0x2000):  // PPUCTRL
		oldValue = this->registers.PPUCTRL;
		if (this->cycleCount >= PRE_RENDER_LINE * PPU_CYCLES_PER_LINE) {  // Ignore writes to PPUCTRL until pre-render line is reached.
			this->registers.PPUCTRL = data;
		}
		break;
	case(0x2001):  // PPUMASK
		this->registers.PPUMASK = data;
		break;
	case(0x2006):  // TODO: there is a quirk regarding internal-t I don't fully understand yet relating to PPUADDR; read nesdev for more info.
		// Here, the internal register 'w' tracks whether we are writing the first (w = 0) or second (w = 1) 
		// byte to PPUADDR. Note that unlike most >8-bit values in the NES, these writes operate on a
		// big-endian basis.
		oldValue = this->registers.PPUADDR & (w * 0xff | !w * 0x3f00);  // Gets first (upper) 6 bits or last (lower) 8 bits, depending on the value of w.
		this->registers.PPUADDR &= w * 0x3f00 | !w * 0xff;  // Clear first (upper) 6 bits or last (lower) 8 bits, depending on the value of w.
		
		this->registers.PPUADDR += static_cast<uint16_t>(data) << (!w * 8);  // NOTE: I think I shouldn't make it add; rather, set the appropriate bits or set those bits to 0 before adding.
		this->registers.PPUADDR &= 0x7fff;  // Clear the 16th bit because PPUADDR is 15 bits, not 16. (though the last bit is unused for addressing)

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
	// Returns I/O bus after setting some bits based on the register being read; some parts of the bus may be untouched and returned anyway (open bus).
	switch (address) {
	case(0x2002):
		this->w = 0;  // w is cleared upon reading PPUSTATUS.
		// When we want to return PPUSTATUS, we have to return the I/O bus w/ the last 3 bits changed based
		// on some flags.
		this->ioBus &= 0b00011111;  // First, clear out the last 3 bits of ioBus.
		this->ioBus += this->registers.PPUSTATUS;  // Then, add the last 3 bits of PPUSTATUS to it (the other btis in PPUSTATUS should be 0).
		break;
	case(0x2007):
		// Instead of returning the value at the given address, we actually return a value in a buffer;
		// we then update the buffer with the value at the given address. This effectively delays PPUDATA
		// reads by 1.
		this->ioBus = this->PPUDATABuffer;  // NOTE: There is some open-bus behavior when reading from palette RAM, however I do not know enough to emulate it at the moment.
		this->PPUDATABuffer = this->VRAM->getByte(this->registers.PPUADDR);
		// Now we increment PPUADDR by 32 if bit 2 of PPUCTRL is set (the nametable is 32 bytes long, so this essentially goes down).
		// Otherwise, we increment PPUADDR by 1 (going right).
		this->registers.PPUADDR += 1 << (5 * getBit(this->registers.PPUCTRL, 2));
		break;
	default:  // This catches the reads to write-only registers.
		break;
	}

	return this->ioBus;
}

bool PPU::requestingNMI() const {
	// We request an NMI when we are in Vblank AND the 7th bit in PPUCTRL is set.
	bool requestNMI = (getBit(this->registers.PPUCTRL, 7)) && this->inVblank();
	if (requestNMI) {
		int a = 0;
	}
	return requestNMI;
}



bool PPU::inVblank() const {
	// We reach Vblank when we are on dot 1 of the first VBlank line and end on dot 340 of the last Vblank line (line 260).
	int currentLine = this->getLineOn();
	bool onVblank = currentLine > FIRST_VBLANK_LINE && currentLine <= LAST_VBLANK_LINE;  // First check if we are inbetween the first and last vblank (exclusive, inclusive)
	// Return true if we know we are on vblank.
	if (onVblank) {
		return onVblank;
	}
	// If not, check if we are on the first vblank line; if so, check if we are on or past the first dot (dot 0 on the first vblank line should not count).
	onVblank = currentLine == FIRST_VBLANK_LINE && this->getDotOn() >= 1;
	return onVblank;
}

bool PPU::reachedVblank() const {
	int currentLine = this->getLineOn();
	bool beganVblank = currentLine == FIRST_VBLANK_LINE && this->getDotOn() == 1;

	auto a = this->getDotOn();  // DEBUG: Remove when done debugging vblank detection via PPUSTATUS.
	auto b = this->getLineOn();
	if (a == 1 && b == 0xf1) {
		int c = 0;
	}

	return beganVblank;
}

bool PPU::reachedPrerender() const {
	return false;
}

void PPU::updatePPUSTATUS() {  // TODO: Implement sprite overflow and sprite 0 hit flags.
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
