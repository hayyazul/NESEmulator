#pragma once

#include <SDL.h>

constexpr unsigned int DISPLAY_WIDTH = 256, DISPLAY_HEIGHT = 240, PICTURE_REGION_WIDTH = 283, PICTURE_REGION_HEIGHT = 242;
constexpr SDL_Rect DISPLAY_BOUNDS{16, 0, 256, 240};

/* A class to display a mutable surface; the PPU will write to this surface(or use some intermediary) and this class will display it to the user.
 This does NOT emulate NTSC video; the PPU will convert its output to YUV, then from YUV to RGB (I will either implement these conversions in this class, or through some middleman class).
 Note: visual refers to the region displayed on screen (the 256x240 region) and not the full picture region w/ boundaries.
*/
 class Graphics {
public:
	Graphics();
	~Graphics();

	// To be called before drawing.
	void lockDisplay();

	// To be called after drawing and before copying to the window surface.
	void unlockDisplay();

	// Copies the visual display to the window and updates it. NOTE: Might rename the function to make the updating-the-screen part more clear.
	void blitDisplay(SDL_Surface* windowSurface);

	// Draws a pixel with the given color values and at the current location (of the "scanning beam"), then advances the location.
	void drawPixel(uint8_t r, uint8_t g, uint8_t b);  // NOTE: The NES does not output RGB, so this function may change.

private:

	void updateBeamLocation();

	// "The PPU outputs a picture region of 256x240 pixels and a border region extending 16 pixels left, 11 pixels right, and 2 pixels down (283x242)."
	SDL_Surface* display;  // The 283x242 display.
	
	//int scanX, scanY;  
	int pxIdx; // Location of the "scanning beam" 

};