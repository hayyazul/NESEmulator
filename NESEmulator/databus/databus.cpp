#include "databus.h"

DataBus::DataBus()
{
}

DataBus::~DataBus()
{
}

uint8_t DataBus::read(uint16_t address) {
	return 0b0101;
}

uint8_t DataBus::write(uint16_t address, uint8_t value) {
	return 0;
}
