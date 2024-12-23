// databus.h : A middleman between the CPU and memory. 
// Provides an interface to read and write memory.
#pragma once
#include <cstdint>

// Inclusive
constexpr unsigned int STACK_START_ADDR = 0x1FF;
constexpr unsigned int STACK_END_ADDR = 0x100;

class Memory;
class Registers;

class DataBus {
public:
	DataBus();
	DataBus(Memory* memory);
	~DataBus();

	// Sets the internal pointer to a Memory module to the given pointer.
	void attach(Memory* memory);

	// Basic, fundamental read/write operations.
	virtual uint8_t read(uint16_t address);  // Returns the memory located at that address.
	virtual uint8_t write(uint16_t address, uint8_t value);  // Returns the value just written (NOTE: might change this to the previous data value).

private:
	Memory* memory;

};