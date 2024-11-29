#include "nesDatabus.h"

#include <iostream>

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
		this->ppu->readRegister(address);
		break;
	default:
		std::cout << "nesDatabus.cpp, NESDatabus::read; Failed to resolve AddressingSpace." << std::endl;
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
		this->ppu->writeToRegister(address, value);
		return 0;  // TODO: A lot of debugging code utilizes a return from a write; I will need to introduce the PPU to these debuggings tools.
		break;
	default:
		std::cout << "nesDatabus.cpp, NESDatabus::write; Failed to resolve AddressingSpace." << std::endl;
		break;
	}
	
}

AddressingSpace::AddressingSpace getAddressingSpace(uint16_t address) {
	if (address < 0x2000) {
		return AddressingSpace::RAM;
	} else if (address >= 0x2000 && address < 0x2008) {  // TODO: Implement the PPU registers located in the 0x4000s
		return AddressingSpace::PPU_REGISTERS;
	} else {
		return AddressingSpace::MEMORY;
	}
}
