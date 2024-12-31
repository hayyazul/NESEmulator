#include "ppuDatabus.h"

PPUDatabus::PPUDatabus() {
}

PPUDatabus::~PPUDatabus() {
}

void PPUDatabus::attachVRAM(Memory* vram) {
	this->VRAM = vram;
}

void PPUDatabus::attachCHRDATA(Memory* chrData) {
	this->CHRDATA = chrData;
}

void PPUDatabus::attachPalette(Memory* paletteRAM) {
	this->paletteControl = paletteRAM;
}

uint8_t PPUDatabus::read(uint16_t address) {
	Memory* memory = this->getMemoryModule(address);
	this->adjustAddress(address);
	return memory->getByte(address);
}

uint8_t PPUDatabus::write(uint16_t address, uint8_t value) {
	Memory* memory = this->getMemoryModule(address);
	this->adjustAddress(address);
	return memory->setByte(address, value);
}

void PPUDatabus::adjustAddress(uint16_t& address) {
	const uint16_t PALETTE_RAM_ADDR = 0x3f00, PALETTE_SIZE = 0x20;
	const uint16_t NAMETABLES_ADDR = 0x2000, NAMETABLES_SIZE = 0x1000;  // Note: This is the size of all nametables combined.
	const uint16_t PATTERN_TABLES_ADDR = 0x0000, PATTERN_TABLES_SIZE = 0x2000;  // Note: Size of both pattern tables combined.

	if (address >= PALETTE_RAM_ADDR) {
		address %= PALETTE_SIZE;
	} else if (address >= NAMETABLES_ADDR) {
		address %= NAMETABLES_SIZE;
	} else {
		address %= PATTERN_TABLES_SIZE;
	}
}

Memory* PPUDatabus::getMemoryModule(uint16_t address) {
	const uint16_t PALETTE_RAM_ADDR = 0x3f00, PALETTE_SIZE = 0x20;
	const uint16_t NAMETABLES_ADDR = 0x2000, NAMETABLES_ADDR_SIZE = 0x1000;  // Note: This is the size of all nametables combined.
	const uint16_t PATTERN_TABLES_ADDR = 0x0000, PATTERN_TABLES_SIZE = 0x2000;  // Note: Size of both pattern tables combined.

	if (address >= PALETTE_RAM_ADDR) {
		return this->paletteControl;
	}
	else if (address >= NAMETABLES_ADDR) {
		return this->VRAM;
	}
	else {
		return this->CHRDATA;
	}

}
