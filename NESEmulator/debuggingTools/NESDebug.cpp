#include "NESDebug.h"

NESDebug::NESDebug() : NES() {
}

NESDebug::~NESDebug() {
}

void NESDebug::setStdValue(uint8_t val) {
	for (unsigned int i = 0; i < BYTES_OF_MEMORY; ++i) {
		this->memory.setByte(i, val);
	}
}

uint8_t NESDebug::memPeek(uint16_t memoryAddress) {
	return this->databus.read(memoryAddress);
}

Registers NESDebug::registersPeek() {
	return this->CPU.registersPeek();
}

bool NESDebug::registersPeek(char c) {
	return this->CPU.registersPeek().getStatus(c);
}

void NESDebug::registersPoke(Registers registers) {
	this->CPU.registersPoke(registers);
}

void NESDebug::memPoke(uint16_t memoryAddress, uint8_t val) {
	this->CPU.memPoke(memoryAddress, val);
}

bool NESDebug::memFind(uint8_t value, uint16_t& address, int lowerBound, int upperBound) {
	return this->CPU.memFind(value, address, lowerBound, upperBound);
}
