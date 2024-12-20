#include "nametableWatch.h"

#include <array>

NametableDisplayer::NametableDisplayer()
{
}

NametableDisplayer::~NametableDisplayer()
{
}

void NametableDisplayer::displayNametable(Graphics& graphics, PPUDebug& ppu, unsigned int table, unsigned int x, unsigned int y, unsigned int scale) {
    std::array<uint8_t, TABLE_SIZE_IN_BYTES> nametable = ppu.getNametable(table);  // First get nametable data

    // Then draw the name table, looping over the data.
    for (uint16_t i = x; i < x + TABLE_WIDTH; ++i) {
        for (uint16_t j = y; j < y + TABLE_HEIGHT; ++j) {
            int nametableIdx = i - x + (j - y) * TABLE_WIDTH;
            this->displayTile(graphics, ppu, nametable.at(nametableIdx), i, j, scale);
        }
    }

}

void NametableDisplayer::displayTile(Graphics& graphics, PPUDebug& ppu, uint8_t tileId, unsigned int x, unsigned int y, unsigned int scale) {
    /* Display rules :
    00 - Transparent (black)
    01 - Red
    10 - Blue
    11 - White
    */
    const uint32_t BLACK = 0xff000000, RED = 0xffff0000, BLUE = 0xff0000ff, WHITE = 0xffffffff;


}
