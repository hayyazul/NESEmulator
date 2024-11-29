// NESEmulator.h : Program entry.
#pragma once

#include <iostream>
#include <fstream>
#include <iomanip>

#include "6502Chip/CPU.h"
#include "memory/memory.h"
#include "databus/databus.h"
#include "loadingData/parseNESFiles.h"
#include "memory/ram.h"
#include "databus/nesDatabus.h"

// TODO: Add poweron and reset features.
class NES {
public:
	NES();
	NES(NESDatabus* databus, _6502_CPU* CPU, RAM* ram, Memory* vram, PPU* ppu);
	~NES();

	void attachRAM(RAM* ram);
	void attachCartridgeMemory(Memory* memory);
	void attachDataBus(NESDatabus* databus);
	void attachVRAM(Memory* vram);

	void powerOn();  // Performs all the actions the NES should perform upon a power on.
	void reset();  // Performs the actions the NES should perform when reset.

	void loadROM(const char* fileName);

	void run();  // Placeholder until everything is implemented properly.

public:
	// Temporarily public for debugging purposes.
	virtual bool executeMachineCycle();  // For now, 1 Machine Cycle = 1 CPU Cycle. This is not how it is in the actual implementation.

protected:
	
	/* void loadData
	Given an NESFile, loads the data into memory.

	See https://www.nesdev.org/wiki/CPU_memory_map
	*/
	void loadData(NESFileData file);

	// Cartridge ROM usually though not always starts at this address.
	const uint16_t CART_ROM_START_ADDR = 0x8000;  // CART = Cartridge, ADDR = Address

	Memory* memory;  // Contains cartridge data, etc.
	
	_6502_CPU* CPU;
	RAM* ram;
	NESDatabus* databus;

	Memory* VRAM;
	PPU* ppu;
	

	unsigned long int totalMachineCycles = 0;
};