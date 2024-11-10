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

	// Basic, fundamental read/write operations.
	uint8_t read(uint16_t address);  // Returns the memory located at that address.
	uint8_t write(uint16_t address, uint8_t value);  // Returns the value just written (NOTE: might change this to the previous data value).

	// Operations specifically designed for the stack.
	// NOTE: I might move this into the CPU instead, or into some sort of macro class.
	/*
	The stack is in a region of memory located from 0x100 to 0x1ff inclusive.
	The stack starts at a higher value and decreases as more things are put onto it (push).
	It decreases as things are taken off it (pull).
	*/
	/* Probably will remove because they are not actual operations of the databus. I might make them a macro or a helper function which takes a reference to this class instead.
	uint8_t pullStack(uint8_t& stackPtr);
	uint8_t pullStack(Registers& registers);
	void pushStack(uint8_t value, uint8_t& stackPtr);
	void pushStack(uint8_t value, Registers& registers);
	*/
private:
	Memory* memory;

};