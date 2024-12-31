// ppu.h - The PPU class; connected to CHR data, it outputs video based on that data.
// The video output itself is handled w/ some other module. The PPU also has registers
// which are connected to the CPU at addresses 0x2000 to 0x2007 inclusive and 0x4014.
#pragma once

#include "../memory/memory.h"
#include "../databus/ppuDatabus.h"

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


struct PPURegisters {  // NOTE: Some of these registers are not modified in the read/write operations of the PPU. Implementation wise, these do not exist, so I will likely remove these.
	// W - Write Only; R - Read Only; RW - Read and Write; xN - Times you need to read/write to go through the entire register.
	/* W 0x2000
	7  bit  0
	---- ----
	VPHB SINN
	|||| ||||
	|||| ||++- Base nametable address
	|||| ||    (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
	|||| |+--- VRAM address increment per CPU read/write of PPUDATA
	|||| |     (0: add 1, going across; 1: add 32, going down)
	|||| +---- Sprite pattern table address for 8x8 sprites
	||||       (0: $0000; 1: $1000; ignored in 8x16 mode)
	|||+------ Background pattern table address (0: $0000; 1: $1000)
	||+------- Sprite size (0: 8x8 pixels; 1: 8x16 pixels – see PPU OAM#Byte 1)
	|+-------- PPU master/slave select
	|          (0: read backdrop from EXT pins; 1: output color on EXT pins)
	+--------- Vblank NMI enable (0: off, 1: on)
	*/
	uint8_t PPUCTRL;
	uint8_t PPUMASK;  // W 
	uint8_t PPUSTATUS;  // R
	uint8_t OAMADDR;  // W
	uint8_t OAMDATA;  // RW
	uint16_t PPUSCROLL;  // Wx2
	uint16_t PPUADDR;  // Wx2
	uint8_t PPUDATA;  // RW
	uint8_t OAMDMA;  // W
/*
*/
};

// GENERAL TODO:
//  - Implement the following PPU register writes/reads.
//     - PPUMASK
//     - PPUSTATUS
//		  - PPU open bus or 2C05 PPU identifier; see nesdev for more info; this only affects first 5 bits.
//		  - Sprite overflow and Sprite 0 hit flags.
//     - OAMADDR
//     - OAMDATA
//     - PPUSCROLL
//     - OAMDMA

class PPU {
public:
	PPU();
	PPU(Memory* VRAM, Memory* CHRDATA);
	~PPU();

	void attachVRAM(Memory* vram);
	void attachCHRDATA(Memory* chrdata);

	// Executes a single PPU cycle.
	void executePPUCycle();

	// NOTE: Might move these into PPURegisters.
	// Using the address and data given, writes to and performs some operations relating to a given PPU register.
	// Returns the old value of the byte written to; for values w/ >8 bits, the portion written to is returned.
	uint8_t writeToRegister(uint16_t address, uint8_t data);
	// Using the address given, reads from and performs some operations relating to a given PPU register.
	uint8_t readRegister(uint16_t address);

	// When the PPU wants to request an NMI, this function returns true.
	bool requestingNMI() const;

	// Whether the PPU wants to halt the CPU and do OAM DMA copying.
	bool reqeuestingDMA();
	uint8_t getDMAPage() const;  // Gets the page to perform the copying on.

protected:

	// inVblank returns whether the PPU is inbetween dot 1 (0BI) of line 241 (this is when vblank starts) and dot 340 of line 260.
	bool inVblank() const;
	bool reachedVblank() const;  // This instead only checks whether the PPU is exactly on dot 1 of line 241.

	// reachedPrerender returns whether the PPU is at dot 1 (0BI) of the pre-render line.
	bool reachedPrerender() const;

	// Whether the PPU

	// Whether the PPU is currently rendering.
	bool inRendering() const;

	// Updates the PPUSTATUS register; should be called every PPU cycle.
	void updatePPUSTATUS();

	// Gets the scanline the PPU is on; NOTE: might make this protected or even public.
	int getLineOn() const;

	// Gets the dot the PPU is on; NOTE: this is subject to change in the future; the current method of finding the dot is not based on the wiki.
	int getDotOn() const;

	/*
	VERY IMPORTANT NOTE FOR INES FILES!!!
	 
	...the PPU memory map (look it up in the wiki) is similarly divided into palettes, 
	pattern, name and attribute tables. Only the pattern tables are "ready to use" 
	on power up (only if the cart uses CHR-ROM, though!), everything else must be 
	initialized by the program itself, because it's all RAM.
	*/
	
	int cycleCount;  // NOTE: there might be issues with overflow; look into this risk more.
	
	// VRAM (NOTE: for now) should contain 2kb (or 0x800 bytes) which span 0x1000 addresses (0x2000 to 0x2fff)
	// CHRDATA is mapped to some rom or ram data spanning from 0x0000 to 0x2000 (they are the two pattern tables; each of which is 0x1000 bytes big).
	// 0x3000 to 0x3eff mirror 0x2000 to 0x2eff; it goes unused.
	// 0x3f00 to 0x3fff maps to the palette control.

	PPUDatabus databus;
	Memory* VRAM;  // TODO: replace memory w/ a specific child of it designed for VRAM; allow this to be remapped by the cartridge.
	Memory* CHRDATA;  // TODO: implement
	Memory paletteControl;
	Memory OAM;  // Internal memory inside the PPU which contains 256 bytes, 4 bytes defining 1 sprite for 64 sprites.
	
	bool requestingOAMDMA;  // Whether the PPU is requesting an OAMDMA. This gets set true when a write to OAMDMA occurs and false when the requestingDMA method is called and this is true.
	uint8_t dmaPage;

	PPURegisters registers;  // External/shared registers.
	// Internal registers.
	bool w;  // 1 bit
	uint16_t v, t;  // 15 bits
	uint8_t x;  // 3 bits
	// Misc.
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

	/*
	TODO: Make the nametables memory mappings.
	
	$0000-1FFF is normally mapped by the cartridge to a CHR-ROM or CHR-RAM, often with a bank switching mechanism.
	$2000-2FFF is normally mapped to the 2kB NES internal VRAM, providing 2 nametables with a mirroring configuration controlled by the cartridge, but it can be partly or fully remapped to ROM or RAM on the cartridge, allowing up to 4 simultaneous nametables.
	$3000-3EFF is usually a mirror of the 2kB region from $2000-2EFF. The PPU does not render from this address range, so this space has negligible utility.
	$3F00-3FFF is not configurable, always mapped to the internal palette control.
	*/

};