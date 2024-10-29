#include "CPU.h"

_6502_CPU::_6502_CPU() {
	this->instructionSet[0] = Instruction(operations::ADC,
		addressingOperations::immediate,
		2,
		2);
}

_6502_CPU::~_6502_CPU() {}

void _6502_CPU::executeCycle() {
	uint8_t data = this->instructionSet[0].addressingOperation(*this->databus, this->registers);
	this->instructionSet[0].operation(this->registers, data);
}

// Cycle operations