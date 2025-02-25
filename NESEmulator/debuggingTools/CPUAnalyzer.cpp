#include "CPUAnalyzer.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <minmax.h>
#include <sstream>
#include "../memory/memory.h"
#include "../input/cmdInput.h"

CPUDebugger::CPUDebugger() : _6502_CPU(this->databus) {
}

CPUDebugger::CPUDebugger(DebugDatabus* databus) : databus(databus), _6502_CPU(databus) {}

CPUDebugger::~CPUDebugger() {}

// Make it return an error code instead of a bool.
CPUCycleOutcomes CPUDebugger::executeCycle(bool DMACycle) {
	
	// Data that needs to be recorded before execution.
	uint8_t opcode = this->databus->read(this->registers.PC);
	if (!INSTRUCTION_SET.count(opcode)) {
		int _ = 0;
		return FAIL;
	}

	Instruction* instruction = &INSTRUCTION_SET.at(opcode);
	Registers oldRegisters = this->registers;

	// If there are any operands, get those operands.
	uint8_t operands[2] = { 0, 0 };
	for (int i = 1; i < instruction->numBytes; ++i) {
		operands[i - 1] = this->databus->read(this->registers.PC + i);
	}
	CPUCycleOutcomes outcome = _6502_CPU::executeCycle(DMACycle);

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

CPUInternals CPUDebugger::getInternals() const {
	CPUInternals internals;
	internals.registers = this->registers;
	internals.interruptRequested = this->interruptRequested;
	internals.performInterrupt = this->performInterrupt;
	internals.nmiRequested = this->nmiRequested;
	internals.lastNMISignal = this->lastNMISignal;
	internals.getOrPutCycle = this->getOrPutCycle;
	internals.totalCyclesElapsed = this->totalCyclesElapsed;
	internals.opcodeCyclesElapsed = this->opcodeCyclesElapsed;
	internals.currentOpcodeCycleLen = this->currentOpcodeCycleLen;
	internals.performNMI = this->performNMI;

	return internals;
}

void CPUDebugger::attach(DataBus* databus) {
	this->databus = databus;
	_6502_CPU::attach(databus);
}

void CPUDebugger::loadInternals(CPUInternals cpuInternals) {
	this->registers = cpuInternals.registers;
	this->interruptRequested = cpuInternals.interruptRequested;
	this->performInterrupt = cpuInternals.performInterrupt;
	this->nmiRequested = cpuInternals.nmiRequested;
	this->lastNMISignal = cpuInternals.lastNMISignal;
	this->getOrPutCycle = cpuInternals.getOrPutCycle;
	this->totalCyclesElapsed = cpuInternals.totalCyclesElapsed;
	this->opcodeCyclesElapsed = cpuInternals.opcodeCyclesElapsed;
	this->currentOpcodeCycleLen = cpuInternals.currentOpcodeCycleLen;
	this->performNMI = cpuInternals.performNMI;
}

uint64_t CPUDebugger::getNumCycles() const {
	return this->totalCyclesElapsed;
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

CPUInternals::CPUInternals() {};
CPUInternals::~CPUInternals() {}

void CPUInternals::deserializeData(std::stringstream& data) {
	
	// A map to a component enum; this is used so if I change the labels, I do not need to change the code which checks for the labels much.
	enum Component {
		REGISTERS,
		IRQREQ,
		IRQPERFORM,
		NMIREQ,
		LASTNMI,
		CYCLETYPE,
		TOTALCYCLES,
		OPCODECYCLES,
		CURRCYCLELEN
	};
	const std::map<std::string, Component> LABEL_TO_COMPONENT = { 
		{"REGISTERS:", REGISTERS},
		{"IRQREQ:", IRQREQ},
		{"IRQPERFORM:", IRQPERFORM},
		{"NMIREQ:", NMIREQ},
		{"LASTNMI:", LASTNMI},
		{"CYCLETYPE:", CYCLETYPE},
		{"TOTALCYCLES:", TOTALCYCLES},
		{"OPCODECYCLES:", OPCODECYCLES},
		{"CURRCYCLELEN:", CURRCYCLELEN}
	};

	// First, get all the data into a string vector representing the datapoints.
	std::vector<std::string> datapoints;
	for (std::string datapoint; std::getline(data, datapoint, ' ');) {
		datapoints.push_back(datapoint);
	}

	// Then, we will iterate through the datapoints vector.
	Component componentOn = REGISTERS;
	int i = 0;
	for (std::string& datapoint : datapoints) {
		// If we are on a label, then update the component that we are on.
		if (LABEL_TO_COMPONENT.contains(datapoint)) {
			componentOn = LABEL_TO_COMPONENT.at(datapoint);
			continue;
		}

		unsigned long value = std::stoll(datapoint);
		// Insert the data differently given the component.
		switch (componentOn) {
		case(REGISTERS): {
			// The order the registers are serialized: PC, A, X, Y, SP, S
			switch (i) {
			case(0):
				this->registers.PC = (uint16_t)value;
				break;
			case(1):
				this->registers.A = (uint8_t)value;
				break;
			case(2):
				this->registers.X = (uint8_t)value;
				break;
			case(3):
				this->registers.Y = (uint8_t)value;
				break;
			case(4):
				this->registers.SP = (uint8_t)value;
				break;
			case(5):
				this->registers.S = (uint8_t)value;
				break;
			default:
				int _ = 0;  // This should never execute.
				break;
			}
			break;
		}
		case(IRQREQ): {
			this->interruptRequested = (bool)value;
			break;
		}
		case(IRQPERFORM): {
			this->performInterrupt = (bool)value;
			break;
		}
		case(NMIREQ): {
			this->nmiRequested = (bool)value;
			break;
		}
		case(LASTNMI): {
			this->lastNMISignal = (bool)value;
			break;
		}
		case(CYCLETYPE): {
			this->getOrPutCycle = (bool)value;
			break;
		}
		case(TOTALCYCLES): {
			this->totalCyclesElapsed = value;
			break;
		}
		case(OPCODECYCLES): {
			this->opcodeCyclesElapsed = (unsigned int)value;
			break;
		}
		case(CURRCYCLELEN): {
			this->currentOpcodeCycleLen = (unsigned int)value;
			break;
		}
		}

		++i;
	}
}

std::string CPUInternals::getSerialFormat() const {

	std::stringstream preSerializedStr;

	preSerializedStr << "REGISTERS: " << (unsigned long long)registers.PC;
	preSerializedStr << " " << (unsigned long long)registers.A;
	preSerializedStr << " " << (unsigned long long)registers.X;
	preSerializedStr << " " << (unsigned long long)registers.Y;
	preSerializedStr << " " << (unsigned long long)registers.SP;
	preSerializedStr << " " << (unsigned long long)registers.S << '\n';

	preSerializedStr << "IRQREQ: " << (unsigned long long)interruptRequested << '\n';
	preSerializedStr << "IRQPERFORM: " << (unsigned long long)performInterrupt << '\n';

	preSerializedStr << "NMIREQ: " << (unsigned long long)nmiRequested << '\n';
	preSerializedStr << "LASTNMI: " << (unsigned long long)lastNMISignal << '\n';

	preSerializedStr << "CYCLETYPE: " << (unsigned long long)getOrPutCycle << '\n';

	preSerializedStr << "TOTALCYCLES: " << (unsigned long long)totalCyclesElapsed << '\n';
	preSerializedStr << "OPCODECYCLES: " << (unsigned long long)opcodeCyclesElapsed << '\n';
	preSerializedStr << "CURRCYCLELEN: " << (unsigned long long)currentOpcodeCycleLen << '\n';

	return preSerializedStr.str();
}