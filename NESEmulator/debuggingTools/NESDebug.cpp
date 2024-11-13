#include "NESDebug.h"
#include "debugDatabus.h"
#include "CPUAnalyzer.h"

NESDebug::NESDebug() : NES() {
	delete this->databus;
	this->databusInstance.attach(&this->memory);
	this->databus = &this->databusInstance;  // Gets casted into a vanilla databus pointer (used for normal ops)
	this->cpuInstance.attach(&this->databusInstance);  // Attaches the debugger pointer instead.
	this->CPU = (_6502_CPU*)(&this->cpuInstance);
}

NESDebug::~NESDebug() {
	// The CPU and Databus instances will get destroyed because they are 
	// a part of this class.
}

bool NESDebug::setRecord(bool record) {
	bool oldRecordActions = this->databusInstance.getRecordActions();
	this->databusInstance.setRecordActions(record);
	return oldRecordActions;
}

bool NESDebug::getRecord(bool record) const {
	return this->databusInstance.getRecordActions();
}

void NESDebug::clearRecord() {
	this->cpuInstance.clearExecutedInstructions();
}

void NESDebug::setStdValue(uint8_t val) {
	for (unsigned int i = 0; i < BYTES_OF_MEMORY; ++i) {
		this->memory.setByte(i, val);
	}
}

uint8_t NESDebug::memPeek(uint16_t memoryAddress) {
	return this->databus->read(memoryAddress);
}
CPUDebugger* NESDebug::getCPUPtr() {
	CPUDebugger* cpuPtr = &this->cpuInstance;
	return cpuPtr;
}
/*
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
*/