#include "CPU.h"

_6502_CPU::_6502_CPU() {
	std::vector<CycleOperation> cycles;
	cycles.push_back(addNextByteToAcc);
	this->instructions[0] = Instruction{ cycles, IMMEDIATE, 2 };
}

_6502_CPU::~_6502_CPU() {}

void _6502_CPU::executeCycle() {
	// First check if the number of cpu cycles elapsed since the opcode was started surpassed the current instruction.
	// In this case, iterate the program counter appropriately and set the new instruction.
	if (this->opcodeCyclesElapsed > this->currentInstruction->numOfCycles) {
		this->registers.programCounter += this->currentInstruction->numOfCycles;
		this->currentInstruction = this->decodeOpcode(this->databus->read(this->registers.programCounter));
	} else {
		++this->opcodeCyclesElapsed;
		this->currentInstruction->cycleOperations[this->opcodeCyclesElapsed](this->registers, this->databus);
	}
}

Instruction* _6502_CPU::decodeOpcode(uint8_t opcode) {
	return &this->instructions[0];
}


// Cycle operations

void addNextByteToAcc(Registers& registers, DataBus* databus) {
	uint8_t memoryValue = databus->read(registers.programCounter + 1);
	registers.accumulator += memoryValue;
}