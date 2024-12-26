// CPU.h : Emulates the Ricoh 6502 CPU at the instruction level
// (the number of cycles is still taken into account during execution).

#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <map>
#include <iostream>
#include <iomanip>

#include "../databus/databus.h"
#include "../instructions/instructions.h"
#include "../globals/helpers.hpp"

constexpr int numOfInstructions = 1;
const uint16_t RESET_VECTOR_ADDRESS = 0xfffc;

enum CPUCycleOutcomes {
	FAIL,  // Occurs when an attempt to execute an illegal opcode is made.
	PASS,  // Occurs when we are still waiting for the opcode's cycle counter to tick down.
	INSTRUCTION_EXECUTED  // Occurs when an instruction was executed this cycle.
};

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

	// Prints the contents of the register in a human-readable format.
	void dumpContents() {

		// Now the registers (excluding flags)
		std::cout << "A: " << displayHex(this->A, 2)
			<< ", X: " << displayHex(this->X, 2)
			<< ", Y: " << displayHex(this->Y, 2)
			<< ", SP: " << displayHex(this->SP, 2)
			<< ", PC: " << displayHex(this->PC, 4) << " | Flags: C=";

		// Lastly, the flags.
		std::cout << this->getStatus('C')
			<< ", Z=" << this->getStatus('Z')
			<< ", I=" << this->getStatus('I')
			<< ", D=" << this->getStatus('D')
			<< ", V=" << this->getStatus('V')
			<< ", N=" << this->getStatus('N');
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
		uint8_t statusMask = 0b00000000;
		//                     NV1BDIZC  
		if (status == 'C') {  // Carry flag
			statusMask = 0b00000001;
		} else if (status == 'Z') {  // Zero flag
			statusMask = 0b00000010;
		} else if (status == 'I') {  // Interrupy Disable flag (see NESdev for more info)
			statusMask = 0b00000100;
		} else if (status == 'D') {  // Decimal flag (see NESdev for more info)
			statusMask = 0b00001000;
		} else if (status == 'B') {  // B flag (see NESdev for more info)
			statusMask = 0b00010000;
		// There is a bit between the B and V bits which is always 1; make sure to set it to 1 in 
		// instructions which affect the status flags.
		} else if (status == 'V') {  // Carry flag  // IMPORTANT TODO: Why is the carry flag here and up there at the same time?
			statusMask = 0b01000000;
		} else if (status == 'N') {  // Decimal flag (present but disabled)
			statusMask = 0b10000000;
		}
		return statusMask;
	}
	
};

class _6502_CPU {
public:
	_6502_CPU();
	_6502_CPU(DataBus* databus);
	~_6502_CPU();

	/* void attach
	Sets the internal pointer to a databus to this new databus.
	*/
	virtual void attach(DataBus* databus);

	/* bool executeCycle
	Executes a machine cycle which for now is equivalent to one cpu cycle. Returns True if the cycle has been successful, false if
	otherwise (e.g. illegal opcode).
	*/
	virtual CPUCycleOutcomes executeCycle();

	// Makes a request for an IRQ interrupt; ignored if the Interrupt Disable Flag (I) is set to 1.
	virtual void requestInterrupt();

	// Makes a request for an NMI; unignorable. This only makes a request if the parameter is true AND the previous value of the parameter when it was last called was false.
	virtual void requestNMI(bool request);

	// Gets whether this is a get (false) or put (true) cycle.
	bool getCycleType() const;

	// Turns the current cycle from get to put or from put to get.
	void alternateCycle();

	/* void reset
	Resets the CPU, which involves setting the PC to the location indicated by the reset vector and decrementing the stack pointer by 3.
	
    NOTE: I am not sure if that last part about decrementing the SP is entirely correct.

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

protected:
    // Map between bytes and their associated opcodes. NOTE: I might want to try getting this into some constant global.
	std::map<uint8_t, Instruction> INSTRUCTION_SET;

	Registers registers;

	bool interruptRequested;  // Whether a REQUEST for an interrupt has been made.
	bool performInterrupt;  // Whether to PERFORM an interrupt in the current cpu cycle.

	bool nmiRequested;  // Whether a REQUEST for an NMI has been made.
	bool lastNMISignal;  // The last NMI signal; so if the PPU is on Vblank, this gets set to true. This also prevents another NMI from being requested (assuming the PPU's Vblank status is still true).

	bool getOrPutCycle;  // Bool indicating whether the current cycle is a get (false) or a put (true) cycle. Starts as a get cycle, alternates back and forth every cycle. 
	long unsigned int totalCyclesElapsed = 0;  // Total CPU cycles elapsed since startup. 
	unsigned int opcodeCyclesElapsed = 0;  // A cycle counter that is present since the CPU began executing a given instruction. Resets when it reaches the number of cycles for a given instruction.
	unsigned int currentOpcodeCycleLen = 0;  // The number of cycles the current opcode uses.

	void executeOpcode(uint8_t opcode);

	void performInterruptActions();
	
	void performNMIActions();

private:
	DataBus* databus;
	void setupInstructionSet();
};