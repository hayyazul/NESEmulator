#include "CPUAnalyzer.h"
#include <iostream>

CPUDebugger::CPUDebugger() : _6502_CPU(this->databus) {}

CPUDebugger::CPUDebugger(DebugDatabus* databus) : databus(databus), _6502_CPU(databus) {}

CPUDebugger::~CPUDebugger() {}

// TODO: make it cycle accurate (e.g., make it so 1 CPU cycle != 1 instruction).
bool CPUDebugger::executeCycle() {
	// Execute a CPU cycle, but record some info first.

	//ExecutedInstruction(uint8_t opcode, Instruction* instruction, unsigned int numActions, Registers registers, uint8_t operands[2])
	this->databus->setRecordActions(false);
	
	// Data that needs to be recorded before execution.
	uint8_t opcode = this->databus->read(this->registers.PC);
	if (!INSTRUCTION_SET.count(opcode)) {
		this->databus->setRecordActions(true);
		return false;
	}
	Instruction* instruction = &INSTRUCTION_SET.at(opcode);
	Registers oldRegisters = this->registers;

	// If there are any operands, get those operands.
	uint8_t operands[2] = { 0, 0 };
	for (int i = 1; i < instruction->numBytes; ++i) {
		operands[i - 1] = this->databus->read(this->registers.PC + i);
	}

	unsigned int lastMemOpsSize = this->databus->getNumActions();

	// Finally, execute the cycle.
	this->databus->setRecordActions(true);
	bool success = _6502_CPU::executeCycle();

	// Get the last info needed to describe an executed instruction: how many databus actions it took.
	unsigned int numOfDatabusActions = this->databus->getNumActions() - lastMemOpsSize;
	this->executedInstructions.push(ExecutedInstruction(opcode, instruction, numOfDatabusActions, oldRegisters, operands));

	return success;
}

bool CPUDebugger::undoInstruction() {
	if (this->executedInstructions.size() == 0) {
		std::cout << "Warning: there have been no executed instructions; no instruction has been undone." << std::endl;
		return false;
	}

	ExecutedInstruction lastInstruction = this->executedInstructions.top();
	this->executedInstructions.pop();
	for (int i = 0; i < lastInstruction.numOfActionsInvolved; ++i) {
		this->databus->undoMemAction();
	}
	this->registers = lastInstruction.oldRegisters;
	return true;
}

ExecutedInstruction CPUDebugger::getLastExecutedInstruction() {
	if (!this->executedInstructions.size()) {
		std::cout << "Warning: the stack of executed instructions is empty. No changes have been made." << std::endl;
		return ExecutedInstruction()
	}

	return this->executedInstructions.top();
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
	bool oldRecActionFlag = this->databus->getRecordActions();
	this->databus->setRecordActions(false);
	uint8_t val = this->databus->read(memoryAddress);
	this->databus->setRecordActions(oldRecActionFlag);
	return val;
}

Registers CPUDebugger::registersPeek() {
	return this->registers;
}

void CPUDebugger::memPoke(uint16_t memoryAddress, uint8_t val) {
	bool oldRecActionFlag = this->databus->getRecordActions();
	this->databus->setRecordActions(false);
	this->databus->write(memoryAddress, val);
	this->databus->setRecordActions(oldRecActionFlag);
}

void CPUDebugger::registersPoke(Registers registers) {
	this->registers = registers;
}

// Sets address to the first address which equals the given value if it is found; otherwise it remains unchanged.
bool CPUDebugger::memFind(uint8_t value, uint16_t& address, int lowerBound, int upperBound) {
	bool oldRecActionFlag = this->databus->getRecordActions();
	this->databus->setRecordActions(false);

	uint16_t startAddr = lowerBound == -1 ? 0 : (uint16_t)lowerBound;
	uint16_t endAddr = upperBound == -1 ? 0xffff : (uint16_t)upperBound;
	for (uint16_t i = startAddr; i <= endAddr; ++i) {
		if (this->databus->read(i) == value) {
			address = i;
			this->databus->setRecordActions(oldRecActionFlag);
			return true;
		};

		if (i == endAddr) {
			break;
		}
	}

	this->databus->setRecordActions(oldRecActionFlag);
	return false;
}

void CPUDebugger::setStdMemValue(uint8_t value) {
	for (unsigned int i = 0; i <= 0xffff; ++i) {
		this->memPoke(i, value);
	}
}

std::array<uint8_t, 0x100> CPUDebugger::dumpStack() {
	bool oldRecActionFlag = this->databus->getRecordActions();
	this->databus->setRecordActions(false);

	std::array<uint8_t, 0x100> stack;
	for (uint8_t i = 0x0; i <= 0xff; ++i) {
		stack[i] = this->databus->read(i + STACK_END_ADDR);
		if (i == 0xff) {
			break;
		}
	}

	this->databus->setRecordActions(oldRecActionFlag);
	return stack;
}
