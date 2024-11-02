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

	void run();  // Placeholder until everything is implemented properly.

private:
	_6502_CPU CPU;
	Memory memory;
	DataBus databus;
};