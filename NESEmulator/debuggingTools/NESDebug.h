// testOpcodes.h - A series of functions and classes to test the opcodes. This file provides a decoder for the .json test formats and way to automatically test them.
#pragma once

#include "../6502Chip/CPU.h"
#include "../NESEmulator.h"

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

	_6502_CPU* CPU_ptr = &this->CPU;
	Memory* memory_ptr = &this->memory;
	DataBus* databus_ptr = &this->databus;

	// Debug variables
	unsigned long int totalMachineCycles = 0;
	unsigned long int CYCLE_LIMIT = 100000;  // Referring to the machine cycle.

	void setStdValue(uint8_t val);  // Initializes all memory values to the one given here.

	// Direct memory operations. Peek = Getter, Poke = Setter, mem = memory. Serve a purely debug role.
	uint8_t memPeek(uint16_t memoryAddress);

	// TODO: output a dump of the memory.
	//void memDump(const char* filename);
	
	// TODO: directly affect memory instead of using the CPU's debugger poke/peek functions.
	Registers registersPeek();
	bool registersPeek(char c);

	void registersPoke(Registers registers);

	void memPoke(uint16_t memoryAddress, uint8_t val);

	// Searches for a memory value and gets the first address which satisifies this condition. Returns true if found, false if not.
	// Range (optional) is inclusive.
	bool memFind(uint8_t value, uint16_t& address, int lowerBound = -1, int upperBound = -1);

private:
};
