// ppu.h - The PPU class; connected to CHR data, it outputs video based on that data.
// The video output itself is handled w/ some other module. The PPU also has registers
// which are connected to the CPU at addresses 0x2000 to 0x2007 inclusive and 0x4014.
#pragma once

//  TODO: Make the nametables memory mappings.
//  TODO: Fix and organize timing control.

#include <map>
#include "../memory/memory.h"
#include "../databus/ppuDatabus.h"
#include "../graphics/graphics.h"

const int VRAM_SIZE = 0x800;  // The size of the internal VRAM that the NES has in bytes.

// Lines are 0-based indexed from 0 to 260; lines 0 to 239 are visible, 240 is the post-render line, 241 to 260 are the VBlank lines, and 261 is the pre-render line.
// The following values are the lines where each respective scanline group STARTS.
const int VISIBLE_LINE = 0;  // AKA the first render line.
const int LAST_RENDER_LINE = 239;
const int POST_RENDER_LINE = 240;
const int FIRST_VBLANK_LINE = 241;
const int LAST_VBLANK_LINE = 260;
const int PRE_RENDER_LINE = 261;

const int TOTAL_LINES = 262;
const int LINES_BETWEEN_VBLANKS = TOTAL_LINES;  // There are 262 lines total, so the interval between Vblanks is 262.
const int PPU_CYCLES_PER_LINE = 341;  // Self-explanatory.

// Collection of latches involved in rendering.
struct Latches {
	// Internal latches which will transfer to the shift registers every 8 cycles.  
	uint16_t patternLatchLow, patternLatchHigh;
	bool attributeLatchLow, attributeLatchHigh;
	uint8_t nametableByteLatch;

	Latches();
	~Latches();
};

// Collection of shift registers involved in rendering.
struct ShiftRegisters {
	uint16_t patternShiftRegisterLow, patternShiftRegisterHigh;  // Contains appropriate pattern bits.
	uint8_t attributeShiftRegisterLow, attributeShiftRegisterHigh;  // Contains the attribute data for the given tile.

	ShiftRegisters();
	~ShiftRegisters();

	ShiftRegisters& operator>>=(const int& n);

	void transferLatches(Latches latches);
};

// Position of the PPU's "beam", i.e. what dot and cycle it is on.
struct PPUPosition {
	PPUPosition();
	~PPUPosition();

	int scanline, dot;

	// Updates the position of the scanline and or dot; returns whether a new frame has started.
	bool updatePosition(bool oddFrame);
	
	// Returns whether the position is in Vblank, pass true to check if it has only reached it (on the first dot of Vblank).
	bool inVblank(bool reached = false) const;  
	// Returns whether the position is in Hblank, pass true to check if it has only reached it (on the first dot of Hblank).
	bool inHBlank(bool reached = false) const;
	// Returns whether the position is in prerender, pass true to check if it has only reached it (on the first dot of prerender).
	bool inPrerender(bool reached = false) const;
	// Returns whether the position is on the render lines, pass true to check if it has only reached it (on the first dot of the render lines).
	bool onRenderLines(bool reached = false) const;  	// Whether the PPU is in the rendering region (does not mean the PPU is rendering, that also depends on whether rendering is enabled).

	// NOTE: I might make the scanline and dot private, add getters, and make an inflexible interface to modify their value.
};

class PPU {
public:
	PPU();
	PPU(Memory* VRAM, Memory* CHRDATA);
	~PPU();

	void attachGraphics(Graphics* graphics);
	void attachVRAM(Memory* vram);
	void attachCHRDATA(Memory* chrdata);

	// Executes a single PPU cycle.
	void executePPUCycle();

	// Using the address and data given, writes to and performs some operations relating to a given PPU register.
	// Tries to return old value written to, but this is not always possible so do not rely on the output of this function for that functionality.
	uint8_t writeToRegister(uint16_t address, uint8_t data);
	// Using the address given, reads from and performs some operations relating to a given PPU register.
	uint8_t readRegister(uint16_t address);

	// When the PPU wants to request an NMI, this function returns true.
	bool requestingNMI() const;

	// Whether the PPU wants to halt the CPU and do OAM DMA copying.
	bool reqeuestingDMA();
	uint8_t getDMAPage() const;  // Gets the page to perform the copying on.

protected:

	// reachedPrerender returns whether the PPU is at dot 1 (0BI) of the pre-render line.
	bool isRendering() const;  	// Whether the PPU is currently rendering.

	// Updates the PPUSTATUS register; should be called every PPU cycle. This might be removed or put into a larger function which updates the internal states of the PPU.
	void updatePPUSTATUS();
	void updateRenderingRegisters();  // Updates internal registers for rendering; should only be called if rendering is enabled.
	void performDataFetches();  // Performs the data fetches associated w/ cycles 1-256 on the rendering lines.

	// Updates the location of the scanning beam. NOTE: might remove.
	void updateBeamLocation();
	void incrementScrolling(bool axis = false);  // Increments the x and v registers, handling overflow for both appropriately. false - x axis, true - y axis.

	void drawPixel();  // Draws a pixel to graphics depending on the internal register values. (see the NESdev's page on PPU Rendering for details).
	
	const std::map<uint16_t, uint32_t> paletteMap;

	// Internal latches which will transfer to the shift registers every 8 cycles.  
	Latches latches;

	// Internal shift registers relating to drawing.
	ShiftRegisters shiftRegisters;

	Graphics* graphics;  // A pointer to the graphics object which will be drawn to.

	/*
	VERY IMPORTANT NOTE FOR INES FILES!!!
	...the PPU memory map (look it up in the wiki) is similarly divided into palettes, 
	pattern, name and attribute tables. Only the pattern tables are "ready to use" 
	on power up (only if the cart uses CHR-ROM, though!), everything else must be 
	initialized by the program itself, because it's all RAM.
	*/
	
	PPUPosition beamPos;  // Represents the current dot and scanline 
	int cycleCount, frameCount;  // NOTE: there might be issues with overflow; look into this risk more.
	
	// VRAM (NOTE: for now) should contain 2kb (or 0x800 bytes) which span 0x1000 addresses (0x2000 to 0x2fff)
	// CHRDATA is mapped to some rom or ram data spanning from 0x0000 to 0x2000 (they are the two pattern tables; each of which is 0x1000 bytes big).
	// 0x3000 to 0x3eff mirror 0x2000 to 0x2eff; it goes unused.
	// 0x3f00 to 0x3fff maps to the palette control.

	PPUDatabus databus;  // Databus which maps to VRAM, CHRDATA, and palette RAM. This is NOT connected to OAM, which has its own memory.
	Memory* VRAM;  // TODO: replace memory w/ a specific child of it designed for VRAM; allow this to be remapped by the cartridge.
	Memory* CHRDATA;  // TODO: implement
	Memory paletteControl;
	Memory OAM;  // Internal memory inside the PPU which contains 256 bytes, 4 bytes defining 1 sprite for 64 sprites.
	
	bool requestingOAMDMA;  // Whether the PPU is requesting an OAMDMA. This gets set true when a write to OAMDMA occurs and false when the requestingDMA method is called and this is true.
	uint8_t dmaPage;

	// Internal registers.
	bool w;  // 1 bit
	uint16_t v, t;  // 15 bits
	uint8_t x;  // 3 bits
	// Misc.
	uint8_t control;  // Written via PPUCTRL.
	uint8_t mask;  // Written via PPUMASK.
	uint8_t status;  // Read via PPUSTATUS
	uint8_t OAMAddr;

	uint8_t PPUDATABuffer;  // A buffer to hold the value at the last VRAM address; used in conjunction w/ reads on PPUDATA.
	uint8_t ioBus;  // The I/O data bus; this must be at least partly emulated to make some PPU register read/write operations work.
	/* This is mainly because of 1. reads to write - only registers should return the I / O bus value; 2. PPUSTATUS returns the first 5 bits of this bus.
	 On the actual console, the values in this bus decay, but I won't emulate that (for now?).
	
	"PPU open bus holds the last value the CPU wrote to or read from any PPU register (CPU $2000-3FFF). Even if writing to a read-only PPU register, 
	the PPU's internal bus is still updated with that value. When you read from a PPU register, you get this open bus value in these 3 cases:

	1. Reading from $2000, $2001, $2003, $2004 (except on the 2C02G-0 and H-0), $2005, or $2006
	2. In bits 4-0 when reading from $2002
	3. In bits 7-6 when reading from palette RAM ($3F00-3FFF) via $2007 on 2C02G-0 and H-0.

	PPU open bus eventually decays, but it takes at least a few milliseconds after the value was last set. Reading PPU open bus does not set the value, 
	so for example, repeatedly reading $2002 will keep bits 7-5 active while bits 4-0 eventually decay."

	 - Fiskbit, NesDev forums admin
	*/	

};