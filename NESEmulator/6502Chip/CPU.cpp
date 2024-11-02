#include "CPU.h"

_6502_CPU::_6502_CPU() : databus(nullptr) {
	this->setupInstructionSet();
}

_6502_CPU::_6502_CPU(DataBus* databus) : databus(databus) {
	this->setupInstructionSet();
}

_6502_CPU::~_6502_CPU() {}

void _6502_CPU::executeCycle() {
	uint8_t opcode = 0x00;
}

void _6502_CPU::executeOpcode(uint8_t opcode) {
	uint16_t data = this->instructionSet[opcode].addressingOperation(*this->databus, this->registers);

	if (this->instructionSet[opcode].isMemOp) {
		this->instructionSet[opcode].operation(this->registers, *this->databus, data);
	}
	else {
		this->instructionSet[opcode].operation(this->registers, data);
	}
}

uint8_t _6502_CPU::memPeek(uint16_t memoryAddress) {
	return this->databus->read(memoryAddress);
}

Registers _6502_CPU::registersPeek() {
	return this->registers;
}

void _6502_CPU::memPoke(uint16_t memoryAddress, uint8_t val) {
	this->databus->write(memoryAddress, val);
}

void _6502_CPU::setupInstructionSet() {
	this->instructionSet[0xA9] = Instruction(ops::LDA,
		dataAddrOp::immediate,
		2,
		2);
	this->instructionSet[0x85] = Instruction(ops::STA,
		addr16bitOp::zeropage,
		2,
		3);
}

// Cycle operations