#include "nesDatabus.h"
#include "../input/inputPort.h"

#include <iostream>

// TODO: Support player 2.

//NESDatabus::NESDatabus() : DataBus(), ram(nullptr), ppu(nullptr) {}
NESDatabus::NESDatabus(Memory* memory, RAM* ram, PPU* ppu) : DataBus(memory), ram(ram), ppu(ppu) {}
NESDatabus::~NESDatabus() {}

void NESDatabus::attach(RAM* ram) {
	this->ram = ram;
}

void NESDatabus::attach(Memory* memory) {
	DataBus::attach(memory);
}

void NESDatabus::attach(PPU* ppu) {
	this->ppu = ppu;
}

void NESDatabus::attach(InputPort* input_port) {
	this->input_port = input_port;
}

uint8_t NESDatabus::read(uint16_t address) {
	// First, check if the address is within the RAM address space.
	AddressingSpace::AddressingSpace addrSpace = getAddressingSpace(address);
	switch (addrSpace) {
	case(AddressingSpace::RAM):
		return this->ram->getByte(address);
		break;
	case(AddressingSpace::MEMORY):
		return DataBus::read(address);
		break;
	case(AddressingSpace::PPU_REGISTERS):
		return this->ppu->readRegister(address);
		break;
	case(AddressingSpace::INPUT_REGISTERS):
		return 0x40 | this->input_port->readAndClock();  // The upper 3 bits returned is open bus, which is USUALLY 0b010, so the output is usually 0b0100'000N.
		break;
	default:
		// Note: This should never happen.
		std::cout << "nesDatabus.cpp, NESDatabus::read; Failed to resolve AddressingSpace." << std::endl;
		return 0;
		break;
	};
}

uint8_t NESDatabus::write(uint16_t address, uint8_t value) {
	AddressingSpace::AddressingSpace addrSpace = getAddressingSpace(address);
	switch (addrSpace) {
	case(AddressingSpace::RAM):
		return this->ram->setByte(address, value);
		break;
	case(AddressingSpace::MEMORY):
		return DataBus::write(address, value);
		break;
	case(AddressingSpace::PPU_REGISTERS):
		return this->ppu->writeToRegister(address, value);
		break;
	case(AddressingSpace::INPUT_REGISTERS):
		this->input_port->setLatch(value & 0b1);  // Sets the latch associated w/ the input port to value of the first bit.
		return 0;
		break;
	default:
		std::cout << "nesDatabus.cpp, NESDatabus::write; Failed to resolve AddressingSpace." << std::endl;
		return 0;
		break;
	}
	
}

AddressingSpace::AddressingSpace getAddressingSpace(uint16_t address) {
	if (address < 0x2000) {
		return AddressingSpace::RAM;
	} else if ((address >= 0x2000 && address < 0x2008) || address == 0x4014) {  // TODO: Implement the PPU registers located in the 0x4000s
		return AddressingSpace::PPU_REGISTERS;
	} else if (address == 0x4016 || address == 0x4017) {
		return AddressingSpace::INPUT_REGISTERS;
	} else {
		return AddressingSpace::MEMORY;
	}
}
