// NESEmulator.h : Program entry.
#pragma once

#include <iostream>

#include "6502Chip/CPU.h"
#include "memory/memory.h"
#include "databus/databus.h"

class NES {
public:
	NES();
	~NES();

	void loadROM(const char* fileName);

	void run();  // Placeholder until everything is implemented properly.

public:
	// Temporarily public for debugging purposes.
	void executeMachineCycle();  // For now, 1 Machine Cycle = 1 CPU Cycle. This is not how it is in the actual implementation.

private:
	_6502_CPU CPU;
	Memory memory;
	DataBus databus;

	// Debug variables
	unsigned long int totalMachineCycles = 0;
	unsigned long int CYCLE_LIMIT = 100;  // Referring to the machine cycle.
};