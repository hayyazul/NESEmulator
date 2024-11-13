// testOpcodes.h - A series of functions and classes to test the opcodes. This file provides a decoder for the .json test formats and way to automatically test them.
#pragma once

#include "../6502Chip/CPU.h"
#include "../NESEmulator.h"
#include "../debuggingTools/CPUAnalyzer.h"

#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <algorithm>

inline uint32_t memAddrToiNESAddr(uint16_t memAddr) {
	return (memAddr - 0x8000 + 0x10) % 0x4000;
};

class NESDebug : public NES {
public:
	NESDebug();
	~NESDebug();

	// Cartridge ROM usually though not always starts at this address.
	const uint16_t CART_ROM_START_ADDR = 0x8000;  // CART = Cartridge, ADDR = Address

	// Debug variables
	unsigned long int totalMachineCycles = 0;
	unsigned long int CYCLE_LIMIT = 100000;  // Referring to the machine cycle.

	// Sets the CPU and Databus recorders to the given value; returns the old value.
	bool setRecord(bool record);
	// Sets the CPU and Databus recorders to the given value; returns the old value.
	bool getRecord(bool record) const;

	void clearRecord();  // Deletes the store executed instructions vector and memops stack.

	void setStdValue(uint8_t val);  // Initializes all memory values to the one given here.

	CPUDebugger* getCPUPtr();

	// Direct memory operations. Peek = Getter, Poke = Setter, mem = memory. Serve a purely debug role.
	uint8_t memPeek(uint16_t memoryAddress);

	// TODO: output a dump of the memory.
	//void memDump(const char* filename);
	/*  TODO: reimplement w/ the Debug CPU.
	// TODO: directly affect memory instead of using the CPU's debugger poke/peek functions.
	Registers registersPeek();
	bool registersPeek(char c);

	void registersPoke(Registers registers);

	void memPoke(uint16_t memoryAddress, uint8_t val);

	// Searches for a memory value and gets the first address which satisifies this condition. Returns true if found, false if not.
	// Range (optional) is inclusive.
	bool memFind(uint8_t value, uint16_t& address, int lowerBound = -1, int upperBound = -1);
	*/
private:
	// Instances of the debugger versions of the databus and CPU.
	DebugDatabus databusInstance;
	CPUDebugger cpuInstance;
};
