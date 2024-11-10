#include "databus.h"
#include "../memory/memory.h"
#include "../6502Chip/CPU.h"

DataBus::DataBus() {
}

DataBus::DataBus(Memory* memory) : memory(memory) {
}

DataBus::~DataBus() {
}

uint8_t DataBus::read(uint16_t address) {
	return this->memory->getByte(address);	
}

uint8_t DataBus::write(uint16_t address, uint8_t value) {
	return this->memory->setByte(address, value);
}
/*
uint8_t DataBus::pullStack(uint8_t& stackPtr) {
	// The stack pointer currently points to the address above
	// the last occupied one; so decrement, then access the 
	// stack addresses.
	--stackPtr;
	return this->read(STACK_END_ADDR + stackPtr);
}

uint8_t DataBus::pullStack(Registers& registers) {
	// The stack pointer currently points to the address above
	// the last occupied one; so decrement, then access the 
	// stack addresses.
	return this->pullStack(registers.SP);
}

void DataBus::pushStack(uint8_t value, uint8_t& stackPtr) {
	// The stackPtr is pointing to an unoccupied memory addr,
	// so we will write there first--- then we will increment it.
	this->write(STACK_END_ADDR + stackPtr, value);
	++stackPtr;
}

void DataBus::pushStack(uint8_t value, Registers& registers) {
	this->pushStack(value, registers.SP);
}
*/