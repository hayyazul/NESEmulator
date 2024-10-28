#pragma once
#include <cstdint>

class Memory;

class DataBus {
public:
	DataBus();
	~DataBus();

	uint8_t read(uint16_t address);
	uint8_t write(uint16_t address, uint8_t value);

private:
	Memory* memory;

};