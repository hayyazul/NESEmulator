// Contains a simple function to load a .pal file into a map containing uint8_t-uint32_t pairs, the former being an index into the RGB value of the latter.
#pragma once

#include <map>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>

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

std::map<uint16_t, uint32_t> loadPalette(std::string filename) {
	const int FILENAME_SIZE = filename.size();
	const bool FILENAME_TOO_SMALL = filename.size() < 5;  // If there are 4 characters or less in the filename, it can not have a .pal extension.

	std::string lastChars = filename.substr(FILENAME_SIZE - 4, 4);
	std::transform(lastChars.begin(), lastChars.end(), lastChars.begin(), ::toupper);  // Account for file extensions w/ any capitalization.

	const bool WRONG_FILE_EXTENSION = lastChars != ".PAL";

	if (FILENAME_TOO_SMALL || WRONG_FILE_EXTENSION) {
		std::cout << "Failed to load palette file; not a .pal file: " << filename << std::endl;
		return std::map<uint16_t, uint32_t>{};
	}

	std::ifstream file{ filename, std::ios::binary };

	if (!file) {
		std::cout << "Failed to load palette file." << std::endl;
		return std::map<uint16_t, uint32_t>{};
	}

	std::map<uint16_t, uint32_t> paletteMap;
	uint8_t r, g, b;
	uint32_t color = 0x000000ff;  // RGBA color; the alpha channel is always 0xff, we are only taking the RGB values from the file.
	uint16_t idx = 0;  // Index of the current color (as used by the NES).

	while (file >> std::noskipws >> r >> g >> b) {
		color = (r << (8 * 3)) + (g << (8 * 2)) + (b << 8) + 0x000000ff;
		paletteMap[idx] = color;
		++idx;
	}

	return paletteMap;
};