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

// TODO: make it cycle accurate (e.g., make it so 1 CPU cycle != 1 instruction).
// Make it return an error code instead of a bool.
CPUCycleOutcomes CPUDebugger::executeCycle() {
	// Execute a CPU cycle, but record some info first.

	// Save the old value of the record actions flag.
	bool oldRecordActions = this->databus->getRecordActions();
	this->databus->setRecordActions(false);
	
	// Data that needs to be recorded before execution.
	uint8_t opcode = this->databus->read(this->registers.PC);
	if (!INSTRUCTION_SET.count(opcode)) {
		this->databus->setRecordActions(oldRecordActions);
		return FAIL;
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
	this->databus->setRecordActions(oldRecordActions);
	CPUCycleOutcomes outcome = _6502_CPU::executeCycle();

	// TODO: Fix the bug where the constant 1 is allowed to be unset; it should not, the CPu should check if it is unset and re-set it to 1.
	// DEBUG:
	// Validating the constant 1 in the 5th bit:
	if (!(0b00100000 & this->registers.S)) {
		this->registers.S |= 0b00100000;
	}

	// Get the last info needed to describe an executed instruction: how many databus actions it took.
	unsigned int numOfDatabusActions = this->databus->getNumActions() - lastMemOpsSize;
	if (outcome == INSTRUCTION_EXECUTED) {
		ExecutedInstruction* executedInstruction  = new ExecutedInstruction(
			opcode,
			instruction,
			numOfDatabusActions,
			oldRegisters,
			operands,
			this->cycleActions.size(),
			this->totalCyclesElapsed - 1);
		
		this->executedInstructions.push_back(*executedInstruction);
		this->cycleActions.emplace_back(true, executedInstruction);
	} else {
		this->cycleActions.emplace_back();
	}

	return outcome;
}

bool CPUDebugger::pcAt(uint16_t address) {
	return this->registers.PC == address;
}

void CPUDebugger::attach(DebugDatabus* databus) {
	this->databus = databus;
	_6502_CPU::attach(databus);
}

CPUCycleOutcomes CPUDebugger::undoCPUCycle() {
	// Return true if we can undo a cycle; false if not. We can undo if the # of cycles elapsed is > 0.
	CPUCycleOutcomes outcome = PASS;
	if (this->totalCyclesElapsed) {
		// Check if there have been executed instructions before accessing any elements in it.
		--this->totalCyclesElapsed;
		--this->opcodeCyclesElapsed;
		CycleAction lastAction = this->cycleActions.back();
		if (lastAction.instructionExecuted) {  // Check if we are on the cycle count of the last instruction; if so, undo this one.
				this->undoInstruction(*lastAction.executedInstruction);
				outcome = INSTRUCTION_EXECUTED;
		}

		this->cycleActions.pop_back();
	} else {
		outcome = FAIL;
	}

	return outcome;
}
/*
bool CPUDebugger::undoCyclesUntilInstruction() {
	CPUCycleOutcomes outcome = PASS;
	while (outcome == PASS) {  // Undo CPU cycles until an instruction is undone or we run out of instructions to undo.
		outcome = this->undoCPUCycle();
	}

	return outcome == INSTRUCTION_EXECUTED;
}
*/
bool CPUDebugger::undoInstruction(ExecutedInstruction instruction) {
	// First check if there are any executed instructions.
	if (this->executedInstructions.size() == 0) {
		std::cout << "Warning: there have been no executed instructions; no instruction has been undone." << std::endl;
		return false;
	}

	// Then, undo the mem
	for (int i = 0; i < instruction.numOfActionsInvolved; ++i) {
		this->databus->undoMemAction();
	}
	this->registers = instruction.oldRegisters;
	this->executedInstructions.pop_back();


	if (this->executedInstructions.size() != 0) {
		ExecutedInstruction lastInstruction = this->executedInstructions.back();
		this->opcodeCyclesElapsed = lastInstruction.numOfCycles;
		this->currentOpcodeCycleLen = lastInstruction.numOfCycles;
	} else {
		this->opcodeCyclesElapsed = 0;
		this->currentOpcodeCycleLen = 0;
	}
	return true;
}

bool CPUDebugger::undoIRQ() {
	return false;
}

bool CPUDebugger::undoNMI() {
	return false;
}

ExecutedInstruction CPUDebugger::getLastExecutedInstruction() {
	if (!this->executedInstructions.size()) {
		std::cout << "Warning: the stack of executed instructions is empty. No changes have been made." << std::endl;
		return ExecutedInstruction();
	}

	return this->executedInstructions.back();
}

bool CPUDebugger::correspondsWithLog(std::vector<ExecutedOpcodeLogEntry>& log, bool checkLast) {

	int entriesToCheck = min(log.size(), this->executedInstructions.size());

	// Important note: the log may NOT correspond w/ the list of executed instructions, even if 
	if (log.size() < this->executedInstructions.size()) {  // TODO: reword this.
		std::cout << "Warning: The log to compare with has fewer elements than the number of executed instructions. The program can not validate the log any further (though it can validate the first this->executedInstructions.size() instructions)." << std::endl;
	}

	if (checkLast) {
		int lastEntryIdx = entriesToCheck - 1;
		if (lastEntryIdx < 0) {
			return true;  // If there are no entries to check, default to true.
		}
		return log.at(lastEntryIdx) == this->executedInstructions.at(min(lastEntryIdx, this->executedInstructions.size() - 1));
	}

	for (int i = 0; i < entriesToCheck; ++i) {
		if (log.at(i) != this->executedInstructions.at(i)) {
			return false;
		}
	}

	return true;
}

std::vector<CycleAction> CPUDebugger::getCycleActions() {
	return this->getCycleActions(this->cycleActions.size());
}

// Returns vector of executed instructions, from least recent to most/
std::vector<CycleAction> CPUDebugger::getCycleActions(unsigned int lastN) {
	std::vector<CycleAction> cycleActions;

	// How far back the first element is.
	int firstElementOffset = lastN < this->cycleActions.size() ? lastN : this->cycleActions.size();

	if (this->cycleActions.size()) {
		for (unsigned int i = 0; i < firstElementOffset; ++i) {
			cycleActions.push_back(this->cycleActions.at(this->cycleActions.size() - firstElementOffset + i));
		}
	}

	return cycleActions;
}

void CPUDebugger::clearExecutedInstructions() {
	this->cycleActions = std::vector<CycleAction>();
	this->databus->clearRecordedActions();
}


std::vector<ExecutedInstruction> CPUDebugger::getExecutedInstructions() {
	return this->getExecutedInstructions(this->executedInstructions.size());
}

std::vector<ExecutedInstruction> CPUDebugger::getExecutedInstructions(unsigned int lastN) {
	std::vector<ExecutedInstruction> executedInstructions;

	// How far back the first element is.
	int firstElementOffset = lastN < this->executedInstructions.size() ? lastN : this->executedInstructions.size();

	if (this->executedInstructions.size()) {
		for (unsigned int i = 0; i < firstElementOffset; ++i) {
			executedInstructions.push_back(this->executedInstructions.at(this->executedInstructions.size() - firstElementOffset + i));
		}
	}

	return executedInstructions;
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
