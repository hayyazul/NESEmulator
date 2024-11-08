#include "NESEmulator.h"

NES::NES() {
	this->databus = DataBus(&this->memory);
	this->CPU = _6502_CPU(&this->databus);
}

NES::~NES() {}

void NES::loadROM(const char* fileName) {
 	NESFileData NESFile;
	Result result = parseiNESFile(fileName, NESFile);

	if (result == SUCCESS) {
		this->loadData(NESFile);
		this->CPU.reset();
	} else {
		std::cout << "iNES file parsing failed: " << result << std::endl;
	}
}

void NES::run() {
	while (this->totalMachineCycles < this->CYCLE_LIMIT) {
		this->executeMachineCycle();
	}
}

bool NES::executeMachineCycle() {
	// TODO: change it from 1:1 machine cycle to cpu cycle to its true value.
	bool result = this->CPU.executeCycle();
	++this->totalMachineCycles;
	return result;
}

void NES::loadData(NESFileData file) {
	// For now, we will only load in the reset vector.
	this->databus.write(RESET_VECTOR_ADDRESS, file.RESETVector[0]);
	this->databus.write(RESET_VECTOR_ADDRESS + 1, file.RESETVector[1]);

	// Then we load in ROM data.
	uint16_t j = 0;
	for (uint16_t i = this->CART_ROM_START_ADDR; i < 0xfffa; ++i) {
		if (j < file.programDataSize) {
			this->databus.write(i, file.programData[j]);
		} else if (j - file.programDataSize < file.characterDataSize) {
			this->databus.write(i, file.programData[j - file.programDataSize]);
		}
		++j;
	}
}
