#include "CPUAnalyzer.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "../memory/memory.h"
#include "../input/cmdInput.h"

CPUDebugger::CPUDebugger() : _6502_CPU(this->databus) {}

CPUDebugger::CPUDebugger(DebugDatabus* databus) : databus(databus), _6502_CPU(databus) {}

CPUDebugger::~CPUDebugger() {}

// TODO: make it cycle accurate (e.g., make it so 1 CPU cycle != 1 instruction).
bool CPUDebugger::executeCycle() {
	// Execute a CPU cycle, but record some info first.

	bool oldRecordActions = this->databus->getRecordActions();
	this->databus->setRecordActions(false);
	
	// Data that needs to be recorded before execution.
	uint8_t opcode = this->databus->read(this->registers.PC);
	if (!INSTRUCTION_SET.count(opcode)) {
		this->databus->setRecordActions(oldRecordActions);
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
	this->databus->setRecordActions(oldRecordActions);
	bool success = _6502_CPU::executeCycle();

	// Get the last info needed to describe an executed instruction: how many databus actions it took.
	unsigned int numOfDatabusActions = this->databus->getNumActions() - lastMemOpsSize;
	this->executedInstructions.push_back(ExecutedInstruction(opcode, instruction, numOfDatabusActions, oldRegisters, operands));

	return success;
}

void CPUDebugger::attach(DebugDatabus* databus) {
	this->databus = databus;
	_6502_CPU::attach(databus);
}

bool CPUDebugger::undoInstruction() {
	if (this->executedInstructions.size() == 0) {
		std::cout << "Warning: there have been no executed instructions; no instruction has been undone." << std::endl;
		return false;
	}

	ExecutedInstruction lastInstruction = this->executedInstructions.back();
	this->executedInstructions.pop_back();
	for (int i = 0; i < lastInstruction.numOfActionsInvolved; ++i) {
		this->databus->undoMemAction();
	}
	this->registers = lastInstruction.oldRegisters;
	return true;
}

ExecutedInstruction CPUDebugger::getLastExecutedInstruction() {
	if (!this->executedInstructions.size()) {
		std::cout << "Warning: the stack of executed instructions is empty. No changes have been made." << std::endl;
		return ExecutedInstruction();
	}

	return this->executedInstructions.back();
}

std::vector<ExecutedInstruction> CPUDebugger::getExecutedInstructions() {
	return this->getExecutedInstructions(this->executedInstructions.size());
}

// Returns vector of executed instructions, from least recent to most/
std::vector<ExecutedInstruction> CPUDebugger::getExecutedInstructions(unsigned int lastN) {
	std::vector<ExecutedInstruction> executedInstructions;

	if (this->executedInstructions.size()) {
		for (unsigned int i = 0; i < lastN && i < this->executedInstructions.size(); ++i) {
			executedInstructions.push_back(this->executedInstructions.at(this->executedInstructions.size() - lastN + i));
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

// 
void CPUDebuggerTest() {
	Memory memory;
	DebugDatabus databus{ &memory };
	DebugDatabus* databusPtr = &databus;
	CPUDebugger cpu{ databusPtr };
	cpu.setStdMemValue(0xcd);
	cpu.memPoke(RESET_VECTOR_ADDRESS, 0x00);
	cpu.memPoke(RESET_VECTOR_ADDRESS + 1, 0x02);
	cpu.powerOn();
	// Loading in a basic test program starting from 0x0200:
	uint8_t program[16] = {
		0xa9, 0x01,
		0x8d, 0x00, 0x07,
		0xa9, 0x05,
		0x8d, 0x01, 0x07,
		0xa9, 0x08,
		0x8d, 0x02, 0x07 };
	/*	In assembly:
	LDA #$01
	STA $0700
	LDA #$05
	STA $0701
	LDA #$08
	STA $0702  */
	for (int i = 0; i < 16; ++i) {
		cpu.memPoke(0x0200 + i, program[i]);
	}
	// Dumping the initial values:
	std::vector<uint8_t> testValues = cpu.memDump(0x0700, 0x0705);
	std::cout << "Memory before execution at 0x0700: ";
	for (int i = 0; i < testValues.size(); ++i) {
		std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) << (int)testValues.at(i);
		if (i != testValues.size() - 1) {
			std::cout << ", ";
		}
	}
	std::cout << std::endl;
	// Execute program
	for (int i = 0; i < 6; ++i) {
		cpu.executeCycle();
	}
	testValues = cpu.memDump(0x0700, 0x0705);
	std::cout << "Memory after execution at 0x0700: ";
	for (int i = 0; i < testValues.size(); ++i) {
		std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) << (int)testValues.at(i);
		if (i != testValues.size() - 1) {
			std::cout << ", ";
		}
	}
	std::cout << std::endl;
	// Undo execution (testing out user input as well)
	CommandlineInput input;
	ExecutedInstruction execInstr;
	Registers r;
	execInstr = cpu.getLastExecutedInstruction();
	std::cout << std::setfill('-') << std::setw(20) << '-' << std::endl;
	execInstr.print();
	char inputChar = '0';
	while (inputChar != 'q') {
		std::cout << std::setfill('-') << std::setw(20) << '-' << std::endl;
		inputChar = input.getUserChar("What to perform (q: quit, u: undo the last instruction, d: dump registers and 0x0700 to 0x0705): ");
		std::cout << std::endl;
		switch (inputChar) {
		case('u'):
			cpu.undoInstruction();
			execInstr = cpu.getLastExecutedInstruction();
			execInstr.print();
			std::cout << std::endl;
			break;
		case('d'):
			testValues = cpu.memDump(0x0700, 0x0705);
			r = cpu.registersPeek();
			std::cout << "Register values: A = 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)r.A <<
				", S = 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)r.S <<
				", SP = 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)r.SP <<
				", X = 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)r.X <<
				", Y = 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)r.Y <<
				", PC = 0x" << std::hex << std::setfill('0') << std::setw(4) << (int)r.PC << std::endl;
			std::cout << "Memory (0x0700 to 0x0705 inclusive): ";
			for (int i = 0; i < testValues.size(); ++i) {
				std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) << (int)testValues.at(i);
				if (i != testValues.size() - 1) {
					std::cout << ", ";
				}
			}
			std::cout << std::endl;
			break;
		default:
			break;
		}
	}
}
