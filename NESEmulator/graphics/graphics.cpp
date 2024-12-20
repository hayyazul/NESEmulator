#include "graphics.h"

#include <iostream>
#include <SDL.h>

Graphics::Graphics() : pxIdx(0), w(PICTURE_REGION_WIDTH), h(PICTURE_REGION_HEIGHT), displayBounds(DISPLAY_BOUNDS) {
	this->display = SDL_CreateRGBSurfaceWithFormat(NULL, w, h, 8, SDL_PIXELFORMAT_RGBA8888);  // NOTE: For some reason, I am forced to use 32 bits to define color. Need to figure out how to make it use 8 bits (if it becomes a problem).
	this->pixels = (uint32_t*)this->display->pixels;
}

Graphics::Graphics(unsigned int w, unsigned int h) : pxIdx(0), w(w), h(h) {
	this->displayBounds = SDL_Rect(0, 0, w, h);
	this->display = SDL_CreateRGBSurfaceWithFormat(NULL, w, h, 8, SDL_PIXELFORMAT_RGBA8888);
	this->pixels = (uint32_t*)this->display->pixels;
}

Graphics::Graphics(unsigned int w, unsigned int h, SDL_Rect displayBounds) : pxIdx(0), w(w), h(h), displayBounds(displayBounds){
	// Check if the given bounds are valid; if not, default to
	if (!this->isBoundsValid(displayBounds)) {
		std::cout << "Warning, bounds given for a Graphics object are invalid; defaulting to full bounds (w: " << this->w << ", h: " << this->h << ")";
		this->displayBounds = SDL_Rect(0, 0, w, h);
	}
	this->display = SDL_CreateRGBSurfaceWithFormat(NULL, w, h, 8, SDL_PIXELFORMAT_RGBA8888);
	this->pixels = (uint32_t*)this->display->pixels;
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
	SDL_BlitScaled(this->display, &this->displayBounds, windowSurface, nullptr);
}

void Graphics::clear(uint8_t r, uint8_t g, uint8_t b) {
	for (unsigned int i = 0; i < this->w; ++i) {
		for (unsigned int j = 0; j < this->h; ++j) {
			this->drawPixel(r, g, b, i, j);
		}
	}
}

void Graphics::drawRect(uint8_t r, uint8_t g, uint8_t b, unsigned int x, unsigned int y, unsigned int scaleX, unsigned int scaleY) {
	// This draws a rectangle w/ its top-left corner at the given coordinates w/ side length (in pixels) equal to the scale.
	
	for (unsigned int i = 0; i < scaleX; ++i) {
		for (unsigned int j = 0; j < scaleY; ++j) {
			this->drawPixel(r, g, b, x + i, y + j);
		}
	}
}

void Graphics::drawSquare(uint8_t r, uint8_t g, uint8_t b, unsigned int x, unsigned int y, unsigned int scale) {
	this->drawRect(r, g, b, x, y, scale, scale);
}

void Graphics::drawPixel(uint8_t r, uint8_t g, uint8_t b, unsigned int x, unsigned int y) {
	uint32_t color = this->getRGBValue(r, g, b);
	int idx = this->convertXYToIdx(x, y);
	this->pixels[idx] = color;
}

void Graphics::drawPixel(uint8_t r, uint8_t g, uint8_t b) {
	uint32_t color = this->getRGBValue(r, g, b);
	this->pixels[this->pxIdx] = color;

	this->updateBeamLocation();
}

int Graphics::getPxIdx() const {
	return this->pxIdx;
}

void Graphics::setPxIdx(int newPxIdx) {
	this->pxIdx = newPxIdx % (this->w * this->h);;
}

// Private methods

int Graphics::convertXYToIdx(unsigned int x, unsigned int y) const {
	return (x + y * this->w) % (this->w * this->h);
}

uint32_t Graphics::getRGBValue(uint8_t r, uint8_t g, uint8_t b) {
	return SDL_MapRGB(this->display->format, r, g, b);;
}

bool Graphics::isBoundsValid(SDL_Rect displayBounds) const {
	// Assumes w and h have been set (which they should have been in the constructor).

	bool xWithinBounds = this->w >= displayBounds.x + displayBounds.w && displayBounds.x >= 0;
	bool yWithinBounds = this->h >= displayBounds.y + displayBounds.h && displayBounds.y >= 0;

	return xWithinBounds && yWithinBounds;
}

void Graphics::updateBeamLocation() {
	this->setPxIdx(this->pxIdx + 1);
}
