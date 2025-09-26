// Contains a simple function to load a .pal file into a map containing uint8_t-uint32_t pairs, the former being an index into the RGB value of the latter.
#pragma once

#include <map>
#include <string>

/* FORMAT OF THE MAP

On the NES, some values are associated w/ some colors, e.g. 0x02 w/ blue, 0x00 w/ gray, etc. The range of these values span 5 bits, so 0bNNNNNN.
Furthermore, the PPU has a register (PPUMASK) which can emphasize different or multiple colors. The given .pal file defines not just the colors 
for a given value, but the colors for a given value under arbitrary emphasis on bits. The kinds of colors emphasized is indicated by the upper 3 bits
of the value in the map. Note that the NES still only uses 5 bits, but in this emulator 8 bits are used to indicate the emphasis.

So, a given value which maps to a single uint32_t which represents a color has the following form: 0bBGRNNNNNN, where B, G, and R are bits indicating
their respective color's emphasis. Note also how this has 9 bits, so we must use a uint16_t.
*/


// Takes in a .pal file and loads a palette into a constant map.
std::map<uint16_t, uint32_t> loadPalette(std::string filename);