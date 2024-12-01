// testOpcodes.h - A series of functions and classes to test the opcodes. This file provides a decoder for the .json test formats and way to automatically test them.
#pragma once

#include "../6502Chip/CPU.h"
#include "../NESEmulator.h"
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

// Struct describing what happened in a single machine cycle (made equal to 1 PPU cycle).
struct MachineAction {  // NOTE: 48 bytes is a lot.
	long int cycle;
	bool NMIRequested;
	bool instructionExecuted;  // Whether an instruction has been executed.
	// A PPU cycle always has occured.

	std::stack<PPUActions> ppuActions;  // NOTE: For now, this has nothing.
	//std::stack<DatabusAction> databusActions;

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
	~NESDebug();

	// Executes machine cycles until either a failure or an instruction is executed.
	NESCycleOutcomes executeInstruction();

	NESCycleOutcomes executeMachineCycle() override;
	MachineAction undoMachineCycle();  // Undos the last machine cycle; returns it.

	std::vector<MachineAction> getMachineActions();
	std::vector<MachineAction> getMachineActions(int numOfActions);

	bool setRecord(bool record);  // Sets the CPU and Databus recorders to the given value; returns the old value.
	bool getRecord(bool record) const;  // Gets the values of the record flags.
	void clearRecord();  // Deletes the store executed instructions vector and memops stack.

	void setStdValue(uint8_t val);  // Initializes all memory values to the one given here.

	CPUDebugger* getCPUPtr();
	PPUDebug* getPPUPtr();

	// Direct memory operations. Peek = Getter, Poke = Setter, mem = memory. Serve a purely debug role.
	uint8_t memPeek(uint16_t memoryAddress);

private:
	void performMachineAction(MachineAction machineAction, bool reverseOrder=false);

	std::vector<MachineAction> machineActions;

	// Instances of the debugger versions of the databus and CPU.
	DebugDatabus databusInstance;
	CPUDebugger cpuInstance;
	PPUDebug ppuInstance;

	// Cartridge ROM usually though not always starts at this address.
	const uint16_t CART_ROM_START_ADDR = 0x8000;  // CART = Cartridge, ADDR = Address

	// Debug variables
	unsigned long int CYCLE_LIMIT = 1000000000;  // Referring to the machine cycle.

};
