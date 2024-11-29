// ppu.h - The PPU class; connected to CHR data, it outputs video based on that data.
// The video output itself is handled w/ some other module. The PPU also has registers
// which are connected to the CPU at addresses 0x2000 to 0x2007 inclusive and 0x4014.
#pragma once

#include "../memory/memory.h"

struct PPURegisters {
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
//     - OAMADDR
//     - OAMDATA
//     - PPUSCROLL
//     - OAMDMA

class PPU {
public:
	PPU();
	PPU(Memory* VRAM, Memory* CHRDATA);
	~PPU();

	void attach(Memory* vram);
	void attachCHRDATA(Memory* chrdata);

	// Executes a single PPU cycle.
	void executePPUCycle();


	// NOTE: Might move these into PPURegisters.
	// Using the address and data given, writes to and performs some operations relating to a given PPU register.
	void writeToRegister(uint16_t address, uint8_t data);
	// Using the address given, reads from and performs some operations relating to a given PPU register.
	uint8_t readRegister(uint16_t address);

	// When the PPU wants to request an NMI, this function returns true.
	bool requestingNMI() const;

	// Debug Methods
	
	// Displays the nametable from VRAM using the given table id; displays nothing upon
	// invalid input.
	void displayNametable(int table = 0);

private:
	/*
	VERY IMPORTANT NOTE FOR INES FILES!!!
	 
	...the PPU memory map (look it up in the wiki) is similarly divided into palettes, 
	pattern, name and attribute tables. Only the pattern tables are "ready to use" 
	on power up (only if the cart uses CHR-ROM, though!), everything else must be 
	initialized by the program itself, because it's all RAM.
	*/
	
	int cycleCount;

	// VRAM (NOTE: for now) should contain 2kb (or 0x800 bytes) which span 0x1000 addresses.
	// CHRDATA is mapped to some rom or ram data.
	// 0x3000 to 0x3eff mirror 0x2000 to 0x2eff; it goes unused.
	// 0x3f00 to 0x3fff maps to the palette control.

	Memory* VRAM;  // TODO: replace memory w/ a specific child of it designed for VRAM; allow this to be remapped by the cartridge.
	Memory* CHRDATA;  // TODO: implement
	PPURegisters registers;  // External/shared registers.
	// Internal registers.
	bool w;  // 1 bit
	uint16_t v, t;  // 15 bits
	uint8_t x;  // 3 bits
	// Misc.
	uint8_t PPUDATABuffer;  // A buffer to hold the value at the last VRAM address; used in conjunction w/ reads on PPUDATA.

	/*
	$0000-1FFF is normally mapped by the cartridge to a CHR-ROM or CHR-RAM, often with a bank switching mechanism.
	$2000-2FFF is normally mapped to the 2kB NES internal VRAM, providing 2 nametables with a mirroring configuration controlled by the cartridge, but it can be partly or fully remapped to ROM or RAM on the cartridge, allowing up to 4 simultaneous nametables.
	$3000-3EFF is usually a mirror of the 2kB region from $2000-2EFF. The PPU does not render from this address range, so this space has negligible utility.
	$3F00-3FFF is not configurable, always mapped to the internal palette control.
	*/

};