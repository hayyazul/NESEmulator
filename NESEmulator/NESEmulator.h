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

enum NESCycleOutcomes {
	FAIL_CYCLE,  // Usually caused by an illegal instruction.
	PASS_CYCLE,  // Neither a PPU nor CPU cycle was executed.
	PPU_CYCLE,  // Only the PPU's cycle was executed.
	CPU_CYCLE,  // Only the CPU's cycle was executed (should never happen).
	BOTH_CYCLE,  // Both the PPU's and CPU's cycle was executed.
	INSTRUCTION_AND_PPU_CYCLE  // A CPU and PPU cycle was executed; the CPU had an instruction executed.
};

// TODO: Add poweron and reset features.
class NES {
public:
	NES();
	NES(NESDatabus* databus, _6502_CPU* CPU, RAM* ram, Memory* vram, PPU* ppu);
	~NES();

	virtual NESCycleOutcomes executeMachineCycle();

	void powerOn();  // Performs all the actions the NES should perform upon a power on.
	void reset();  // Performs the actions the NES should perform when reset.

	virtual void attachRAM(RAM* ram);
	virtual void attachCartridgeMemory(Memory* memory);
	virtual void attachDataBus(NESDatabus* databus);
	virtual void attachPPU(PPU* ppu);
	virtual void attachVRAM(Memory* vram);
	
	void loadROM(const char* fileName);

protected:
	
	/* void loadData
	Given an NESFile, loads the data into memory.

	See https://www.nesdev.org/wiki/CPU_memory_map
	*/
	void loadData(NESFileData file);

	// Cartridge ROM usually though not always starts at this address.
	const uint16_t CART_ROM_START_ADDR = 0x8000;  // CART = Cartridge, ADDR = Address

	// Initialized by NES; TODO: make it attach/deattachable.
	Memory* memory;  // Contains cartridge data, etc.
	
	_6502_CPU* CPU;
	RAM* ram;  // Initialized by NES; can not be remapped.
	NESDatabus* databus;

	Memory* VRAM;  // Initialized to size 0x800 by NES; TODO: make it attach/deattachable (to support mappers)
	PPU* ppu;

	unsigned long int totalMachineCycles = 0;
};