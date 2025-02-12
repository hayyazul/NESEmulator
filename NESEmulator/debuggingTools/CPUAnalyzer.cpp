#include "CPUAnalyzer.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <minmax.h>
#include "../memory/memory.h"
#include "../input/cmdInput.h"
#include "../loadingData/parseLogData.h"

CPUDebugger::CPUDebugger() : _6502_CPU(this->databus) {
}

CPUDebugger::CPUDebugger(DebugDatabus* databus) : databus(databus), _6502_CPU(databus) {}

CPUDebugger::~CPUDebugger() {}

// Make it return an error code instead of a bool.
CPUCycleOutcomes CPUDebugger::executeCycle() {
	
	// Data that needs to be recorded before execution.
	uint8_t opcode = this->databus->read(this->registers.PC);
	if (!INSTRUCTION_SET.count(opcode)) {
		return FAIL;
	}

	Instruction* instruction = &INSTRUCTION_SET.at(opcode);
	Registers oldRegisters = this->registers;

	// If there are any operands, get those operands.
	uint8_t operands[2] = { 0, 0 };
	for (int i = 1; i < instruction->numBytes; ++i) {
		operands[i - 1] = this->databus->read(this->registers.PC + i);
	}
	CPUCycleOutcomes outcome = _6502_CPU::executeCycle();

	// TODO: Fix the bug where the constant 1 is allowed to be unset; it should not, the CPU should check if it is unset and re-set it to 1.
	// DEBUG:
	// Validating the constant 1 in the 5th bit:
	if (!(0b00100000 & this->registers.S)) {
		this->registers.S |= 0b00100000;
	}
	return outcome;
}

bool CPUDebugger::pcAt(uint16_t address) {
	return this->registers.PC == address;
}

void CPUDebugger::attach(DataBus* databus) {
	this->databus = databus;
	_6502_CPU::attach(databus);
}

std::vector<uint8_t> CPUDebugger::memDump(uint16_t startAddr, uint16_t endAddr) {
	std::vector<uint8_t> values;
	uint8_t value;
	for (unsigned int i = startAddr; i <= endAddr; ++i) {
		value = this->memPeek(i);
		values.push_back(value);
	}

	return values;
}

// TODO: Disable the databus debugging capabilities before executing any of these.
uint8_t CPUDebugger::memPeek(uint16_t memoryAddress) {
	uint8_t val = this->databus->read(memoryAddress);
	return val;
}

Registers CPUDebugger::registersPeek() {
	return this->registers;
}

void CPUDebugger::memPoke(uint16_t memoryAddress, uint8_t val) {
	this->databus->write(memoryAddress, val);	
}

void CPUDebugger::registersPoke(Registers registers) {
	this->registers = registers;
}

// Sets address to the first address which equals the given value if it is found; otherwise it remains unchanged.
bool CPUDebugger::memFind(uint8_t value, uint16_t& address, int lowerBound, int upperBound) {
	
	uint16_t startAddr = lowerBound == -1 ? 0 : (uint16_t)lowerBound;
	uint16_t endAddr = upperBound == -1 ? 0xffff : (uint16_t)upperBound;
	for (uint16_t i = startAddr; i <= endAddr; ++i) {
		if (this->databus->read(i) == value) {
			address = i;
			return true;
		};

		if (i == endAddr) {
			break;
		}
	}

	return false;
}

void CPUDebugger::setStdMemValue(uint8_t value) {
	for (unsigned int i = 0; i <= 0xffff; ++i) {
		this->memPoke(i, value);
	}
}

std::array<uint8_t, 0x100> CPUDebugger::dumpStack() {
	std::array<uint8_t, 0x100> stack;
	for (int i = 0; i < 0x100; ++i) {
		stack[i] = this->databus->read(i + STACK_END_ADDR);
	}
	return stack;
}
