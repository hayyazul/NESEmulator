#include "graphics.h"

#include <iostream>
#include <SDL.h>

Graphics::Graphics() : pxIdx(0) {
	this->display = SDL_CreateRGBSurfaceWithFormat(NULL, PICTURE_REGION_WIDTH, PICTURE_REGION_HEIGHT, 8, SDL_PIXELFORMAT_RGBA8888);
}

Graphics::~Graphics() {
	SDL_FreeSurface(this->display);
}

void Graphics::lockDisplay() {
	SDL_LockSurface(this->display);
}

void Graphics::unlockDisplay() {
	SDL_UnlockSurface(this->display);
}

void Graphics::blitDisplay(SDL_Surface* windowSurface) {
	SDL_BlitScaled(this->display, &DISPLAY_BOUNDS, windowSurface, nullptr);
}

void Graphics::drawPixel(uint8_t r, uint8_t g, uint8_t b) {
	uint32_t* buffer = (uint32_t*)this->display->pixels;
	uint32_t color = SDL_MapRGB(this->display->format, r, g, b);
	buffer[this->pxIdx] = color;

	this->updateBeamLocation();
}

// Private methods

void Graphics::updateBeamLocation() {
	this->pxIdx = (this->pxIdx + 1) % (PICTURE_REGION_HEIGHT * PICTURE_REGION_WIDTH);
}
