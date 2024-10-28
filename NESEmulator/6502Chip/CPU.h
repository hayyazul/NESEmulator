#pragma once
#include <cstdint>
#include <vector>
#include <array>

#include "../databus/databus.h"

constexpr int numOfInstructions = 1;

// A cycle operation is an operation an instruction performs per cycle.
typedef void (*CycleOperation)(Registers& registers, DataBus* databus);

struct Registers {
	uint8_t accumulator;
	uint8_t status;
	uint16_t programCounter;
	uint8_t stackPtr;
	uint8_t Xindex;
	uint8_t Yindex;

	inline bool getStatus(const char* status) {
		uint8_t statusMask = 0b0000000;
		if (status == "negative") {
			statusMask = 0b1;
		}
		else if (status == "overflow") {
			statusMask = 0b10;
		}
		else if (status == "break") {
			statusMask = 0b100;
		}
		else if (status == "interrupt disable") {
			statusMask = 0b1000;
		}
		else if (status == "zero") {
			statusMask = 0b10000;
		}
		else if (status == "carry") {
			statusMask = 0b100000;
		}

		return (statusMask & this->status);
	}
};

enum AddressingModes {
	IMPLICIT,
	ACCUMULATOR,
	IMMEDIATE,
	ZERO_PAGE,
	ZERO_PAGE_X,
	ZERO_PAGE_Y,
	RELATIVE,
	ABSOLUTE,
	ABSOLUTE_X,
	ABSOLUTE_Y,
	INDIRECT,
	INDIRECT_X,
	INDIRECT_Y
};

// An opcode which fetches data every cycle or performs an operation.
struct Instruction {
	// Array of pointers to sub operations which it performs in order.
	std::vector<CycleOperation> cycleOperations;

	// Size of the instruction in bytes; instructions are made up of different bytes.
	int instructionSize;
	int numOfCycles;

	AddressingModes addressingMode;

	Instruction() {}
	Instruction(std::vector<CycleOperation> cycleOperations, AddressingModes addressingMode, int instructionSize) {
		this->cycleOperations = cycleOperations;
		this->instructionSize = instructionSize;
		this->numOfCycles = this->cycleOperations.size();
		this->addressingMode = addressingMode;
	}

	~Instruction() {}
};

class _6502_CPU {
public:
	_6502_CPU();
	~_6502_CPU();

	void executeCycle();

private:

	Instruction* decodeOpcode(uint8_t opcode);

	std::array<Instruction, 1> instructions;	
	Instruction* currentInstruction;

	Registers registers;
	DataBus* databus;

	long unsigned int totalCyclesElapsed = 0;
	unsigned int opcodeCyclesElapsed = 0;
};