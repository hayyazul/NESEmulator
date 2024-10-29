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
	uint8_t immediate(DataBus& databus, Registers& registers);
}