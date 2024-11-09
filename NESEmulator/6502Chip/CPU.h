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
const uint16_t RESET_VECTOR_ADDRESS = 0xfffc;

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
		S(0b00100000),  // The 3rd bit is always 1.
		PC(0),
		SP(0),  // The stack pointer starts at 0x1ff and ends at 0x100. So lower values <-> higher in the stack.
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

	bool operator==(const Registers& otherRegisters) {
		return  otherRegisters.A == this->A && 
				otherRegisters.S == this->S && 
				otherRegisters.SP == this->SP && 
				otherRegisters.PC == this->PC && 
				otherRegisters.X == this->X && 
				otherRegisters.Y == this->Y;
	}

private:
	inline uint8_t getStatusMask(const char status) {
		// See NESdev for implementation details.
		uint8_t statusMask = 0b0000000;
		if (status == 'C') {  // Carry flag
			statusMask = 0b1;
		} else if (status == 'Z') {  // Zero flag
			statusMask = 0b10;
		} else if (status == 'I') {  // Interrupy Disable flag (see NESdev for more info)
			statusMask = 0b100;
		} else if (status == 'D') {  // Decimal flag (see NESdev for more info)
			statusMask = 0b1000;
		} else if (status == 'B') {  // B flag (see NESdev for more info)
			statusMask = 0b10000;
		} else if (status == 'C') {  // Carry flag
			statusMask = 0b1000000;
		} else if (status == 'D') {  // Decimal flag (present but disabled)
			statusMask = 0b1000000;
		}
		return statusMask;
	}
	
};

class _6502_CPU {
public:
	_6502_CPU();
	_6502_CPU(DataBus* databus);
	~_6502_CPU();

	/* bool executeCycle
	Executes a machine cycle which for now is equivalent to one cpu cycle. Returns True if the cycle has been successful, false if
	otherwise (e.g. illegal opcode).
	*/
	bool executeCycle();

	/* void reset
	Resets the CPU, which involves:
	
	Setting flags
	TODO: reset PPU and APU flags and variables.
	*/
	void reset();

	/* void powerON
	Initializes CPU values to those expected when powering the console up.

	It does many things similarly to reset, but does a few extra things,
	including clearing all flags.
	TODO: reset PPU and APU flags and variables.
	*/
	void powerOn();

public:
	// Temporarily public; with the exception of the first, never use these for non-debugging spurposes. 
	void executeOpcode(uint8_t opcode);

	// Direct memory operations. Peek = Getter, Poke = Setter, mem = memory. Serve a purely debug role.
	uint8_t memPeek(uint16_t memoryAddress);
	Registers registersPeek();
	void memPoke(uint16_t memoryAddress, uint8_t val);
	void registersPoke(Registers registers);

	// Searches for a memory value and gets the first address which satisifies this condition. Returns true if found, false if not.
	// Range (optional) is inclusive.
	bool memFind(uint8_t value, uint16_t& address, int lowerBound = -1, int upperBound = -1);

	std::array<uint8_t, 0x100> dumpStack();

private:

	void setupInstructionSet();

	// Map between bytes and their associated opcodes.
	std::map<uint8_t, Instruction> instructionSet;
	Registers registers;
	DataBus* databus;

	long unsigned int totalCyclesElapsed = 0;  // Total CPU cycles elapsed since startup.
	unsigned int opcodeCyclesElapsed = 0;  // A cycle counter that is present since the CPU began executing a given instruction. Resets when it reaches the number of cycles for a given instruction.
	unsigned int currentOpcodeCycleLen = 0;  // The number of cycles the current opcode uses.
};