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
	attributeShiftRegisterLow(0),
	attributeShiftRegisterHigh(0),
	patternShiftRegisterLow(0),
	patternShiftRegisterHigh(0),
	attributeLatchLow(0),
	attributeLatchHigh(0),
	nametableByteLatch(0),
	patternLatchLow(0),
	patternLatchHigh(0),
	paletteMap(loadPalette("resourceFiles/2C02G_wiki.pal")),
	dot(0),
	scanline(0),
	frameCount(0)
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
	attributeShiftRegisterLow(0),
	attributeShiftRegisterHigh(0),
	patternShiftRegisterLow(0),
	patternShiftRegisterHigh(0),
	attributeLatchLow(0),
	attributeLatchHigh(0),
	nametableByteLatch(0),
	patternLatchLow(0),
	patternLatchHigh(0),
	paletteMap(loadPalette("resourceFiles/2C02G_wiki.pal")),
	dot(0),
	scanline(0),
	frameCount(0)
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
		this->drawPixel();
	}

	++this->cycleCount;
	this->updateBeamLocation();
}


uint8_t PPU::writeToRegister(uint16_t address, uint8_t data) {
	// Deduces what register the operation should occur on, then performs the appropriate operation.
	
	// All writes, even to read-only registers, change the I/O bus. (NOTE: doublecheck this)
	this->ioBus = data;

	uint8_t oldValue = 0;

	// For specific behaviors, look at PPU Scrolling.
	switch (address) {
	case(0x2000):  // PPUCTRL
		oldValue = this->registers.PPUCTRL;
		if (this->cycleCount >= PRE_RENDER_LINE * PPU_CYCLES_PER_LINE) {  // Ignore writes to PPUCTRL until pre-render line is reached.
			this->registers.PPUCTRL = data;
		}

		copyBits(this->t, 10, 11, (uint16_t)data, 0, 1);
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
		this->v += 1 << (5 * getBit(this->registers.PPUCTRL, 2));
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

bool PPU::inHBlank() const {
	return this->dot >= 257;
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

	return (backgroundRendering || spriteRendering); // NOTE: For now, this function will return whether rendering is enabled.
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
	uint16_t currentDot = this->dot;
	// TODO: Fix a bug relating to timing; when currentDot is 0x100, this->v's coarse x component is 0x1b.
	if (currentDot == 0) {
		// Idle cycle.
	} else if (currentDot <= 0x100 && this->scanline < 240) {
		if (currentDot == 0x100) {
			this->v;
			this->incrementScrolling(true);
		} 
		if (currentDot % 8 == 0) {
			this->incrementScrolling();
		}

		// Datafetching cycles; also the drawing cycles, but drawing is handled outside of this function.
		this->performDataFetches();
	} else if (currentDot == 0x101) {  // Dot 257
		// At this point, bits related to horizontal positioning in t is copied to v.
		copyBits(this->v, this->t, 0, 4);
		copyBits(this->v, this->t, 10, 10);
	} else if (currentDot >= 280 && currentDot <= 304 && this->scanline == PRE_RENDER_LINE) {
		copyBits(this->v, this->t, 5, 9);
		copyBits(this->v, this->t, 11, 14);
	}
	else if (currentDot >= 328) {  // NOTE: I feel like there is an error in this implementation; it offsets the coarse x by 2 every frame.
		if (currentDot % 8 == 0) {
			this->incrementScrolling();
		}
	}
}

// TODO: Refactor
void PPU::performDataFetches() {
	// TODO: Give this variable and function a better name.
	uint8_t cycleCounter = (this->dot - 1) % 8;  // This variable ranges from 0 to 7 and represents cycles 8, 16, 24... 256.

	// The shift registers are shifted to the right by 1 every data-fetching cycle.
	this->patternShiftRegisterLow >>= 1;
	this->patternShiftRegisterHigh >>= 1;
	this->attributeShiftRegisterLow >>= 1;
	this->attributeShiftRegisterHigh >>= 1;

	/* TEST: transfering attribute shifters every cycle. (didnt seem to work)
	if (this->attributeLatchLow) {
		int c = 0;
	}
	this->attributeShiftRegisterLow |= this->attributeLatchLow << 8;
	this->attributeShiftRegisterHigh |= this->attributeLatchHigh << 8;
	*/

	// The pattern and attribute shifters are reloaded on cycle counter 0. (NOTE: It might transfer on cycleCounter 7, judging from frame timing diagram.)
	if (cycleCounter == 0) {
		this->patternShiftRegisterLow |= this->patternLatchLow << 8;
		this->patternShiftRegisterHigh |= this->patternLatchHigh << 8;
		this->attributeShiftRegisterLow |= this->attributeLatchLow * 0xff;
		this->attributeShiftRegisterHigh |= this->attributeLatchHigh * 0xff;
	}

	const uint16_t FIRST_NAMETABLE_ADDR = 0x2000, PATTERN_TABLE_ADDR = getBit(this->registers.PPUCTRL, 4) << 12;
	auto z = getBit(this->registers.PPUCTRL, 4);
	auto y = z << 12;
	uint16_t addr;  // Variable to hold addresses which may be used. NOTE: Might be removed.
	uint16_t a, b, c;  // TODO: rename these.
	// Now, we will load the latches every other cycle.
	switch (cycleCounter) {
	case(1):  // Fetching nametable byte.
		// Using the internal v register to define the scroll.
		// NOTE: For now, we are only rendering the first nametable w/o proper mirroring or any scrolling.
		//addr = FIRST_NAMETABLE_ADDR + getBits(this->v, 0, 4) + getBits(this->v, 5, 9) * 32;
		a = getBits(this->v, 0, 4);
		b = getBits(this->v, 5, 9);
		//c = b << 1;  // When we get the coarse y and store it in b, they are offset by 5, so 0bYYYYY00000. First, we shift it right by 5 to account for this, then multiply by 32
		// To account for the length (in tiles) of the x-axis. Simplified, this is the same as not shifting at all.
		addr = FIRST_NAMETABLE_ADDR + a + b;
		this->nametableByteLatch = this->databus.read(addr);		

		if (this->nametableByteLatch == 0x01) {  // TODO: Fix weird bug where VRAM has some erroneous values (such as 0x3f at 0x2a).
			c = 0;
		}
		break;
	case(3):  // Fetching attribute table byte.
		addr = FIRST_NAMETABLE_ADDR + 0x3c0; // 0x3c0 = Size of nametable; after this is the attribute table.
		// We can form the attribute address via the following format:
		// NN1111YYYXXX; where NN is the nametable select, 1111 a constant offset, YYY and XXX the high bits of their coarse offsets.
		// NOTE: I am  not 100% sure if this will be a bug, but this uses the new value of v instead of the old one. If buggy behavior arises, look here.
		a = getBits(this->v, 2, 4) >> 2;  // Coarse x (ignore lower 2 bits)
		b = getBits(this->v, 7, 9) >> 7;  // Coarse y (ignore lower 2 bits)
		// a and b form the x, y coordinate for a nametable tile.

		//c = b / 4;  // Note: Do NOT "simplify" this into b * 2; b / 4 truncates decimal values, "snapping" the coarse y to the correct attribute row..
		b *= 8;
		//b = a / 4;

		addr += a + b;  // Offset the address given what part of the nametable we are 
		c = this->databus.read(addr);  // TODO: Debug; I think there is a bug here but it isn't clear.
		
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

		if (this->nametableByteLatch == 0x01) {  // TODO: Fix weird bug where VRAM has some erroneous values (such as 0x3f at 0x2a).
			a = 0;
		}
		a = getBit(this->v, 6) << 1 + getBit(this->v, 1);  // This selects which part of the byte given the quadrant this tile is in.
	
		// Finally we move the bits into the appropriate latches.
		this->attributeLatchLow = getBit(c, 2 * a);
		this->attributeLatchHigh = getBit(c, 2 * a + 1);
		break;
	case(5):  // Fetching pattern table tile low.
		// Using the nametable byte, we will grab the associated pattern.
		// NOTE: For now, we will use a specific pattern table.
		addr = PATTERN_TABLE_ADDR + this->nametableByteLatch * 16;  // Each pattern is 16 bytes large.
		// We will get a different pair of bytes from the pattern depending on the current line (e.g. get 1st pair on line 0, 2nd pair on line 1...).
		a = this->getLineOn();
		b = a % 240;
		c = 2 * b;
		addr += this->scanline % 8;  // Selecting the line. TODO: There is almost certainly a better way to fetch pattern table bytes than this.
		// 
		this->patternLatchLow = this->databus.read(addr);

		break;
	case(7):  // Fetching pattern table tile high.
		addr = PATTERN_TABLE_ADDR + this->nametableByteLatch * 16;  // Each pattern is 16 bytes large.
		addr += this->scanline % 8;  // Selecting the line.
		this->patternLatchHigh = this->databus.read(addr + 0x8);  // The upper bits are offset by 8 from the low bits.

		if (this->nametableByteLatch == 0x24 && (this->patternLatchLow || this->patternLatchHigh)) {
			c = 0;
		}
		break;
	default:
		break;
	}
}

void PPU::updateBeamLocation() {
	// First, check if we need to skip dot 340, line 261 (only do this on odd frames).
	if (this->frameCount & 0b1) {  // Check if the frame is odd
		if (this->dot == 340 && this->scanline == 261) {
			this->dot = 0;
			this->scanline = 0;
			return;
		}
	}

	// Then, increment dot and scanline (the latter only if dot > 340).
	if (++this->dot > 340) {
		this->dot = 0;
		if (++this->scanline > 261) {
			this->scanline = 0;
			++this->frameCount;
		}
	}
}

int PPU::getLineOn() const {
	int lineOn = (this->cycleCount / PPU_CYCLES_PER_LINE) % TOTAL_LINES;
	if (this->scanline == 0) {
		int a = 0;
	}
	//return lineOn;
	return this->scanline;
}

int PPU::getDotOn() const {
	auto a = this->cycleCount % PPU_CYCLES_PER_LINE;
	//return this->cycleCount % PPU_CYCLES_PER_LINE;  // From what I read so far, I think cycles 1-256 (0BI) draw each dot, so I will use that to determine the dot we are on.
	return this->dot;
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

void PPU::drawPixel() {
	if (this->graphics == nullptr) {  // If we are not given a graphics object, do not attempt to draw.
		std::cout << "Warning: No graphics object provided for PPU; no output will be displayed." << std::endl;
		return;
	}

	// Now, figure out the color we need to draw.
	uint16_t colorKey = 0;
	// First, copy the emphasis values from PPUMASK to the color key.
	copyBits(colorKey, 6, 8, (uint16_t)this->registers.PPUMASK, 5, 7);

	// Now we have to find the color index for this pixel. For now, we will find out the color index for the background.
	
	// Getting the high and low bits of the pattern at the appropriate point.
	const uint16_t backgroundPaletteAddress = 0x3f00;  // The starting address for the background palette.
	uint16_t addr = backgroundPaletteAddress;
	// Indexing the palette.
	if (this->scanline == 0x12 * 8 && this->dot == 0x9 * 8) {
		int c = 0;
	}

	addr += getBit(this->patternShiftRegisterHigh, (7 - this->x)) << 1;
	addr += getBit(this->patternShiftRegisterLow, (7 - this->x));
	// Indexing which palette we want.
	addr += 4 * (getBit(this->attributeShiftRegisterLow, (7 - this->x)) + (getBit(this->attributeShiftRegisterHigh, (7 - this->x)) << 1));
	
	// Now, using this addr, we will get the color located at that addr.
	colorKey |= this->databus.read(addr);

	if (this->dot < 0x100 && this->scanline < 0xf0) {  // Do not draw past dot 255 or scanline 240
		// NOTE: Temporary solution; I am not sure why but using Graphics::drawPixel results in a slight barber pole effect instead of a stable picture.

		this->graphics->drawSquare(this->paletteMap.at(colorKey), this->dot, this->scanline, 1);
	}
}
