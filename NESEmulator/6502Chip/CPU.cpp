#include "CPU.h"

_6502_CPU::_6502_CPU() : databus(nullptr) {
	this->setupInstructionSet();
}

_6502_CPU::_6502_CPU(DataBus* databus) : databus(databus) {
	this->setupInstructionSet();
}

_6502_CPU::~_6502_CPU() {}

void _6502_CPU::executeCycle() {
	// First check if the number of cycles elapsed corresponds with the number of cycles the instruction takes up. If so, execute the next instruction.
	if (this->opcodeCyclesElapsed == this->currentOpcodeCycleLen) {
		this->currentOpcodeCycleLen = 0;  // Reset the opcode counter.
		uint8_t opcode = this->databus->read(this->registers.PC);  // Get the next opcode.
		this->currentOpcodeCycleLen = this->instructionSet[opcode].cycleCount;  // Get how many cycles this opcode will be using.

		this->executeOpcode(opcode);
		this->registers.PC += this->instructionSet[opcode].numBytes * ~this->instructionSet[opcode].modifiesPC;  // Only move the program counter forward if the instruction does not modify the PC.
	}
	++this->opcodeCyclesElapsed;
	++this->totalCyclesElapsed;
}

void _6502_CPU::executeOpcode(uint8_t opcode) {
	this->instructionSet[opcode].performOperation(this->registers, *this->databus);
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
		addrModes::immediate,
		2,
		2);
	this->instructionSet[0x85] = Instruction(ops::STA,
		addrModes::zeropage,
		2,
		3);
	this->instructionSet[0x81] = Instruction(ops::STA,
		addrModes::indirectX,
		2,
		6);
}

// Cycle operations