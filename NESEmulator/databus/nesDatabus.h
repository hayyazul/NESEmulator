// A databus designed specifically for the NES; you must connect this to RAM; PPU, APU, I/O registers; and a mapper.
#pragma once

#include "databus.h"
#include "../memory/ram.h"

constexpr int RAM_ADDRESSES = 0x2000;  // Contains the size of RAM in bytes; so in this case RAM takes up addresses 0x0000 to 0x2000.

class NESDatabus : public DataBus {
public:
	NESDatabus();
	NESDatabus(Memory* memory);
	NESDatabus(RAM* ram);
	NESDatabus(Memory* memory, RAM* ram);
	~NESDatabus();

	void attach(RAM* ram);
	void attach(Memory* memory);

	virtual uint8_t read(uint16_t address) override;  // Returns the memory located at that address.
	virtual uint8_t write(uint16_t address, uint8_t value) override;  // Returns the value just written (NOTE: might change this to the previous data value).

private:
	RAM* ram;

};