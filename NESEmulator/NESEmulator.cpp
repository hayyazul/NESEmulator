#include "NESEmulator.h"

NES::NES() {
	this->databus = DataBus(&this->memory);
	this->CPU = _6502_CPU(&this->databus);
}

NES::~NES() {
	this->memory.~Memory();  // This deletes the 65kb allocated for it.
}

void NES::run() {
	return;  // TODO
}
