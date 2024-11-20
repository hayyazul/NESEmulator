#include "nesDatabus.h"

NESDatabus::NESDatabus() : DataBus(), ram(nullptr) {}
NESDatabus::NESDatabus(Memory* memory) : DataBus(memory), ram(nullptr) {}
NESDatabus::NESDatabus(RAM* ram) : DataBus(), ram(ram) {}
NESDatabus::NESDatabus(Memory* memory, RAM* ram) : DataBus(memory), ram(ram) {}
NESDatabus::~NESDatabus() {}

void NESDatabus::attach(RAM* ram) {
	this->ram = ram;
}

void NESDatabus::attach(Memory* memory) {
	DataBus::attach(memory);
}

uint8_t NESDatabus::read(uint16_t address) {
	// First, check if the address is within the RAM address space.
	if (address < 0x2000) {
		return this->ram->getByte(address);
	} else {  // TODO: Include ability to get values from the various other registers.
		return DataBus::read(address);
	}
	
}

uint8_t NESDatabus::write(uint16_t address, uint8_t value) {
	if (address < 0x2000) {
		return this->ram->setByte(address, value);
	}
	else {  // TODO: Include ability to get values from the various other registers.
		return DataBus::write(address, value);
	};
}
