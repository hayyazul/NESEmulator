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



class NESDebug : public NES {
public:
	NESDebug();
	//NESDebug(NESDatabus* databus, _6502_CPU* CPU, RAM* ram, Memory* vram, PPU* ppu);
	~NESDebug();

	// Returns whether a frame has just finished drawing (e.g. when the PPU is at line 239, dot 256).
	NESCycleOutcomes executeMachineCycle() override;

	// --- Debug Methods ---
	bool frameFinished() const;  // Returns true when the frame is finished drawing.
	
	PPUInternals getPPUInternals() const;  // Returns a struct containing the value of every internal (excludes VRAM and CHRDATA) element of the PPU.

	// Instances of the debugger versions of the databus and CPU.
	PPUDebug debugPPU;
	CPUDebugger debugCPU;
	NESDatabus debugDatabus;
	Memory debugVRAM{ VRAM_SIZE };
	RAM debugRAM;
	Memory debugMemory{ 0x10000 };

};
