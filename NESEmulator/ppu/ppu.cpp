#include "ppu.h"

#include "../globals/helpers.hpp"
#include "../loadingData/loadPalette.h"
#include <iomanip>
#include <iostream>

PPU::PPU() : 
	VRAM(nullptr), 
	CHRDATA(nullptr), 
	OAM(Memory{0x100}),
	paletteControl(Memory{0x20}),
	cycleCount(0),
	PPUDATABuffer(0),
	control(0),
	mask(0),
	status(0),
	OAMAddr(0),
	w(0),
	t(0),
	v(0),
	x(0),
	requestingOAMDMA(false),
	dmaPage(0),
	ioBus(0),
	graphics(nullptr),
	latches(),
	backgroundShiftRegisters(),
	spriteShiftRegisters(),
	paletteMap(loadPalette("resourceFiles/2C02G_wiki.pal")),
	beamPos(),
	frameCount(0), 
	spriteEvalCycle()
{
	this->databus.attachPalette(&paletteControl);
}

PPU::PPU(Memory* VRAM, Memory* CHRDATA) :
	VRAM(VRAM),
	CHRDATA(CHRDATA),
	OAM(Memory{ 0x100 }),
	paletteControl(Memory{ 0x20 }),
	cycleCount(0),
	PPUDATABuffer(0),
	control(0),
	mask(0),
	status(0),
	OAMAddr(0),
	w(0),
	t(0),
	v(0),
	x(0),
	requestingOAMDMA(false),
	dmaPage(0),
	ioBus(0),
	graphics(nullptr),
	latches(),
	backgroundShiftRegisters(),
	spriteShiftRegisters(),
	paletteMap(loadPalette("resourceFiles/2C02G_wiki.pal")),
	beamPos(),
	frameCount(0),
	spriteEvalCycle()
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
	
	// NOTE: When dealing w/ the weird bugs relating to donkey kong's erroneous display, start your investigation here.
	// Setting the address to 0x2001 seems to place the latter (pattern 0x40) in the first tile but does weird stuff w/ 0x2000.
	//this->VRAM->setByte(0x2000, 0x40);
	//this->VRAM->setByte(0x2001, 0x40);


	//if (this->beamPos.lineInRange(240, 240)) {
		//return;
	//}

	this->updatePPUSTATUS();

	if (this->v == 0x800) {
		int _ = 0;
	}

	//if (this->OAMAddr != 0)
	
	if (this->isRendering(true)) {
		this->updateRenderingRegisters();
	}
	
	if (this->beamPos.onRenderLines() && getBit(this->mask, 3)) {
		this->drawPixel();
	}

	this->updateBeamLocation();

	++this->cycleCount;
}

uint8_t PPU::writeToRegister(uint16_t address, uint8_t data) {
	// Deduces what register the operation should occur on, then performs the appropriate operation.
	
	// All writes, even to read-only registers, change the I/O bus. (NOTE: doublecheck this)
	this->ioBus = data;

	uint8_t oldValue = 0;

	// For specific behaviors, look at PPU Scrolling.
	switch (address) {
	case(0x2000):  // PPUCTRL
		oldValue = control;
		if (this->cycleCount >= PRE_RENDER_LINE * PPU_CYCLES_PER_LINE) {  // Ignore writes to PPUCTRL until pre-render line is reached.
			control = data;
		}

		copyBits(this->t, 10, 11, (uint16_t)data, 0, 1);
		break;
	case(0x2001):  // PPUMASK
		this->mask = data;
		break;
	case(0x2003):  // OAMADDR - The addressing space for OAM is only 0x100 bytes long.
		this->OAMAddr = data;
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


		if (!this->isRendering()) {
			if (this->OAMAddr == 0 && data != 0x7f) {
				int _ = 0;
			}
			else if (data == 0x7f && this->OAMAddr != 0) {
				int _ = 0;
			}

			this->OAM.setByte(this->OAMAddr, data);
			++this->OAMAddr;  // OAMADDR is incremented only on writes to OAMDATA, not reads.
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
		w = !w;
		break;
 	case(0x2006):  // PPUADDR // TODO: there is a quirk regarding internal-t I don't fully understand yet relating to PPUADDR; read nesdev for more info.
		// Here, the internal register 'w' tracks whether we are writing the first (w = 0) or second (w = 1) 
		// byte to PPUADDR. Note that unlike most >8-bit values in the NES, these writes operate on a
		// big-endian basis.
		oldValue = this->t & (w * 0xff | !w * 0x3f00);  // Gets first (upper) 6 bits or last (lower) 8 bits, depending on the value of w.
		
		if (!w) {
			copyBits(this->t, 8, 13, (uint16_t)data, 0, 5);
			clrBit(this->t, 14);
		} else {
			copyBits(this->t, (uint16_t)data, 0, 7);
			this->v = this->t;
		}

		w = !w;
		break;
	case(0x2007):  // PPUDATA
		// Modify VRAM.
		// NOTE: There is some odd behavior regarding writes and reads to PPUDATA during rendering. This is currently not emulated, but some games do make use of such behavior.
		//oldValue = this->getByte(this->registers.PPUADDR);
		oldValue = this->databus.write(this->v, data);
		// Now we increment PPUADDR by 32 if bit 2 of PPUCTRL is set (the nametable is 32 bytes long, so this essentially goes down).
		// Otherwise, we increment PPUADDR by 1 (going right).
		this->v += 1 << (5 * getBit(this->control, 2));
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
		if (this->frameCount == 0x2a && this->beamPos.lineInRange(0x0ff, 0x105)) {
			int _ = 0;
		}
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
		this->ioBus += this->status;  // Then, add the last 3 bits of PPUSTATUS to it (the other btis in PPUSTATUS should be 0).
		// Now we clear the V blank flag of status:
		this->status &= 0b01111111;
		break;
	case(0x2004):  // OAMDATA
		this->ioBus = this->OAM.getByte(this->OAMAddr);  // Simply gets the value from OAM at OAMADDR.
		break;
	case(0x2007):  // PPUDATA
		// Instead of returning the value at the given address, we actually return a value in a buffer;
		// we then update the buffer with the value at the given address. This effectively delays PPUDATA
		// reads by 1.
		this->ioBus = this->PPUDATABuffer;  // NOTE: There is some open-bus behavior when reading from palette RAM, however I do not know enough to emulate it at the moment.
		this->PPUDATABuffer = this->databus.read(this->v);
		// Now we increment PPUADDR by 32 if bit 2 of PPUCTRL is set (the nametable is 32 bytes long, so this essentially goes down).
		// Otherwise, we increment PPUADDR by 1 (going right).
		this->v += 1 << (5 * getBit(this->control, 2));
		break;
	default:  // This catches the reads to write-only registers.
		break;
	}

	return this->ioBus;
}
bool PPU::requestingNMI() const {
	// We request an NMI when we are in Vblank AND the 7th bit in PPUCTRL is set.
	bool requestNMI = (getBit(this->control, 7)) && this->beamPos.inVblank(true);
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
bool PPU::isRendering(bool includePrerender) const {
	// The PPU is rendering if 1. either background OR sprite rendering is on, 2. it is inbetween scanlines 0 and 239 inclusive.
	bool backgroundRendering = getBit(this->mask, 3);
	bool spriteRendering = getBit(this->mask, 4);

	bool onRenderLines = this->beamPos.onRenderLines() || (this->beamPos.inPrerender() && includePrerender);

	return (backgroundRendering || spriteRendering) && onRenderLines; // NOTE: For now, this function will return whether rendering is enabled.
}
void PPU::updatePPUSTATUS() {  // TODO: Implement sprite overflow and sprite 0 hit flags.
	if (this->beamPos.inVblank(true)) {
		setBit(this->status, 7);
	} else if (this->beamPos.inPrerender(true)) {
		clrBit(this->status, 7);
	}
}
void PPU::updateRenderingRegisters() {
	// The PPU will update differently based on the current cycle.
	// See frame timing diagram for more info.

	if (this->beamPos.dotInRange(1, 256) || this->beamPos.dotInRange(328, 340)) {

		if (this->beamPos.dot == 0x100) {
			this->incrementScrolling(true);  // At the end of a render line, increment fine y (coarse y if fine y overflows).
		}
		if (this->beamPos.dot % 8 == 0) {  // At the end of a tile, increment coarse x.
			this->incrementScrolling();

		}
	}

	// NOTE: I am unsure about this->beamPos.dotInRange(328, 335); 
	// in line with what the wiki says, which says that it should increment coarse x again at 336.
	// Background fetches
	// The condition before the || ensures to perform datafetches throughout the frame, the latter ensures data fetches for the 1st 2 tiles for the next frame.
	if (this->beamPos.inRender() || this->beamPos.dotInRange(321, 336)) { 
		this->performBackgroundFetches();
	}
	// FIXME: inRender fails to transfer tile data on cycle 257

	if (this->beamPos.inHblank(true)) {  // Upon reaching Hblank, transfer bits in t to v.
		copyBits(this->v, this->t, 0, 4);
		copyBits(this->v, this->t, 10, 10);
	}
	if (this->beamPos.dotInRange(280, 304) && this->beamPos.inPrerender()) {  // Same as above, but every cycle in the pre render line in this specific region.
		copyBits(this->v, this->t, 5, 9);
		copyBits(this->v, this->t, 11, 14);
	}

	// Sprite fetches (NOTE: I don't know if sprite fetches are disabled in non-rendering or on non-rendering lines; for now this code assumes the fetches are always happening.)
	/*
	NOTE: I will not be implementing many of the cycle-level details as they will not affect most games. Only if I decide
	to try to support more titles with more exotic programming practices will I attempt any changes. The main way I am simplifying
	this code is by not respecting cycle-level accuracy, meaning multiple bytes are written at once on a regular basis.
	*/
	/* Note:
	On odd cycles, data is read from (primary) OAM
	On even cycles, data is written to secondary OAM 
	(unless secondary OAM is full, in which case it will read the value in secondary OAM instead)
	*/
	if (this->beamPos.dot == 0x0) {
		// NOTE: I don't know if the PPU actually enables writing for the secondary OAM on cycle 0; this is just a guess.
		// IGNORE COMMENT // I also do not know, and in fact doubt, whether it sets OAMAddr to 0.
		if (this->beamPos.inRange(0x7f, 0x7f, 0x0, 0x0)) {
			int _ = 0;
		}
		this->secondaryOAM.freeAllBytes();
		this->spriteEvalCycle.reset();
	}

	// Perform shifts if in the given range.
	if (this->beamPos.dotInRange(0x0, 0x100)) {
		this->updateSpriteShiftRegisters();
	}
	
	// OAM, 2ndOAM, and Shift register transfers
	if (this->beamPos.dotInRange(0x1, 0x40)) {  // Set all bytes in secondary OAM to 0xff.
		this->secondaryOAM.setByte((this->beamPos.dot - 1) / 2, 0xff);
		if (this->beamPos.dot == 0x40) {
			this->spriteEvalCycle.setState(FINDING_SPRITES);
		}
	} else if (this->beamPos.dotInRange(0x41, 0x100)) {  // Sprite evaluation period.
		this->performSpriteEvaluation();
	} else if (this->beamPos.dotInRange(0x101, 0x140)) {  // 2ndOAM-to-shiftRegister transfer period.
		this->transferSpriteData();
	}

	// Misc
	// "OAMADDR is set to 0 during each of ticks 257–320 (the sprite tile loading interval) of the pre-render and visible scanlines" -- NESDev, OAMADDR
	bool a, b, c;
	a = this->beamPos.lineInRange(0, 239);
	b = this->beamPos.lineInRange(261, 261);
	c = this->beamPos.dotInRange(257, 320);
	if ((this->beamPos.lineInRange(0, 239) || this->beamPos.lineInRange(261, 261)) && this->beamPos.dotInRange(257, 320)) {
		this->OAMAddr = 0;
	}


}
void PPU::fetchPatternData(uint8_t patternID, bool table, bool high, int line, uint16_t& pattern, bool flipH, bool flipV) {
	
	if (line > 7 || line < 0) {
		int _ = 0;  // This should never happen.
	}

	if (flipV) {  // Access the bottom of the pattern if flipping vertically.
		line = 7 - line;
	}
	
	const uint16_t PATTERN_TABLE_SIZE = 0x1000;

	// Using the nametable byte, we will grab the associated pattern.
	uint16_t addr = (PATTERN_TABLE_SIZE * table) + (0x10 * patternID);  // Each pattern is 16 bytes large.
	
	// We will get a different pair of bytes from the pattern depending on the current line (e.g. get 1st pair on line 0, 2nd pair on line 1...).
	addr += line;  // Selecting the line. 
	addr += 0x8 * high;

	uint16_t patternLatch = this->databus.read(addr);
	if (flipH) {  // Reverse the bits if flipping horizontally.
		patternLatch = reverseBits(patternLatch, 8);
	}

	pattern = patternLatch;
}
// TODO: Refactor
void PPU::performBackgroundFetches() {
	// TODO: Give this variable and function a better name.
	uint8_t cycleCounter = (this->beamPos.dot) % 8;  // This variable ranges from 0 to 7 and represents cycles 8, 16, 24... 256.
	bool fetchingNextLineTiles = this->beamPos.dotInRange(0x100, 341);

	// The shift registers are shifted to the right by 1 every data-fetching cycle.
	this->backgroundShiftRegisters >>= 1;

	// The pattern and attribute shifters are reloaded on cycle counter 0. (NOTE: It might transfer on cycleCounter 7, judging from frame timing diagram.)
	if (cycleCounter == 0) {  // NOTE: Maybe the last tile fetched (the 2nd of the next line) is not being transferred? 
		this->backgroundShiftRegisters.transferLatches(this->latches);
	}

	const uint16_t FIRST_NAMETABLE_ADDR = 0x2000, NAMETABLE_SIZE = 0x400;
	const uint16_t NAMETABLE_ADDR = FIRST_NAMETABLE_ADDR + NAMETABLE_SIZE * getBits(this->control, 0, 1), BACKGROUND_PATTERN_TABLE_ADDR = getBit(this->control, 4) << 12;
	// Now, we will load the latches every other cycle.
	switch (cycleCounter) {
	case(1): { // Fetching nametable byte.
		// Using the internal v register to define the scroll.
		// NOTE: For now, we are only rendering the first nametable w/o proper mirroring or any scrolling.
		//addr = FIRST_NAMETABLE_ADDR + getBits(this->v, 0, 4) + getBits(this->v, 5, 9) * 32;
		uint16_t coarseX = getBits(this->v, 0, 4);
		uint16_t coarseY = getBits(this->v, 5, 9);
		//c = b << 1;  // When we get the coarse y and store it in b, they are offset by 5, so 0bYYYYY00000. First, we shift it right by 5 to account for this, then multiply by 32
		// To account for the length (in tiles) of the x-axis. Simplified, this is the same as not shifting at all.
		
		uint16_t addr = NAMETABLE_ADDR + coarseX + coarseY;
		if (addr >= 0x23ba) {
			int _ = 0;
		}
		if (addr == 0x2000) {  // NOTE: This fetch should ONLY happen before rendering starts; that is not what is happening here. These are being fetched at the start of render after the first (correctly timed) fetches.
			int _ = 0;
		}
		if (addr == 0x2002) {
			int _ = 0;
		}

		this->latches.nametableByteLatch = this->databus.read(addr);
		if (this->latches.nametableByteLatch != 36) {
			int _ = 0;
		}

		break;
	}
	case(3): { // Fetching attribute table byte.
		uint16_t addr = NAMETABLE_ADDR + 0x3c0; // 0x3c0 = Size of nametable; after this is the attribute table.

		// We can form the attribute address via the following format:
		// NN1111YYYXXX; where NN is the nametable select, 1111 a constant offset, YYY and XXX the high bits of their coarse offsets.
		// NOTE: I am  not 100% sure if this will be a bug, but this uses the new value of v instead of the old one. If buggy behavior arises, look here.
		uint16_t xCoord = getBits(this->v, 2, 4) >> 2;  // Coarse x (ignore lower 2 bits)
		uint16_t yCoord = getBits(this->v, 7, 9) >> 7;  // Coarse y (ignore lower 2 bits)
		// a and b form the x, y coordinate for a nametable tile.

		yCoord *= 8;

		addr += xCoord + yCoord;  // Offset the address given what part of the nametable we are 
		uint16_t attrByte = this->databus.read(addr);  // TODO: Debug; I think there is a bug here but it isn't clear.

		/* Once we have the attribute byte, we select two bits given the 2x2 quadrant in the 4x4 block we are in.
		A block is divided into 4 pieces like so:

			01
			23

		Where 0, 1, 2, 3 represent the 2 bits in the attribute byte (0 - 1st 2 bits, 1 - bits 2 & 3 (0BI), etc.)

		The quadrant is determined by bit 1 of the x and y coarse offsets.

		  0 1 x
		0 0 1
		1 2 3
		y

		Or expanded to include the 0th coarse bit:

		   00 01 10 11 x
		00  0  0  1  1
		01  0  0  1  1
		10  2  2  3  3
		11  2  2  3  3
		 y

		 So the formula, w/ y1 representing bit 1 of coarse y and x1 bit 1 of coarse x is:

		 a = (y1 << 1) + x1

		 And the bit indexing formula is:
		 upper bit = 2 * a + 1
		 lower bit = 2 * a

		*/
		uint8_t upperBit = getBit(this->v, 6) << 1;  // This selects which part of the byte given the quadrant this tile is in.
		uint8_t lowerBit = getBit(this->v, 1);
		uint8_t corner = lowerBit + upperBit;

		// Finally we move the bits into the appropriate latches.
		this->latches.attributeLatchLow = getBit(attrByte, 2 * corner);
		this->latches.attributeLatchHigh = getBit(attrByte, 2 * corner + 1);
		break;
	}
	case(5): { // Fetching pattern table tile low.
		// Using the nametable byte, we will grab the associated pattern.
		/*
		Fun fact: I found an incredibl(y frustrating)e bug here.
		See the this->beamPos.scanline % 8? That selects the line of pixels for the pattern.
		The problem? This fetching routine also fetches tiles on the NEXT line, so everything on the left is ruined.
		
		*/
	this->fetchPatternData(this->latches.nametableByteLatch, 
			getBit(this->control, 4), 
			false, 
			(this->beamPos.scanline + fetchingNextLineTiles) % 8,
			this->latches.patternLatchLow);
		break;
	}
	case(7): { // Fetching pattern table tile high.
		this->fetchPatternData(this->latches.nametableByteLatch,
			getBit(this->control, 4),
			true,
			(this->beamPos.scanline + fetchingNextLineTiles) % 8,
			this->latches.patternLatchHigh);
		break;
	}
	default:
		break;

	}
}
void PPU::performSpriteEvaluation() {
	/* From NESDev
	The value of OAMADDR at this tick determines the starting address for sprite evaluation for 
	this scanline, which can cause the sprite at OAMADDR to be treated as it was sprite 0, 
	both for sprite-0 hit and priority. If OAMADDR is unaligned and does not point to the 
	Y position (first byte) of an OAM entry, then whatever it points to 
	(tile index, attribute, or X coordinate) will be reinterpreted as a Y position, 
	and the following bytes will be similarly reinterpreted. 
	No more sprites will be found once the end of OAM is reached, 
	effectively hiding any sprites before the starting OAMADDR.
	*/
	// this->OAMAddr SHOULD be 0, but if it isn't this function should emulate this inappropriate behavior accordingly.
	
	switch (this->spriteEvalCycle.evalState) { 
		// NOTE: The sprite flicker bug occurs here because the code inside the block causes the bug (having .byteType prevents the bug from occuring by preventing any of this code from executing.)
	case(FINDING_SPRITES): {
	
		//if (this->spriteIdx >= 64) {
		// 	int _ = 0;  // This should never happen.
		//}

		// NOTE: Odd-read and even-writes are not emulated; both occur in the same cycle implementation-wise.
		// NOTE: Another thing which isn't emulated is the exact timing for primary-to-secondary OAM transfers; 
		// when a sprite is detected to be in range of the next line

		if (this->spriteEvalCycle.onByte(Y_COORD)) {
			uint8_t yCoord = this->OAM.getByte(this->OAMAddr);  // Stores y-coordinate of the sprite
			uint8_t nextLine = (this->beamPos.scanline + 1) % 262;  // Allows for line 261 to line 0 wrapping.

			// Checks if the next line is within the range for the sprite.
			// TODO: Implement checking for 8x16 sprites.
			if (yCoord <= nextLine && nextLine <= yCoord + 7) {  // If it is, copy over the next few bytes.
				this->secondaryOAM.setFreeByte(this->OAM.getByte(this->OAMAddr));
				this->spriteEvalCycle.setState(COPY_SPRITE_DATA);
				break;
			}
		}
		this->OAMAddr += 4;
		this->spriteEvalCycle.setState(INCREMENT_CHECK);
	}
	break;
	case(COPY_SPRITE_DATA): {
		++this->OAMAddr;
		++this->spriteEvalCycle;
		if (this->spriteEvalCycle.onByte(Y_COORD)) {  // When we wrap back to the first byte type.
			this->spriteEvalCycle.setState(INCREMENT_CHECK);
			break;
		}
		this->secondaryOAM.setFreeByte(this->OAM.getByte(this->OAMAddr));
	}
	break;
	case(INCREMENT_CHECK): {
		this->OAMAddr %= 0x100;
		if (this->OAMAddr < 4) {  // Checks if all sprites have been evaluated by seeing if OAMAddr wrapped around.
			this->OAMAddr = 0;
			this->spriteEvalCycle.setState(POINTLESS_COPYING);
		}
		else if (!this->secondaryOAM.getWriteState()) {  // Checks if secondaryOAM is filled.
			this->spriteEvalCycle.setState(SPRITE_OVERFLOW);
		}
		else {
			this->spriteEvalCycle.setState(FINDING_SPRITES);
		}
		break;
	}
	case(SPRITE_OVERFLOW): {
		uint8_t yCoord = this->OAM.getByte(this->OAMAddr);  // Stores y-coordinate of the sprite
		uint8_t nextLine = (this->beamPos.scanline + 1) % 262;  // Allows for line 261 to line 0 wrapping.
		if (yCoord <= nextLine && nextLine <= yCoord + 7) {  // If another sprite is detected in this area, 
			// TODO: set sprite overflow flag.
		}
		this->OAMAddr += 5;  // Should be 4, but there is a bug in the PPU which also increments the byte being evaluated, so increment by 5 to account for this (4 + 1, sprite index increment + byte index increment).
		this->spriteEvalCycle.setState(INCREMENT_CHECK);
		break;
	}
	case(POINTLESS_COPYING): {
		// NOTE: the failed writes are not emulated.
		//this->secondaryOAM.setFreeByte(this->OAM.getByte(4 * this->spriteIdx));
		//++this->OAMAddr %= 64;
		this->OAMAddr = 0;
		break;
	}
	default:
		break;
	}
}
void PPU::transferSpriteData() {
	// This period should last for 64 cycles, 2 cycles for each byte, and another 2 to idle while the PPU fetches sprite pattern data.
	int relativeDot = (this->beamPos.dot - 0x101);  // Ranges from 0 to 63.
	int sprite = relativeDot / 8;
	uint16_t secondaryOAMAddr = relativeDot / 2;

	int nextLine = (this->beamPos.scanline + 1) % 262;

	// TODO: Support 8x16 sprites.
	// Note: This emulator will not emulate the exact cycle-behavior of these transfers. 
	// Instead, it will do multiple cycles at once and idle till the next one it needs to do something.
	if (relativeDot % 8 == 0) {  // When we are on a new sprite (every 8 dots), perform all fetches at once.
		// First get the sprite y.
		uint8_t spriteY = this->secondaryOAM.getByte(secondaryOAMAddr);
		uint8_t patternID = this->secondaryOAM.getByte(secondaryOAMAddr + 1);
		uint8_t attributes = this->secondaryOAM.getByte(secondaryOAMAddr + 2);
		uint8_t spriteX = this->secondaryOAM.getByte(secondaryOAMAddr + 3);

		if (spriteY == 0xff) {  // This is an "empty" sprite slot; fill all values w/ 0xff00 and 0xff
			this->spriteShiftRegisters.at(sprite).patternShiftRegisterLow = 0xff00;
			this->spriteShiftRegisters.at(sprite).patternShiftRegisterHigh = 0xff00;
			this->spriteShiftRegisters.at(sprite).attributeBits = 0xb11;
			this->spriteShiftRegisters.at(sprite).x = 0x100;  // Sets the position at dot 256, which is outside the rendering range.
			return;
		}

		bool patternTable = getBit(this->control, 3);

		// Then get the line the sprite will need to display the next line.
		int spriteLine = nextLine - spriteY;
		if (spriteLine < 0 && spriteY != 0xff) {  // If the sprite's Y is 0xff, then it will be hidden (it won't wrap around to the top).  
			int _ = 0;  // NOTE: This has been hit at some points, possibly indicating a bug.
			return;
		}

		bool flipH = !getBit(attributes, 6);  // Due to the way the shift register works, sprites are flipped when they enter. NOTE: I think this behavior applies to backgrounds too; I might include it in the fetchPatternData function.
		bool flipV = getBit(attributes, 7);

		// Now we fetch the patterns. 
		this->fetchPatternData(patternID,
			patternTable,
			false,
			spriteLine,
			this->spriteShiftRegisters.at(sprite).patternShiftRegisterLow,
			flipH,
			flipV);
		this->fetchPatternData(patternID,
			patternTable,
			true,
			spriteLine,
			this->spriteShiftRegisters.at(sprite).patternShiftRegisterHigh,
			flipH,
			flipV);
		// NOTE: The above methods clobber the lower bits. This is considered acceptable because this is a non-rendering
		// period, but in case any bugs arise, they should be looked into.
		this->spriteShiftRegisters.at(sprite) <<= 8;

		// Transfering the location of the sprite.
		this->spriteShiftRegisters.at(sprite).x = spriteX;

		// And lastly, the attribute data.
		this->spriteShiftRegisters.at(sprite).attributeBits = getBits(attributes, 0, 1);		
	}
}
void PPU::updateSpriteShiftRegisters() {
	// Assumptions: Sprite shift registers were already loaded the line before, we are on the dot to be rendered.
	// This method will execute every cycle between cycles 1 and 256.
	
	// NOTE: This is almost definitely not how it is done in actual hardware.
	// NOTE: I might remove this method.

	// Every sprite shift unit will be shifted right. When a shift unit is shifted right, it first
	// decrements its x until it reaches 0, afterwards it starts shifting its shift registers.
	// So if we are on pixel 0 (cycle 1) and a sprite unit has x = 5, we will decrement it and get x = 4.
	// This continues until x = 0, or when pixel = 4. Then on pixel = 5, the shift register itself is shifted 
	// down into a position which allows its first pixel to be rendered.
	
	this->spriteShiftRegisters >>= 1;
}
void PPU::updateBeamLocation() { 
	if (this->beamPos.updatePosition(this->frameCount & 1)) {
		++this->frameCount;
	}
}
void PPU::incrementScrolling(bool axis) {  // Increments scrolling
	// TODO: Clean up code.

	// Format of the v internal register: yyyNNYYYYYXXXXX
	// y = fine Y scroll; N = nametable; Y = coarse Y scroll; X = coarse X scroll. Fine X scroll is located in the x internal register.
	// When a fine axis overflows, its coarse portion increments. When the coarse axis overflows, the next nametable corresponding to their axis is selected.
	
	// TODO: Implement the nametable incrementing. When I implement this, I will might have to prevent erroneous nametable incrementing (i.e. when scroll = 0).

	if (axis) {  // Increment y
		if ((this->v & 0b111000000000000) == 0b111000000000000) {  // Increment coarse y; Checks if fine y scroll is overflowing
			this->v ^= 0b111000000000000;

			if ((this->v & 0b1110100000) == 0b1110100000) {  // Check for overflow in coarse y. Note overflow occurs after coarse row 29.
				this->v ^= 0b1110100000;
				
			} else {
				this->v += 0b100000;
			}

		} else { // Increment fine y
			this->v += 0b001000000000000; 
		}
	} else {  // Increment coarse x; fine x is not incremented during rendering.
		if ((this->v & 0b11111) == 0b11111) {  // Check for overflow in coarse x.
			this->v ^= 0b11111;
		} else {
			++this->v;
		}
	}
}
uint8_t PPU::getBGColor() {  // TODO
	const uint16_t backgroundPaletteAddress = 0x3f00;  // The starting address for the background palette.

	// Indexing the palette.
	uint8_t bgPaletteIdx = this->backgroundShiftRegisters.getPattern(this->x);
	// Indexing which palette we want.
	auto a = this->backgroundShiftRegisters.getPattern(this->x);
	uint8_t bgPalette = this->backgroundShiftRegisters.getAttribute(this->x);

	// Creating the address to take the color index from.
	uint16_t addr = backgroundPaletteAddress;
	addr += 4 * bgPalette + bgPaletteIdx;

	// Reading and returning the color index.
	uint8_t colorIdx = this->databus.read(addr);
	return colorIdx;
}
uint8_t PPU::getSpriteColor() {
	// Search for a non-transparent (if it exists) pixel w/ the lowest sprite index possible at the current location.
	const uint16_t spritePaletteAddress = 0x3f10;
	uint16_t color = 0;
	// Find the first non-transparent pixel.
	for (int i = 0; i < 8; ++i) {
		uint8_t pattern = this->spriteShiftRegisters.at(i).getPattern(x);
		// If the pattern is 0b00, then it is considered transparent. If the resulting color is 0b00, it is also transparent.
		if (!pattern) continue;
		// Now get the color.
		uint8_t palette = this->spriteShiftRegisters.at(i).getAttribute();
		uint16_t colorAddr = spritePaletteAddress + 4 * palette + pattern;  
		color = this->databus.read(colorAddr);
		// Now we check the color
		if (!color) continue;
		break;  // If both cases are passed, then we will return this color.
	}

	return color;
}
void PPU::drawPixel() {

	if (this->graphics == nullptr) {  // If we are not given a graphics object, do not attempt to draw.
		//std::cout << "Warning: No graphics object provided for PPU; no output will be displayed." << std::endl;
		return;
	}

	// We will figure out what color we need to draw.
	uint16_t colorKey = 0;
	// First, copy the emphasis values from PPUMASK to the color key.
	copyBits(colorKey, 6, 8, (uint16_t)this->mask, 5, 7);

	// Now we have to find the color index for this pixel.
	// Getting the high and low bits of the pattern at the appropriate point.
	const uint16_t spritePaletteAddress = 0x3f10;


	uint8_t spriteColorIdx = this->getSpriteColor();
	uint8_t bgColorIdx = this->getBGColor();  // Note: bg stands for background.

	// TODO: Take into account sprite vs bg priority; for now, sprites are always given priority.
	if (spriteColorIdx) {
		colorKey |= spriteColorIdx;
	} else {
		colorKey |= bgColorIdx;
	}

	// NOTE: The fact that drawing starts on dot 0 but the rest of the PPU is idle on that cyc;e might cause the weird left-tile visual bugs.
	if (this->beamPos.dot < 0x100 && this->beamPos.scanline < 0xf0) {  // Do not draw past dot 256 or scanline 240
		this->graphics->drawSquare(this->paletteMap.at(colorKey), this->beamPos.dot, this->beamPos.scanline, 1);
	}
}

// --- Non-PPU Methods --- //

BackgroundShiftRegisters::BackgroundShiftRegisters() : 
	patternShiftRegisterLow(0),
	patternShiftRegisterHigh(0),
	attributeShiftRegisterLow(0),
	attributeShiftRegisterHigh(0)
{}
BackgroundShiftRegisters::~BackgroundShiftRegisters() {}
uint8_t BackgroundShiftRegisters::getPattern(int x) const {
	uint8_t pattern = getBit(this->patternShiftRegisterHigh, x) << 1;  // Fetching the high bit.
	pattern += getBit(this->patternShiftRegisterLow, x);  // Then the low bit.

	return pattern;
}
uint8_t BackgroundShiftRegisters::getAttribute(int x) const {
	uint8_t pattern = getBit(this->attributeShiftRegisterHigh, x) << 1;  // Fetching the high bit.
	pattern += getBit(this->attributeShiftRegisterLow, x);  // Then the low bit.
	return pattern;
}
BackgroundShiftRegisters& BackgroundShiftRegisters::operator>>=(const int& n) {
	this->patternShiftRegisterHigh >>= 1;
	this->patternShiftRegisterLow >>= 1;
	this->attributeShiftRegisterHigh >>= 1;
	this->attributeShiftRegisterLow >>= 1;

	return *this;
}
void BackgroundShiftRegisters::transferLatches(BackgroundLatches latches) {
	this->patternShiftRegisterLow |= reverseBits(latches.patternLatchLow, 8) << 8;  // The pattern will be fed right-to-left, so mirror the pattern to ensure proper feeding.
	this->patternShiftRegisterHigh |= reverseBits(latches.patternLatchHigh, 8) << 8;
	this->attributeShiftRegisterLow |= latches.attributeLatchLow * 0xff << 8;
	this->attributeShiftRegisterHigh |= latches.attributeLatchHigh * 0xff << 8;
}

BackgroundLatches::BackgroundLatches() :
	patternLatchLow(0),
	patternLatchHigh(0),
	attributeLatchLow(0),
	attributeLatchHigh(0),
	nametableByteLatch(0)
{}
BackgroundLatches::~BackgroundLatches() {}

PPUPosition::PPUPosition() :
	scanline(0),
	dot(0)
{}
PPUPosition::~PPUPosition() {}
bool PPUPosition::updatePosition(bool oddFrame) {
	// First, check if we need to skip dot 340, line 261 (only do this on odd frames).
	if (oddFrame & 0b1) {  // Check if the frame is odd
		if (this->dot == 340 && this->scanline == 261) {
			this->dot = 0;
			this->scanline = 0;
			return true;
		}
	}

	// Then, increment dot and scanline (the latter only if dot > 340).
	if (++this->dot > 340) {
		this->dot = 0;
		if (++this->scanline > 261) {
			this->scanline = 0;
			return true;
		}
	}

	// If we never needed to wrap the scanline, then the frame has not changed.
	return false;
}
bool PPUPosition::dotInRange(int lowerBound, int upperBound) const {
	return (this->dot <= upperBound) && (this->dot >= lowerBound);
}
bool PPUPosition::lineInRange(int lowerBound, int upperBound) const {
	return (this->scanline <= upperBound && this->scanline >= lowerBound);
}
bool PPUPosition::inRange(int lineLowerBound, int lineUpperBound, int dotLowerBound, int dotUpperBound) const {
	return this->dotInRange(dotLowerBound, dotUpperBound) && this->lineInRange(lineLowerBound, lineUpperBound);
}
bool PPUPosition::inVblank(bool reached) const {
	if (reached) {  // In this case we only care if we are exactly on the start of vblank.
		bool beganVblank = this->scanline == FIRST_VBLANK_LINE && this->dot == 1;
		return beganVblank;
	}

	// We reach Vblank when we are on dot 1 of the first VBlank line and end on dot 340 of the last Vblank line (line 260).
	bool onVblank = this->scanline > FIRST_VBLANK_LINE && this->scanline <= LAST_VBLANK_LINE;  // First check if we are inbetween the first and last vblank (exclusive, inclusive)
	// Return true if we know we are on vblank.
	if (onVblank) {
		return onVblank;
	}
	// If not, check if we are on the first vblank line; if so, check if we are on or past the first dot (dot 0 on the first vblank line should not count).
	onVblank = this->scanline == FIRST_VBLANK_LINE && this->dot >= 1;
	return onVblank;
}
bool PPUPosition::inHblank(bool reached) const {
	if (reached) {
		return this->dot == 257;
	}
	return this->dot >= 257;
}
bool PPUPosition::inPrerender(bool reached) const {
	if (reached) {
		return this->scanline == PRE_RENDER_LINE && this->dot == 1;
	}
	
	return this->scanline == PRE_RENDER_LINE;
}
bool PPUPosition::inRender(bool reached) const {
	bool onRenderLine = this->onRenderLines(reached);
	if (onRenderLine) {  // First check if we are on the right line.
		if (reached) {  // Check if we are exactly on the first dot for reached, if we are within the range for not reached.
			return this->dot == 0x1;
		}
		return this->dot >= 0x1 && this->dot <= 0x101;
	}

	return false;
}
bool PPUPosition::onRenderLines(bool reached) const {
	if (reached) {
		return this->scanline == VISIBLE_LINE;
	}

	bool onRenderLines = this->scanline >= VISIBLE_LINE && this->scanline <= LAST_RENDER_LINE;
	return onRenderLines;
}
bool PPUPosition::inTrueVblank(bool reached) const {
	const int TRUE_VBLANK_START_LINE = 0x240;
	if (reached) {  // In this case we only care if we are exactly on the start of vblank.
		bool beganVblank = this->scanline == TRUE_VBLANK_START_LINE && this->dot == 0;
		return beganVblank;
	}

	// We reach Vblank when we are on dot 1 of the first VBlank line and end on dot 340 of the last Vblank line (line 260).
	bool onVblank = this->scanline > TRUE_VBLANK_START_LINE && this->scanline <= LAST_VBLANK_LINE;  // First check if we are inbetween the first and last vblank (exclusive, inclusive)
	// Return true if we know we are on vblank.
	if (onVblank) {
		return onVblank;
	}
	// If not, check if we are on the first vblank line; if so, check if we are on or past the first dot (dot 0 on the first vblank line should not count).
	onVblank = this->scanline == TRUE_VBLANK_START_LINE && this->dot >= 0;
	return onVblank;
}

SpriteShiftUnit::SpriteShiftUnit() : 
	patternShiftRegisterLow(0), 
	patternShiftRegisterHigh(0), 
	attributeBits(0),
	x(0)
{}
SpriteShiftUnit::~SpriteShiftUnit() {}
uint8_t SpriteShiftUnit::getPattern(int x) const {
	x = 7 - x;  // Keep in mind that the index starts from left to right.
	if (this->patternShiftRegisterLow & 0xff) {
		int _ = 0;
	}
	uint8_t pattern = getBit(this->patternShiftRegisterHigh, x) << 1;  // Fetching the high bit.
	pattern |= getBit(this->patternShiftRegisterLow, x);  // Then the low bit.
	return pattern;
}
uint8_t SpriteShiftUnit::getAttribute() const {
	return this->attributeBits;
}
SpriteShiftUnit& SpriteShiftUnit::operator>>=(int n) {
	 
	// Make sure x is fully decremented before shifting any of the registers.
	if (this->x > 0) {
		if (this->x > n) {
			this->x -= n;
			return *this;
		} else {
			n -= this->x;
			this->x = 0;
		}
	}

	this->patternShiftRegisterHigh >>= n;
	this->patternShiftRegisterLow >>= n;

	return *this;
}
SpriteShiftUnit& SpriteShiftUnit::operator<<=(int n) {
	this->patternShiftRegisterHigh <<= n;
	this->patternShiftRegisterLow <<= n;
	
	return *this;
}

SpriteShiftRegisters::SpriteShiftRegisters() {}
SpriteShiftRegisters::~SpriteShiftRegisters() {}
SpriteShiftUnit& SpriteShiftRegisters::at(int idx) {
	// If an invalid index is given, default to index 0.
	if (idx >= 8 || idx < 0) {
		idx = 0;
	}
	return this->shiftRegisters.at(idx);
}
uint8_t SpriteShiftRegisters::getPattern(int x) {
	return 0;  // TODO
}
uint8_t SpriteShiftRegisters::getAttribute(int x) {
	return 0;  // TODO
}
void SpriteShiftRegisters::shiftRegister(int sprite) {
	this->shiftRegisters.at(sprite) >>= 1;
}
void SpriteShiftRegisters::operator>>=(const int& n) {
	for (int i = 0; i < 8; ++i) {
		this->shiftRegisters.at(i) >>= n;
	}
}
void SpriteShiftRegisters::operator<<=(const int& n) {
	for (int i = 0; i < 8; ++i) {
		this->shiftRegisters.at(i) <<= n;
	}
}

SpriteEvalCycle::SpriteEvalCycle() : byteType(Y_COORD), evalState(INIT) {}
SpriteEvalCycle::~SpriteEvalCycle() {}
void SpriteEvalCycle::operator++() {
	switch (this->byteType) {
	case(Y_COORD):
		this->byteType = TILE_IDX;
		break;
	case(TILE_IDX):
		this->byteType = ATTRIBUTES;
		break;
	case(ATTRIBUTES):
		this->byteType = X_COORD;
		break;
	case(X_COORD):
		this->byteType = Y_COORD;
		break;
	}
}
bool SpriteEvalCycle::onByte(SpriteByteOn sbo) const {
	return this->byteType == sbo;
}
bool SpriteEvalCycle::onState(SpriteEvaluationState ses) const {
	return this->evalState == ses;
}
void SpriteEvalCycle::setState(SpriteEvaluationState ses) {
	this->evalState = ses;
}
void SpriteEvalCycle::reset() {
	this->byteType = Y_COORD;
	this->evalState = INIT;
}
