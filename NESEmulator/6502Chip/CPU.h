// CPU.h : Emulates the Ricoh 6502 CPU at the instruction level
// (the number of cycles is still taken into account during execution).

#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <map>

#include "../databus/databus.h"
#include "../instructions/instructions.h"

constexpr int numOfInstructions = 1;

// NOTE: Might replace get/set status with bool values.
struct Registers {
	uint8_t A;  // Accumulator
	uint8_t S;  // Status (NV1BDIZC; see getStatusMask for what these letters mean)
	uint16_t PC;  // Program Counter
	uint8_t SP;  // Stack pointer
	uint8_t X;  // X index 
	uint8_t Y;  // Y index

	Registers() :
		A(0),
		S(0),
		PC(0),
		SP(0xFF),  // The stack pointer starts at 0x1ff and ends at 0x100. So lower values <-> higher in the stack.
		X(0),
		Y(0)
	{};

	inline bool getStatus(const char status) {
		uint8_t statusMask = this->getStatusMask(status);
		return (statusMask & this->S);
	}

	inline void setStatus(const char status, bool value) {
		uint8_t statusMask = this->getStatusMask(status);
		if (value) {
			this->S |= statusMask;
		} else {
			this->S &= ~statusMask;
		}
	}

private:
	inline uint8_t getStatusMask(const char status) {
		// See NESdev for implementation details.
		uint8_t statusMask = 0b0000000;
		if (status == 'N') {  // Negative flag
			statusMask = 0b1;
		}
		else if (status == 'V') {  // oVerflow flag
			statusMask = 0b10;
		}
		else if (status == 'B') {  // B flag (see NESdev for more info)
			statusMask = 0b100;
		}
		else if (status == 'I') {  // Interrupt disable (see NESdev for more info)
			statusMask = 0b1000;
		}
		else if (status == 'Z') {  // Zero flag
			statusMask = 0b10000;
		}
		else if (status == 'C') {  // Carry flag
			statusMask = 0b100000;
		}
		return statusMask;
	}
	
};

class _6502_CPU {
public:
	_6502_CPU();
	_6502_CPU(DataBus* databus);
	~_6502_CPU();

	void executeCycle();

public:
	// Temporarily public
	void executeOpcode(uint8_t opcode);

	uint8_t memPeek(uint16_t memoryAddress);
	Registers registersPeek();
	void memPoke(uint16_t memoryAddress, uint8_t val);

private:

	void setupInstructionSet();

	std::map<uint8_t, Instruction> instructionSet;
	Registers registers;
	DataBus* databus;

	long unsigned int totalCyclesElapsed = 0;
	unsigned int opcodeCyclesElapsed = 0;
};