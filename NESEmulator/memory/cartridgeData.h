// cartridgeData.h - The data, ROM and RAM, stored on a cartridge. Designed as the base mapper.
#pragma once

#include "memory.h"

constexpr int SIZE_OF_CARTRIDGE = 0xBFE0;

// TODO: Implement full mapper 0.
class Mapper_0 : public Memory {
public:
	Mapper_0();
	~Mapper_0();
};