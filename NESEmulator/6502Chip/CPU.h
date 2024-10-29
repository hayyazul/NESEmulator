#pragma once
#include <cstdint>
#include <vector>
#include <array>

#include "../databus/databus.h"
#include "../instructions/instructions.h"

constexpr int numOfInstructions = 1;

struct Registers {
	uint8_t accumulator;
	uint8_t status;
	uint16_t programCounter;
	uint8_t stackPtr;
	uint8_t Xindex;
	uint8_t Yindex;

	Registers() :
		accumulator(0),
		status(0),
		programCounter(0),
		stackPtr(0),
		Xindex(0),
		Yindex(0)
	{};

	inline bool getStatus(const char status) {
		uint8_t statusMask = this->getStatusMask(status);
		return (statusMask & this->status);
	}

	inline void setStatus(const char status, bool value) {
		uint8_t statusMask = this->getStatusMask(status);
		if (value) {
			this->status |= statusMask;
		} else {
			this->status &= ~statusMask;
		}
	}

private:
	inline uint8_t getStatusMask(const char status) {
		uint8_t statusMask = 0b0000000;
		if (status == 'N') {
			statusMask = 0b1;
		}
		else if (status == 'V') {
			statusMask = 0b10;
		}
		else if (status == 'B') {
			statusMask = 0b100;
		}
		else if (status == 'I') {
			statusMask = 0b1000;
		}
		else if (status == 'Z') {
			statusMask = 0b10000;
		}
		else if (status == 'C') {
			statusMask = 0b100000;
		}
		return statusMask;
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

class _6502_CPU {
public:
	_6502_CPU();
	~_6502_CPU();

	void executeCycle();

private:

	std::array<Instruction, numOfInstructions> instructionSet;
	Registers registers;
	DataBus* databus;

	long unsigned int totalCyclesElapsed = 0;
	unsigned int opcodeCyclesElapsed = 0;
};