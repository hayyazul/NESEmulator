// A databus designed specifically for the NES; you must connect this to RAM; PPU, APU, I/O registers; and a mapper.
#pragma once

#include "databus.h"
#include "../memory/ram.h"
#include "../ppu/ppu.h"

class InputPort;

constexpr int RAM_ADDRESSES = 0x2000;  // Contains the size of RAM in bytes; so in this case RAM takes up addresses 0x0000 to 0x2000.

namespace AddressingSpace {
	enum AddressingSpace {
		MEMORY,  // Standard memory as mapped by the cartridge.
		RAM,  // The 2kb of RAM on the NES (0x000 to 0x800 inclusive; mirrored up to and including 0x1fff)
		PPU_REGISTERS,  // The 8 addresses (0x2000 to 0x2007 inclusive; mirrored up to and including 0x3fff) involved w/ the PPU.
		INPUT_REGISTERS  // The 2 addresses, 0x4016 and 0x4017, which deal w/ controller input.
	};
}

// Returns the addressing space (e.g. is it memory mapped by the cartridge? CPU RAM? A PPU register? etc.)
AddressingSpace::AddressingSpace getAddressingSpace(uint16_t address);

class NESDatabus : public DataBus {
public:
	NESDatabus(Memory* memory = nullptr, RAM* ram = nullptr, PPU* ppu = nullptr);
	~NESDatabus();

	void attach(RAM* ram);
	void attach(Memory* memory);
	void attach(PPU* ppu);
	void attach(InputPort* input_port);

	virtual uint8_t read(uint16_t address) override;  // Returns the memory located at that address.
	virtual uint8_t write(uint16_t address, uint8_t value) override;  // Returns the value just written (NOTE: might change this to the previous data value).

private:
	RAM* ram;
	PPU* ppu;  
	InputPort* input_port;
};