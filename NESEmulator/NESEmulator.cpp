#include "NESEmulator.h"

NES::NES() {
	this->databus = DataBus(&this->memory);
	this->CPU = _6502_CPU(&this->databus);
}

NES::~NES() {
	this->memory.~Memory();  // This deletes the 65kb allocated for it.
}

void NES::loadROM(const char* fileName) {
}

void NES::run() {
	while (this->totalMachineCycles < this->CYCLE_LIMIT) {
		this->executeMachineCycle();
	}
}

void NES::executeMachineCycle() {
	// TODO: change it from 1:1 machine cycle to cpu cycle to its true value.
	this->CPU.executeCycle();
	++this->totalMachineCycles;
}
