// ppu.h - The PPU class; connected to CHR data, it outputs video based on that data.
// The video output itself is handled w/ some other module. The PPU also has registers
// which are connected to the CPU at addresses 0x2000 to 0x2007 inclusive and 0x4014.
#pragma once

#include "../memory/memory.h"

struct PPURegisters {
	// W - Write Only; R - Read Only; RW - Read and Write; xN - Times you need to read/write to go through the entire register.
	uint8_t PPUCTRL;  // W
	uint8_t PPUMASK;  // W
	uint8_t PPUSTATUS;  // R
	uint8_t OAMADDR;  // W
	uint8_t OAMDATA;  // RW
	uint16_t PPUSCROLL;  // Wx2
	uint16_t PPUADDR;  // Wx2
	uint8_t PPUDATA;  // RW
	uint8_t OAMDMA;  // W
};

class PPU {
public:
	PPU();
	~PPU();

	void attach(PPURegisters* registers);

	void attach(Memory* vram);

	// Executes a single PPU cycle.
	void executePPUCycle();

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

	// VRAM (NOTE: for now) should contain 2kb (or 0x800 bytes) which span 0x1000 addresses.
	// CHRDATA is mapped to some rom or ram data.
	// 0x3000 to 0x3eff mirror 0x2000 to 0x2eff; it goes unused.
	// 0x3f00 to 0x3fff maps to the palette control.

	Memory* VRAM;  // TODO: replace memory w/ a specific child of it designed for VRAM; allow this to be remapped by the cartridge.
	Memory* CHRDATA;  //
	PPURegisters* registers;  // External/shared registers.
	  // Internal registers.

	/*
	$0000-1FFF is normally mapped by the cartridge to a CHR-ROM or CHR-RAM, often with a bank switching mechanism.
	$2000-2FFF is normally mapped to the 2kB NES internal VRAM, providing 2 nametables with a mirroring configuration controlled by the cartridge, but it can be partly or fully remapped to ROM or RAM on the cartridge, allowing up to 4 simultaneous nametables.
	$3000-3EFF is usually a mirror of the 2kB region from $2000-2EFF. The PPU does not render from this address range, so this space has negligible utility.
	$3F00-3FFF is not configurable, always mapped to the internal palette control.
	*/

};