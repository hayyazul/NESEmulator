#include "ppu.h"

#include "../globals/helpers.hpp"
#include "../loadingData/loadPalette.hpp"
#include <iomanip>
#include <iostream>

PPU::PPU() : 
	VRAM(nullptr), 
	CHRDATA(nullptr), 
	OAM(Memory{0x100}),
	paletteControl(Memory{0x20}),
	registers(PPURegisters()), 
	cycleCount(0),
	PPUDATABuffer(0),
	w(0),
	t(0),
	v(0),
	x(0),
	requestingOAMDMA(false),
	dmaPage(0),
	ioBus(0),
	graphics(nullptr),
	attributeShiftRegister(0),
	patternShiftRegisterLow(0),
	patternShiftRegisterHigh(0),
	paletteMap(loadPalette("resourceFiles/2C02G_wiki.pal"))
{
	this->databus.attachPalette(&paletteControl);
}

PPU::PPU(Memory* VRAM, Memory* CHRDATA) :
	VRAM(VRAM),
	CHRDATA(CHRDATA),
	OAM(Memory{ 0x100 }),
	paletteControl(Memory{ 0x20 }),
	registers(PPURegisters()),
	cycleCount(0),
	PPUDATABuffer(0),
	w(0),
	t(0),
	v(0),
	x(0),
	requestingOAMDMA(false),
	dmaPage(0),
	ioBus(0),
	graphics(nullptr),
	attributeShiftRegister(0),
	patternShiftRegisterLow(0),
	patternShiftRegisterHigh(0),
	paletteMap(loadPalette("resourceFiles/2C02G_wiki.pal"))
{
	this->databus.attachPalette(&paletteControl);
}

PPU::~PPU() {}

void PPU::attachGraphics(Graphics* graphics) {
	this->graphics = graphics;
}

void PPU::attachVRAM(Memory* vram) {
	this->VRAM = vram;
	this->databus.attachVRAM(vram);
}

void PPU::attachCHRDATA(Memory* chrdata) {
	this->CHRDATA = chrdata;
	this->databus.attachCHRDATA(chrdata);
}

void PPU::executePPUCycle() {
	this->updatePPUSTATUS();

	if (this->inRendering()) {
		this->updateRenderingRegisters();
	}

	int currentLine = this->getLineOn();
	if (currentLine >= VISIBLE_LINE && currentLine < POST_RENDER_LINE && getBit(this->registers.PPUMASK, 3)) {
		//this->drawPixel();
	}

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
	case(0x2003):  // OAMADDR - The addressing space for OAM is only 0x100 bytes long.
		this->registers.OAMADDR = data;
		break;
	case(0x2004):  // OAMDATA - Increments OAMADDR
		/* Note: from Nesdev:
		Writes to OAMDATA during rendering (on the pre-render line and the visible lines 0–239, 
		provided either sprite or background rendering is enabled) do not modify values in OAM, 
		but do perform a glitchy increment of OAMADDR, bumping only the high 6 bits 
		(i.e., it bumps the [n] value in PPU sprite evaluation – it's plausible that it could 
		bump the low bits instead depending on the current status of sprite evaluation). 
		This extends to DMA transfers via OAMDMA, since that uses writes to $2004. 
		For emulation purposes, it is probably best to completely ignore writes during rendering.
		-----
		*/
		// Following the above note's advice, this write is ignored during rendering.
		if (!this->inRendering()) {
			this->OAM.setByte(this->registers.OAMADDR, data);
			++this->registers.OAMADDR;  // OAMADDR is incremented only on writes to OAMDATA, not reads.
		}
		break;
	case(0x2005):  // PPUSCROLL - Writes to the x scroll when the write latch is 0, writes to y scroll when the latch is 1.
		// Both scrolls are located in the internal t and v register--- the lower 3 bits of the y scroll is also located in these but 
		// the lower 3 bits for the x scroll is in another internal register.
		if (!w) {  // x scroll write
			// Put the upper 5 bits of the write in the lower 5 bits of t.
			copyBits(this->t, 0, 4, (uint16_t)data, 3, 7);
			// Then put the lower 3 bits of the write into the fine x scroll register
			copyBits(this->x, 0, 2, data, 0, 2);
		} else {  // y scroll write
			// Put the upper 5 bits of the write to bits 5 through 9 inclusive of t.
			copyBits(this->t, 5, 9, (uint16_t)data, 3, 7);
			// Then out the lowest 3 of the write to the highest 3 of t.
			copyBits(this->t, 12, 14, (uint16_t)data, 0, 2);
		}
		// NOTE: I do not know if this is the case, but I am copying t over to v on the second write.
		if (w) {
			this->v = this->t;
		}

		w = !w;
		break;
	case(0x2006):  // PPUADDR // TODO: there is a quirk regarding internal-t I don't fully understand yet relating to PPUADDR; read nesdev for more info.
		// Here, the internal register 'w' tracks whether we are writing the first (w = 0) or second (w = 1) 
		// byte to PPUADDR. Note that unlike most >8-bit values in the NES, these writes operate on a
		// big-endian basis.
		oldValue = this->registers.PPUADDR & (w * 0xff | !w * 0x3f00);  // Gets first (upper) 6 bits or last (lower) 8 bits, depending on the value of w.
		this->registers.PPUADDR &= w * 0x3f00 | !w * 0xff;  // Clear first (upper) 6 bits or last (lower) 8 bits, depending on the value of w.
		
		this->registers.PPUADDR += static_cast<uint16_t>(data) << (!w * 8);  // NOTE: I think I shouldn't make it add; rather, set the appropriate bits or set those bits to 0 before adding.
		this->registers.PPUADDR &= 0x7fff;  // Clear the 16th bit because PPUADDR is 15 bits, not 16. (though the last bit is unused for addressing)
		
		this->t = this->registers.PPUADDR;  // Copy this over to the internal register.
		if (w) {  // If we are on the second write, copy from the t internal register to the v internal register.
			this->v = this->t;
		}

		w = !w;  // NOTE: This changes 'w' from first to second byte write; I don't know if writes to 0x2006 should change second to first byte write too. For now it does.
		break;
	case(0x2007):  // PPUDATA
		// Modify VRAM.
		//oldValue = this->getByte(this->registers.PPUADDR);
		oldValue = this->databus.write(this->registers.PPUADDR, data);
		// Now we increment PPUADDR by 32 if bit 2 of PPUCTRL is set (the nametable is 32 bytes long, so this essentially goes down).
		// Otherwise, we increment PPUADDR by 1 (going right).
		this->registers.PPUADDR += 1 << (5 * getBit(this->registers.PPUCTRL, 2));
		break;
	case(0x4014):  // OAMDMA  // TODO: Very important TODO; a write to this address makes the CPU do a lot of stuff.
		// It essentially copies over a page of memory from the CPU into OAM. This process:
		// 1. Takes 513-514 cycles 
		// 2. suspends the CPU for those amount of cycles.

		/* NOTE: The following are some pieces of information which might be relevant:
		 - OAM DMA will copy from the page most recently written to $4014. This means that read-modify-write instructions such as INC $4014, which are able to 
		perform a second write before the CPU can be halted, will copy from the second page written, not the first.	
		
		 - OAM DMA has a lower priority than DMC DMA. If a DMC DMA get occurs during OAM DMA, OAM DMA is briefly paused. (See DMC DMA during OAM DMA)
		    - Only need to worry about this when implementing the APU.
		*/
		this->requestingOAMDMA = true;
		this->dmaPage = data;
		oldValue = 0xff;  // There is no specific old value associated w/ OAMDMA. Defaut to 0xff.
		break;
	default:
		oldValue = 0;  // This condition should never occur (unless some bug in the NES game itself results in a write to a read-only register).
		break;
	}

	return oldValue;
}

uint8_t PPU::readRegister(uint16_t address) {
	// Returns I/O bus after setting some bits based on the register being read; some parts of the bus may be untouched and returned anyway (open bus).
	// Example: a read on PPUMASK, a write-only register, will result in the open bus being read.
	switch (address) {
	case(0x2002):  // PPUSTATUS
		this->w = 0;  // w is cleared upon reading PPUSTATUS.
		// When we want to return PPUSTATUS, we have to return the I/O bus w/ the last 3 bits changed based
		// on some flags.
		this->ioBus &= 0b00011111;  // First, clear out the last 3 bits of ioBus.
		this->ioBus += this->registers.PPUSTATUS;  // Then, add the last 3 bits of PPUSTATUS to it (the other btis in PPUSTATUS should be 0).
		break;
	case(0x2004):  // OAMDATA
		this->ioBus = this->OAM.getByte(this->registers.OAMADDR);  // Simply gets the value from OAM at OAMADDR.
		break;
	case(0x2007):  // PPUDATA
		// Instead of returning the value at the given address, we actually return a value in a buffer;
		// we then update the buffer with the value at the given address. This effectively delays PPUDATA
		// reads by 1.
		this->ioBus = this->PPUDATABuffer;  // NOTE: There is some open-bus behavior when reading from palette RAM, however I do not know enough to emulate it at the moment.
		this->PPUDATABuffer = this->databus.read(this->registers.PPUADDR);
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

bool PPU::reqeuestingDMA() {
	if (this->requestingOAMDMA) { 
		this->requestingOAMDMA = false;  // Remember to set the request to false when it is true.
		return true;
	}
	return false;
}

uint8_t PPU::getDMAPage() const {
	return this->dmaPage;
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


	return beganVblank;
}

bool PPU::reachedPrerender() const {
	return false;
}

bool PPU::inRendering() const {
	// The PPU is rendering if 1. either background OR sprite rendering is on, 2. it is inbetween scanlines 0 and 239 inclusive.
	bool backgroundRendering = getBit(this->registers.PPUMASK, 3);
	bool spriteRendering = getBit(this->registers.PPUMASK, 4);
	
	int scanline = this->getLineOn();
	bool onRenderLines = scanline >= VISIBLE_LINE && scanline <= LAST_RENDER_LINE;

	return (backgroundRendering || spriteRendering) && onRenderLines;
}

void PPU::updatePPUSTATUS() {  // TODO: Implement sprite overflow and sprite 0 hit flags.
	if (this->reachedVblank()) {
		setBit(this->registers.PPUSTATUS, 7);
	} else if (this->reachedPrerender()) {
		clrBit(this->registers.PPUSTATUS, 7);
	}
}

void PPU::updateRenderingRegisters() {
	// The PPU will update differently based on the current cycle.
	uint16_t currentDot = this->getDotOn();
	if (currentDot == 0) {
		// Idle cycle.
	} else if (currentDot < 0x101) {
		// Datafetching cycles; also the drawing cycles, but drawing is handled outside of this function.
		this->performDataFetches();
	}
}

void PPU::performDataFetches() {
	// TODO: Give this variable and function a better name.
	uint8_t cycleCounter = this->cycleCount % 8;  // This variable ranges from 0 to 7 and represents cycles 8, 16, 24... 256.
	// The shifters are reloaded on cycle counter 0.
	if (cycleCounter == 0) {
		this->patternShiftRegisterLow += this->patternLatchLow << 8;
		this->patternShiftRegisterHigh += this->patternLatchHigh << 8;
		this->attributeShiftRegister = this->attributeLatch;
	}

	const uint16_t FIRST_NAMETABLE_ADDR = 0x2000, FIRST_PATTERN_TABLE_ADDR = 0x0000;
	uint16_t addr;  // Variable to hold addresses which may be used. NOTE: Might be removed.
	uint16_t a, b, c;
	// Now, we will load the latches every other cycle.
	switch (cycleCounter) {
	case(1):  // Fetching nametable byte.
		// Using the internal v register to define the scroll.
		// NOTE: For now, we are only rendering the first nametable w/o proper mirroring or any scrolling.
		addr = FIRST_NAMETABLE_ADDR + getBits(this->v, 0, 4) + getBits(this->v, 5, 9) * 32;
		a = getBits(this->v, 0, 4);
		b = getBits(this->v, 5, 9);
		b >>= 4;
		c = b * 32;
		addr = FIRST_NAMETABLE_ADDR + a + c;
		this->nametableByteLatch = this->databus.read(addr);
		// Increment the v register; this has the effect of incrementing coarse X scroll and coarse Y scroll if X's overflows.
		if ((this->v & 0b1111111111) == 0b1111111111) {
			this->v ^= 0b1111111111;
		} else {
			++this->v;
		}
		
		break;
	case(3):  // Fetching attribute table byte.
		addr = FIRST_NAMETABLE_ADDR + 0x3c0; // 0x3c0 = Size of nametable; after this is the attribute table.
		--this->v;
		a = getBits(this->v, 0, 4);
		b = getBits(this->v, 5, 9) >> 5;
		// a and b form the x, y coordinate for a nametable tile.
		++this->v;

		if (b == 1) {
			c = 0;
		}
		addr += (a / 4 + (8 * b));  // Offset the address given what part of the nametable we are 

		this->attributeLatch = this->databus.read(addr);  // TODO: Debug; I think there is a bug here but it isn't clear.
		break;
	case(5):  // Fetching pattern table tile low.
		// Using the nametable byte, we will grab the associated pattern.
		// NOTE: For now, we will use the first pattern table.
		addr = FIRST_PATTERN_TABLE_ADDR + this->nametableByteLatch * 16;  // Each pattern is 16 bytes large.
		// We will get a different pair of bytes from the pattern depending on the current line (e.g. get 1st pair on line 0, 2nd pair on line 1...).
		a = this->getLineOn();
		b = a % 240;
		c = 2 * b;
		if (addr != 0x240) {
			c = 0;
		}
		addr += this->getLineOn() % 240;  // Selecting the line.
		this->patternLatchLow = this->databus.read(addr);
		break;
	case(7):  // Fetching pattern table tile high.
		addr = FIRST_PATTERN_TABLE_ADDR + this->nametableByteLatch * 16;  // Each pattern is 16 bytes large.
		addr += this->getLineOn() % 240;  // Selecting the line.
		this->patternLatchHigh = this->databus.read(addr + 0x8);  // The upper bits are offset by 8 from the low bits.
		break;
	default:
		break;
	}
}

int PPU::getLineOn() const {
	int lineOn = (this->cycleCount / PPU_CYCLES_PER_LINE) % TOTAL_LINES;
	return lineOn;
}

int PPU::getDotOn() const {
	return this->cycleCount % PPU_CYCLES_PER_LINE;  // From what I read so far, I think cycles 1-256 (0BI) draw each dot, so I will use that to determine the dot we are on.
}

void PPU::drawPixel() {
	if (this->graphics == nullptr) {  // If we are not given a graphics object, do not attempt to draw.
		return;
	}

	// Now, figure out the color we need to draw.
	uint16_t colorKey;
	// First, copy the emphasis values from PPUMASK to the color key.
	copyBits(colorKey, 6, 8, (uint16_t)this->registers.PPUMASK, 5, 7);

	// Now we have to find the color index for this pixel. TODO.


	unsigned int dotX = this->getDotOn(), dotY = this->getLineOn();
	if (dotX < 0x100) {  // Do not draw past dot 255.
		// NOTE: Temporary solution; I am not sure why but using Graphics::drawPixel results in a slight barber pole effect.
		this->graphics->drawSquare(this->paletteMap.at(0), dotX, dotY, 1);
	}
}
