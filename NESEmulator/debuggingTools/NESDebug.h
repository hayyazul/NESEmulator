// testOpcodes.h - A series of functions and classes to test the opcodes. This file provides a decoder for the .json test formats and way to automatically test them.
#pragma once

#include "../6502Chip/CPU.h"
#include "../NESEmulator.h"
#include "../databus/nesDatabus.h"
#include "../debuggingTools/CPUAnalyzer.h"
#include "../debuggingTools/PPUDebug.h"

#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <algorithm>

/* TODO:
* - Save states; a state of all values inside the nes at a given point in time, 
* able to be reloaded at whim, serialized, and deserialized.
* - 
*/

inline uint32_t memAddrToiNESAddr(uint16_t memAddr) {
	return (memAddr - 0x8000 + 0x10) % 0x4000;
};


// Displays a dump of memory
void displayMemDump(std::vector<uint8_t>& dump, uint16_t startAddr, uint16_t endAddr, unsigned int bytesPerRow = 16);

void displayMemDumpLine(std::vector<uint8_t>& dump, uint16_t startAddr, uint16_t endAddr, unsigned int row, unsigned int bytesPerRow);

// Contains all updateable internals of the NES.
struct NESInternals {
	std::string name;  // Name of the internals instance.
	PPUInternals ppuInternals;
	CPUInternals cpuInternals;
	OAMDMAUnit oamDMAUnit;

	bool scheduleHalt;  // Whether to halt the CPU next cycle.  
	bool haltCPUOAM;  // Whether the CPU is halted for OAMDMA.
	unsigned long long totalMachineCycles;
	
	RAM ram;

	NESInternals() {};
	NESInternals(std::string name) : name(name) {};
	NESInternals(std::filesystem::path filepath);  // Deserializes the given filepath (assuming it is a .nesstate file).
	~NESInternals() {};

	int getMachineCycles() const;

	// Returns a string meant to be written to a file containing various parts of the internals seperated by new lines.
	std::string getSerialFormat() const;
};

class NESDebug : public NES {
public:
	NESDebug();
	//NESDebug(NESDatabus* databus, _6502_CPU* CPU, RAM* ram, Memory* vram, PPU* ppu);
	~NESDebug();

	// Returns whether a frame has just finished drawing (e.g. when the PPU is at line 239, dot 256).
	NESCycleOutcomes executeMachineCycle() override;

	// Same as above, but executes the specified number of machine cycles (can make it based on CPU cycles). FAILS upon an invalid numCycle count.
	NESCycleOutcomes executeNMachineCycles(unsigned long long numCycles, bool CPUBased);

	// Same as above, but executes until the NES reaches the given (machine by default, but can be CPU) cycle. FAILS if given an invalid cycle count or a cycle count which was already passed.
	NESCycleOutcomes executeTillCycle(unsigned long long cycleCount, bool CPUBased);

	// --- Debug Methods ---
	bool frameFinished() const;  // Returns true when the frame is finished drawing.

	// Gets total number of machine cycles this NES has iterated.
	unsigned long long getNumCycles() const;
	unsigned long long getNumCPUCycles() const;

	void getNESInternals(NESInternals& internals);

	// Loads CPU, PPU, DMA, RAM, and VRAM internals
	void loadInternals(NESInternals internals);
	PPUInternals getPPUInternals() const;  // Returns a struct containing the value of every internal (excludes VRAM and CHRDATA) element of the PPU.
	CPUInternals getCPUInternals() const;
	OAMDMAUnit getOAMDMAUnit() const;  // Returns a reference to the OAM DMA unit inside the NES.
	void getRAM(RAM& RAM);  // Copies the current values inside RAM to the memory indicated by the argument.
	void getVRAM(Memory& VRAM);  // Like getRAM but gets VRAM instead. NOTE: Might be removed.

	// Instances of the debugger versions of the databus and CPU.
	PPUDebug debugPPU;
	CPUDebugger debugCPU;
	NESDatabus debugDatabus;
	Memory debugVRAM{ VRAM_SIZE };
	RAM debugRAM;
	Memory debugMemory{ 0x10000 };

};
