// ppuDatabus.h - A data bus for the PPU.
#pragma once

#include "databus.h"
#include "../memory/memory.h"
//#include "../ppu/ppu.h"

class PPUDatabus : public DataBus {
public:
	PPUDatabus();
	~PPUDatabus();

	// Sets the internal pointer to a Memory module to the given pointer.
	void attachVRAM(Memory* vram);
	void attachCHRDATA(Memory* chrData);
	void attachPalette(Memory* paletteRAM);
	
	// Basic, fundamental read/write operations.
	virtual uint8_t read(uint16_t address) override;  // Returns the memory located at that address.
	virtual uint8_t write(uint16_t address, uint8_t value) override;  // Returns the old value at the given spot.

private:
	void adjustAddress(uint16_t& address);  // Adjusts the address to make it within range of the memory module it maps to.
	Memory* getMemoryModule(uint16_t address);  // Gets the pointer to the relevant memory module given an address

	Memory* VRAM;  
	Memory* CHRDATA;  
	Memory* paletteControl;
};