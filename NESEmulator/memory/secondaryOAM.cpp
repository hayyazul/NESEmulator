#include "secondaryOAM.h"

SecondaryOAM::SecondaryOAM() : Memory(0x20), writeEnabled(true), freeByteIdx(0) {}

SecondaryOAM::~SecondaryOAM() {}

uint8_t SecondaryOAM::setByte(uint16_t address, uint8_t value) {
	if (this->writeEnabled) {
		return Memory::setByte(address, value);
	}

	return 0;
}

uint8_t SecondaryOAM::setFreeByte(uint8_t value) {
	uint8_t oldVal = this->setByte(this->freeByteIdx, value); 
	if (this->freeByteIdx == 0xff) {
		this->writeEnabled = false;
	} else {
		++this->freeByteIdx;
	}

	return oldVal;
}

void SecondaryOAM::freeAllBytes() {
	this->freeByteIdx = 0;
	this->writeEnabled = true;
}

bool SecondaryOAM::setWriteState(bool writeState) {
	bool oldState = this->writeEnabled;
	this->writeEnabled = writeState;
	
	return oldState;
}

bool SecondaryOAM::getWriteState() const {
	return this->writeEnabled;
}
