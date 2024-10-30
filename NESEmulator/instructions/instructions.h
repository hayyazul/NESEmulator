// instructions.h : Holds all the operations and addressing modes; using
// one function from each group you can make an Instruction.

#pragma once
#include <cstdint>

enum AddressingModes;
struct Registers;
class DataBus;

typedef void(*Operation)(Registers& registers, uint8_t data);
typedef uint8_t(*AddressingOperation)(DataBus& databus, Registers& registers);

// An opcode which fetches data every cycle or performs an operation.
struct Instruction {
	Operation operation;
	AddressingOperation addressingOperation;
	unsigned int size;
	unsigned int cycleCount;

	Instruction() : size(0), cycleCount(0) {};
	Instruction(Operation op, AddressingOperation addrOp, unsigned int size, unsigned int cycleCount) : 
		operation(op),
		addressingOperation(addrOp),
		size(size), 
		cycleCount(cycleCount)
	{};

	~Instruction() {};
};

namespace operations {
	void ADC(Registers& registers, uint8_t data);
};

namespace addressingOperations {
	/*
		The following are the various addressing operations an operation can use.
	An example in assembly is provided right of all addressing modes (OPC LLLLL) 
	if applicable.
		Note that a '$' indicates the value is in hex. They are not required and do not 
	affect the addressing mode.
	*/
	
	uint8_t immediate(DataBus& databus, Registers& registers);  // LDA #$7b
	uint8_t implicit(DataBus& databus, Registers& registers);  // CLC [N/A]
	uint8_t accumulator(DataBus& databus, Registers& registers);  // LSR [N/A]
	
	uint8_t zeropage(DataBus& databus, Registers& registers);  // STX $32
	uint8_t zeropageX(DataBus& databus, Registers& registers);  // STY $32,X
	uint8_t zeropageY(DataBus& databus, Registers& registers);  // LDX $10,Y
	uint8_t absolute(DataBus& databus, Registers& registers);  // JMP $1234
	uint8_t absoluteX(DataBus& databus, Registers& registers);  // STA $3000,X
	uint8_t absoluteY(DataBus& databus, Registers& registers);  // AND $4000,Y

	// Works similar to pointers. It first goes to the given memory address, looks at the
	// byte and the byte of the next address, uses those two bytes to make a new address 
	// which it gets the value of.
	uint8_t indirect(DataBus& databus, Registers& registers);  // JMP ($4321)
	uint8_t indirectX(DataBus& databus, Registers& registers);  // STA ($40,X)
	uint8_t indirectY(DataBus& databus, Registers& registers);  // LDA ($20),Y
}