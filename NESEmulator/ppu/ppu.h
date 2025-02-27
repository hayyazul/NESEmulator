// ppu.h - The PPU class; connected to CHR data, it outputs video based on that data.
// The video output itself is handled w/ some other module. The PPU also has registers
// which are connected to the CPU at addresses 0x2000 to 0x2007 inclusive and 0x4014.
#pragma once

//  TODO: Make the nametables memory mappings.
//  TODO: Fully implement PPUSTATUS
//  TODO: Bugs:
//			- Make sure NMI behavior is correct (it likely isn't at the moment).
//			- Some tiles have the wrong attribute (the attribute for the tile to the right).

#include <map>
#include <array>
#include "../memory/memory.h"
#include "../memory/secondaryOAM.h"
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

// Collection of latches involved in rendering the background.
struct BackgroundLatches {
	// Internal latches which will transfer to the shift registers every 8 cycles.  
	uint16_t patternLatchLow, patternLatchHigh;
	bool attributeLatchLow, attributeLatchHigh;
	uint8_t nametableByteLatch;

	BackgroundLatches();
	~BackgroundLatches();
};

struct SpriteShiftUnit {
	uint16_t patternShiftRegisterLow, patternShiftRegisterHigh;
	int attributeBits;  // Ranges from 0-3 and indicates the palette index.
	int x;  // The x coordinate of where the sprite is located; used to know when to start rendering this sprite.

	SpriteShiftUnit();
	~SpriteShiftUnit();

	// Fetches the 2 bits in the low and high shift registers w/ an offset indicating which of the lower 8 bits to get.
	uint8_t getPattern(int x) const;
	uint8_t getAttribute() const;

	// Important Note: These shift operators do not work in the traditional sense. See methods for more details.
	SpriteShiftUnit& operator>>=(int n);
	SpriteShiftUnit& operator<<=(int n);
};

// Collection of shift registers involved in rendering the background..
struct BackgroundShiftRegisters {
	uint16_t patternShiftRegisterLow, patternShiftRegisterHigh;  // Contains appropriate pattern bits.
	uint16_t attributeShiftRegisterLow, attributeShiftRegisterHigh;  // Contains the attribute data for the given tile.

	BackgroundShiftRegisters();
	~BackgroundShiftRegisters();

	// Fetches the 2 bits in the low and high shift registers w/ an offset indicating which of the lower 8 bits to get.
	uint8_t getPattern(int x) const;
	uint8_t getAttribute(int x) const;

	BackgroundShiftRegisters& operator>>=(const int& n);

	void transferLatches(BackgroundLatches latches);
};

// NOTE: Might use this for BackgroundShiftRegisters.


/*
struct SpriteLatches {
	uint16_t patternLatchLow, patternLatchHigh;
	bool attributeLatchLow, attributeLatchHigh;

	SpriteLatches();
	~SpriteLatches();

	// NOTE: Might make a method to access secondaryOAM and get the data from there, given a sprite as input.
};
*/

struct SpriteShiftRegisters {
	std::array<SpriteShiftUnit, 8> shiftRegisters;  // 8 shift registers for 8 sprites in secondary OAM.

	SpriteShiftRegisters();
	~SpriteShiftRegisters();

	// Returns a reference to a shift unit at the given index.
	SpriteShiftUnit& at(int idx);
	
	// Gets the 2 pattern and attribute bits associated w/ a single sprite. This sprite is determined based on the priority, location, and transparency of other sprites and their pixels.
	uint8_t getPattern(int x);
	uint8_t getAttribute(int x);

	// Performs a shift operation for all shift units/sprites.
	void operator>>=(const int& n);
	void operator<<=(const int& n);

	// Shifts the registers associated w/ a given sprite right once. NOTE: Likely to be removed.
	void shiftRegister(int sprite);
};

// Position of the PPU's "beam", i.e. what dot and cycle it is on.
struct PPUPosition {
	PPUPosition();
	~PPUPosition();

	int scanline, dot;

	// Updates the position of the scanline and or dot; returns whether a new frame has started.
	bool updatePosition(bool oddFrame);

	// Checks if the specified axis is within a given range (both ends inclusive).
	bool dotInRange(int lowerBound, int upperBound) const;
	bool lineInRange(int lowerBound, int upperBound) const;
	bool inRange(int lineLowerBound, int lineUpperBound, int dotLowerBound, int dotUpperBound) const;

	// Returns whether the position is in Vblank, pass true to check if it has only reached it (on the first dot of Vblank).
	bool inVblank(bool reached = false) const;  
	// Returns whether the position is in Hblank, pass true to check if it has only reached it (on the first dot of Hblank).
	bool inHblank(bool reached = false) const;
	// Returns whether the position is in prerender, pass true to check if it has only reached it (on the first dot of prerender).
	bool inPrerender(bool reached = false) const;
	// Returns whether the position is in render, pass true to check if it has only reached it (on the first dot of render).
	// Differs from onRenderLines by excluding dots in Hblank (except dot 257) and dot 0.
	bool inRender(bool reached = false) const;
	// Returns whether the position is on the render lines, pass true to check if it has only reached it (on the first dot of the render lines).
	bool onRenderLines(bool reached = false) const;  	// Whether the PPU is in the rendering region (does not mean the PPU is rendering, that also depends on whether rendering is enabled).
	// Returns whether the position is in the true VBlanking period (which starts 1 line and 1 dot before the PPU performs actions relating to the start of Vblank).
	// This is used to determine whether to continue datafetching the various bytes or not.
	bool inTrueVblank(bool reached = false) const;

	// NOTE: I might make the scanline and dot private, add getters, and make an inflexible interface to modify their value.
};

/* NOTE: Potential implementation; still judging whether this is a good idea.
struct PPUInternalRegisters {
	PPUInternalRegisters();
	~PPUInternalRegisters();

	uint16_t v, t;
	uint8_t x;
	bool w;

	uint8_t getX();
	uint8_t getX(bool coarse);

	uint8_t getY();
	uint8_t getY(bool coarse);

};*/

// Describes the different states of the OAM sprite evaulation
enum SpriteEvaluationState {
	INIT,  // Cycles 1-64, initializes OAM to this.
	// Next 4 are between cycles 65-256.
	FINDING_SPRITES,  // 2.1 on NESDev, secondary OAM is not full and is looking for sprites to fill it up.
	COPY_SPRITE_DATA,  // 2.1.a on NESDev, when the next line intersects w/ the sprite, the PPU copies the rest of the sprite data to secondary OAM.
	INCREMENT_CHECK,  // 2.2 on NESDev, decides what to do given how many sprites have been inserted. 
	// If less than 8, return to FINDING_SPRITES, if all sprites have been evaluated, go to 
	SPRITE_OVERFLOW,  // 2.3 on NESDev, the period when evaluating sprite overflow.
	POINTLESS_COPYING  // 2.4 on NESDev, the period when the PPU tries and purposely fails to copy OAM sprite n and byte 0 into the next free slot in secondary OAM.
	// The next 2 are between cycles 257 and 340+0
	// TODO
};

enum SpriteByteOn {
	Y_COORD,
	TILE_IDX,  // AKA the pattern index.
	ATTRIBUTES,
	X_COORD
};

// A struct defining what kind of byte OAMAddr is on (note that if OAMAddr is initialized w/ a an address s.t. 4!|OAMAddr, it will treat the first byte it sees as a Y-coordinate anyway).
class SpriteEvalCycle {
public:
	SpriteEvalCycle();
	~SpriteEvalCycle();

	// Iterates the byteType w/ wrapping.
	void operator++();

	bool onByte(SpriteByteOn sbo) const;
	bool onState(SpriteEvaluationState ses) const;
	void setState(SpriteEvaluationState ses);
	// Resets the byte on and the evaluation state to their defaults (Y_COORD and INIT)
	void reset();


	SpriteByteOn byteType;
	SpriteEvaluationState evalState;
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

	// Whether the PPU is currently rendering. The PPU is considered rendering when within the picture region and background and or sprite rendering is enabled. While nothing is rendered on the pre-render line, 
	// it may be considered as part of rendering region for this function (used to determine whether to update rendering registers).
	bool isRendering(bool includePrerender = false) const;  	

	// Updates the PPUSTATUS register; should be called every PPU cycle. This might be removed or put into a larger function which updates the internal states of the PPU.
	void updatePPUSTATUS();
	

	// Updates the location of the scanning beam. NOTE: might remove.
	void updateBeamLocation();
	void updateRenderingRegisters();  // Updates internal registers for rendering; should only be called if rendering is enabled.
	
	// Performs a pattern fetch given some inputs. Note that line is a value expected to be between 
	void fetchPatternData(uint8_t patternID, bool table, bool high, int line, uint16_t& pattern, bool flipH = false, bool flipV = false);
	void performBackgroundFetches();  // Performs the data fetches associated w/ cycles 1-256 on the rendering lines.
	void performSpriteEvaluation();
	void transferSpriteData();  // Transfers the sprite data from 2ndOAM to their respective shift registers.
	void updateSpriteShiftRegisters();  // Updates the sprite shift registers. NOTE: Might remove.

	void incrementScrolling(bool axis = false);  // Increments the x and v registers, handling overflow for both appropriately. false - x axis, true - y axis.

	// Gets the color index associated w/ the background given the values in the current shift and internal registers.
	uint8_t getBGColor();
	// Gets the color index associated w/ the sprite given the values in the current shift and internal registers.
	uint8_t getSpriteColor();

	void drawPixel();  // Draws a pixel to graphics depending on the internal register values. (see the NESdev's page on PPU Rendering for details).
	

	const std::map<uint16_t, uint32_t> paletteMap;

	// Internal latches which will transfer to the shift registers every 8 cycles.  
	BackgroundLatches latches;
	// Internal shift registers relating to drawing.
	BackgroundShiftRegisters backgroundShiftRegisters;
	// NOTE: There is little information on where sprite patterns are located; I am assuming there are 8 shift registers. 
	// This part of the emulator is highly speculative due to lack of information.
	SpriteShiftRegisters spriteShiftRegisters;  
	
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
	Memory* CHRDATA; 
	Memory paletteControl;
	Memory OAM;  // Internal memory inside the PPU which contains 256 bytes, 4 bytes defining 1 sprite for 64 sprites.
	SecondaryOAM secondaryOAM;  // used for rendering sprites.
	SpriteEvalCycle spriteEvalCycle;
	
	//uint8_t spriteIdx;  // Part of sprite evaluation.
	//uint8_t erroneousByteIdx;  // Part of sprite evaluation. During sprite overflow check, the PPU erroneously increments the address of OAM it is using to access by 5 instead of 4.

	// NOTE: I likely will refactor sprite evaluation. In particular, I might try a state machine. The current implementation is also a state machine, but it is cobbled together very poorly.

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
	uint8_t ioBus;  // The I/O data bus; this must be at least partly emulated to make some PPU register read/write operations work. It is also used for primary-to-secondary OAM data transfer.
	
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