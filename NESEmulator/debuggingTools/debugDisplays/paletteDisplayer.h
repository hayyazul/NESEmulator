#pragma once

#include "../../graphics/graphics.h"
#include "../PPUDebug.h"

void displayPalette(Graphics& graphics, PPUDebug& ppu, unsigned int x, unsigned int y, unsigned int scale = 1);
void displayPalette(Graphics& graphics, PPUDebug& ppu, std::map<uint16_t, uint32_t> paletteMap, unsigned int x, unsigned int y, unsigned int scale = 1);
