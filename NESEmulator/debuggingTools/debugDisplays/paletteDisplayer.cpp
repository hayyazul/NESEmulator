#include "paletteDisplayer.h"

#include "../../loadingData/loadPalette.h"

void displayPalette(Graphics& graphics, PPUDebug& ppu, unsigned int x, unsigned int y, unsigned int scale) {
	const std::map<uint16_t, uint32_t> paletteMap = loadPalette("resourceFiles/2C02G_wiki.pal");
	
	auto palette = ppu.getPalette();
	uint32_t color;
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 0x10; ++j) {
			auto a = (i << 4);
			auto b = a + j;
			auto c = palette.at(b);
			auto d = paletteMap.at(c);
			color = paletteMap.at(palette.at((i << 4) + j));
			graphics.drawSquare(color, x + j * scale, y + i * scale, scale);
		}
	}

}
