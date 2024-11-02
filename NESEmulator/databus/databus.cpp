#include "databus.h"
#include "../memory/memory.h"

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
