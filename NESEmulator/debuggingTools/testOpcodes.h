// testOpcodes.h - A series of functions and classes to test the opcodes. This file provides a decoder for the .json test formats and way to automatically test them.
#pragma once

#include "../6502Chip/CPU.h"
#include "../NESEmulator.h"

#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <algorithm>
/*
// Contains some values which can be passed into the tester to see
// if the CPU is working correctly.

struct HardwareValues {
	Registers registers;
	std::set<std::pair<uint16_t, uint8_t>> memoryValues;  // Each pair is in the format <address, value>
	uint8_t standardMemoryValue;  // All addresses not listed take on this value.

	HardwareValues() {};
	HardwareValues(Registers registers, std::vector<uint16_t> addresses, std::vector<uint8_t> values) {
		registers = registers;
		if (addresses.size() != values.size()) {
			std::cout << "Warning: addresses and values vectors have mismatched dimensions of " << addresses.size() << 
				" and " << values.size() << "respectively; they have not been initialized." << std::endl;
		} else {
			std::pair<uint16_t, uint8_t> ramValuePair;

			// First, put all the values inside of the vector.
			for (unsigned int i = 0; i < addresses.size(); ++i) {
				ramValuePair.first = addresses.at(i);
				ramValuePair.second = values.at(i);
				memoryValues.insert(ramValuePair);
			}
		}
	};

	// Checks if two hardware values are equal; used for asserting equality in test cases.
	bool operator==(const HardwareValues& otherValues) {
		otherValues.registers == this->registers;
		otherValues.memoryValues == this->memoryValues;
	}
};

enum BusOperation {
	READ,
	WRITE
};

// Contains the description of a bus's behavior during one cycle.
struct BusCycleActivity {
	uint16_t address;
	uint8_t value;
	BusOperation operation;
};

struct TestSuite {
	std::string name;  // 3 byte hex header.
	HardwareValues initValues;
	HardwareValues finalValues;  // What the values should be by the end of the test.
	std::vector<BusCycleActivity> busActivity;  // What the databus should be doing, in order.
};
*/

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
