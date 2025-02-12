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


inline uint32_t memAddrToiNESAddr(uint16_t memAddr) {
	return (memAddr - 0x8000 + 0x10) % 0x4000;
};

// NOTE: This will be repurposed.
// Struct describing what happened in a single machine cycle (made equal to 1 PPU cycle).
struct MachineAction {  // NOTE: 48 bytes is a lot.
	long int cycle;
	bool NMIRequested;
	bool instructionExecuted;  // Whether an instruction has been executed.
	// A PPU cycle always has occured.

	//std::stack<PPUActions> ppuActions;  // NOTE: For now, this has nothing.
	//std::stack<DatabusAction> databusActions;

	MachineAction() : cycle(-1), NMIRequested(false), instructionExecuted(false) {};
	//MachineAction(long int cycle, bool NMIRequested, bool instructionExecuted, std::stack<PPUActions> ppuActions) : cycle(cycle), NMIRequested(NMIRequested), instructionExecuted(instructionExecuted), ppuActions(ppuActions) {};

	void print() {
		std::cout << "Cycle: " << std::dec << this->cycle << std::endl;
		std::cout << "   CPU Cycle (rounded down): " << std::dec << this->cycle / 3 << std::endl;
		std::cout << "NMI Requested: " << btos(this->NMIRequested) << std::endl;
		std::cout << "Instruction Executed: " << btos(this->instructionExecuted) << std::endl;
	}
};

class NESDebug : public NES {
public:
	NESDebug();
	//NESDebug(NESDatabus* databus, _6502_CPU* CPU, RAM* ram, Memory* vram, PPU* ppu);
	~NESDebug();

	// Returns whether a frame has just finished drawing (e.g. when the PPU is at line 239, dot 256).
	NESCycleOutcomes executeMachineCycle() override;

	// Extra debug variables and methods.
	bool frameFinished() const;  // Returns true when the frame is finished drawing.

	// Instances of the debugger versions of the databus and CPU.
	PPUDebug debugPPU;
	CPUDebugger debugCPU;
	NESDatabus debugDatabus;
	Memory debugVRAM{ VRAM_SIZE };
	RAM debugRAM;
	Memory debugMemory{ 0x10000 };

};
