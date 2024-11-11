#include "NESEmulator.h"

NES::NES() {  // Not recommended to initialize w/ this.
	this->databus = new DataBus(&this->memory);
	this->CPU = _6502_CPU(this->databus);
	this->CPU.powerOn();
}

NES::NES(DataBus* databus) {
	this->databus = databus;
	this->databus->attach(&this->memory);
	this->CPU.attach(this->databus);
}

NES::~NES() {}

void NES::powerOn() {
	this->CPU.powerOn();
}

void NES::reset() {
	this->CPU.reset();
}

void NES::loadROM(const char* fileName) {  // Remember to reset the NES after loading a ROM.
 	NESFileData NESFile;
	Result result = parseiNESFile(fileName, NESFile);

	if (result == SUCCESS) {
		this->loadData(NESFile);
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
	// Then we load in ROM data.

	// Note: There is a special case regarding PRG ROM; if its size is 0x4000 (16K), we will duplicate it.
	// This is because we start loading in from address 0x8000, and at 16K we will only reach 0xc000. By duplicatin
	bool duplicateData = false;
	if (file.programDataSize == 0x4000) {
		duplicateData = true;
		file.programDataSize *= 2;
		std::vector<uint8_t> tempProgramData;
		// Duplicating data.
		for (int i = 0; i < 2; ++i) {
			for (uint8_t byte : file.programData) {
				tempProgramData.push_back(byte);
			}
		}
		file.programData = tempProgramData;
	}

	uint16_t j = 0;
	for (uint32_t i = this->CART_ROM_START_ADDR; i <= 0xffff; ++i) {
		if (j < file.programDataSize) {
			this->databus->write(i, file.programData[j]);
		} //else if (j - file.programDataSize < file.characterDataSize) {  // NOTE: I am confused on where to put CHR ROM, so for now I don't put it in at all.
			//this->databus.write(i, file.programData[j - file.programDataSize]);
		//}
		++j;
	}
}
