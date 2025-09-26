#include "tableDisplayer.h"

#include <array>
#include <map>
#include "../../globals/helpers.hpp"

NametableDisplayer::NametableDisplayer() {}

NametableDisplayer::~NametableDisplayer() {}

void NametableDisplayer::displayNametable(Graphics& graphics, PPUDebug& ppu, unsigned int table, unsigned int x, unsigned int y, unsigned int scale, bool patternTable) {
    std::array<uint8_t, TABLE_SIZE_IN_BYTES> nametable = ppu.getNametable(table);  // First get nametable data

    const int patternWidth = 8, patternHeight = 8;  // In px

    // Then draw the name table, looping over the data.
    for (uint16_t i = 0; i < TABLE_HEIGHT; ++i) {
        for (uint16_t j = 0; j < TABLE_WIDTH; ++j) {
            int nametableIdx = j + i * TABLE_WIDTH;
            PTDisplayer.displayPattern(graphics, ppu, nametable.at(nametableIdx), patternTable, x + (j * patternWidth * scale), y + (i * patternHeight * scale), scale);
        }
    }

}


PatternTableDisplayer::PatternTableDisplayer() {}

PatternTableDisplayer::~PatternTableDisplayer() {}

void PatternTableDisplayer::displayPatternTable(Graphics& graphics, PPUDebug& ppu, bool table, unsigned int x, unsigned int y, unsigned int scale) {
    const int numOfPatterns = 0x100;
    const int rowSize = 16, colSize = numOfPatterns / rowSize;  // Note: If rowSize does not perfectly divide numOfPatterns, this function will fail to display the whole pattern table.
    const int patternW = 8, patternH = 8;  // These constants can not be modified; a pattern is invariably 8x8 pixels. They are defined for semantics.

    uint8_t patternId;  // Pattern ID to display; calculated in the for-loop.
    int patternX, patternY;  // Location of the pattern.
    // Loop through each pattern and display it. While the pattern table array is one dimensions, we want to display it as a 2d table, so we use a nested for-loop.
    for (int i = 0; i < rowSize; ++i) {
        for (int j = 0; j < colSize; ++j) {
            // Calculate the pattern ID first.
            patternId = j * rowSize + i;
            // Then calculate the location to display the pattern.
            patternX = x + (i * patternW * scale);
            patternY = y + (j * patternH * scale);
            // Then, using this patternId, dsplay the pattern.
            this->displayPattern(graphics, ppu, patternId, table, patternX, patternY, scale);
        }
    }
}

void PatternTableDisplayer::displayPattern(Graphics& graphics, std::array<uint8_t, PATTERN_SIZE_IN_BYTES> pattern, unsigned int x, unsigned int y, unsigned int scale) {
    // There are 8 rows of 8 pixels in a pattern. Each pixel is defined by 2 bits.
    // This is arranged by 2 sets of 8 bytes, the first 8 defining the first bit, the second 8 defining the second.
    const unsigned int numRows = 8, numBits = 8;
    
    /* Display rules :
    00 - Transparent (gray)
    01 - Red
    10 - Green
    11 - Blue
    */
    const uint32_t GRAY = 0x1f1f1fff, GREEN = 0x00ff00ff, RED = 0xff0000ff, BLUE = 0x0000ffff;
    const std::map<uint8_t, uint32_t> pxIdToColor{ {0b00, GRAY}, {0b01, RED}, {0b10, GREEN}, {0b11, BLUE} };

    uint8_t lowerBitField, upperBitField, pxId;
    for (unsigned int i = 0; i < numRows; ++i) {
        lowerBitField = pattern.at(i);
        upperBitField = pattern.at(i + numRows);
        for (unsigned int j = 0; j < numBits; ++j) {
            pxId = (getBitVal(upperBitField, j) << 1) + getBitVal(lowerBitField, j);
            
            // Note: when j is 0, it is indexing bit 0 of the bitfield's row, which is located on the RIGHT.
            // So, we want to display that pixel on the right, so we invert the range when adding it to x,
            // i.e. as j goes from 0 to 7, the number we add goes from 7 * scale to 0 * scale.
            graphics.drawSquare(pxIdToColor.at(pxId), x + (numBits - j) * scale, y + i * scale, scale);
        }
    }
}

void PatternTableDisplayer::displayPattern(Graphics& graphics, PPUDebug& ppu, uint8_t patternId, bool table, unsigned int x, unsigned int y, unsigned int scale) {
    // First, we get the pattern table.
    std::array<uint8_t, PATTERN_SIZE_IN_BYTES> pattern = ppu.getPattern(patternId, table);
 
    // Finally, we draw the pattern w/ this pattern data. 
    this->displayPattern(graphics, pattern, x, y, scale);
}
