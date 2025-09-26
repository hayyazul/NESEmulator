#pragma once

#include <SDL.h>

// Constants specificially referring to the NES's display parameters.
constexpr unsigned int DISPLAY_WIDTH = 256, DISPLAY_HEIGHT = 240, PICTURE_REGION_WIDTH = 283, PICTURE_REGION_HEIGHT = 242;
constexpr SDL_Rect DISPLAY_BOUNDS{16, 0, 256, 240};

/* A class to display a mutable surface; the PPU will write to this surface(or use some intermediary) and this class will display it to the user.
 This does NOT emulate NTSC video.

 NOTE: This implementation is very poor at streaming data; this is fine for now as it will just do debug views, but I want to change this when integrating everything together.

*/

class Graphics {
public:
	Graphics();
	Graphics(unsigned int w, unsigned int h);  // Sets the display bounds to cover the entire surface.
	Graphics(unsigned int w, unsigned int h, SDL_Rect displayBounds);  // If the display bounds are invalid (e.g. a greater width than w), then it defaults to covering the entire surface.
	~Graphics();

	// To be called before drawing.
	void lockDisplay();

	// To be called after drawing and before copying to the window surface.
	void unlockDisplay();

	// Copies the visual display to the window and updates it. NOTE: Might rename the function to make the updating-the-screen part more clear.
	void blitDisplay(SDL_Surface* windowSurface);

	// Returns the display format of the display surface.
	const SDL_PixelFormat* getDisplayFormat() const;

	uint32_t getRGB(uint8_t r, uint8_t g, uint8_t b) const;

	// Clears the display surface by replacing it with a default color value (which can be specified).
	void clear(uint32_t rgb = 0xff000000);

	// Draws a rect with the given color at the specified location and scale; coordinates will not be scaled.
	void drawRect(uint32_t rgb, unsigned int x, unsigned int y, unsigned int scaleX, unsigned int scaleY);
	void drawSquare(uint32_t, unsigned int x, unsigned int y, unsigned int scale);

	// Draws a pixel with the given color at the specified location; does not update the current location. Performs no bounds checking; be weary where you draw.
	void drawPixel(uint32_t rgb, unsigned int x, unsigned int y);
	// Draws a pixel with the given color values and at the current location (of the "scanning beam"), then advances the location.
	void drawPixel(uint32_t rgb);  // NOTE: The NES does not output RGB, so this function may change.

	int getPxIdx() const;  // Gets the current value of pxIdx, which is the location of the "scanning beam".

	void setPxIdx(int newPxIdx);  // Sets the value of pxIdx, which is the location of the "scanning beam". If out of bounds, defaults to maximum allowed value.

	const int w, h, totalPx;  // Width, height, and total pixels of the surface; immutable.

private:

	bool isBoundsValid(SDL_Rect displayBounds) const;  // Checks if a given displayBounds is sound (the rect does not go out of bounds of the display surface dimensions).

	void updateBeamLocation();

	// "The PPU outputs a picture region of 256x240 pixels and a border region extending 16 pixels left, 11 pixels right, and 2 pixels down (283x242)."
	SDL_Surface* display;  // The 283x242 display.
	uint32_t* pixels;  // Pointer to the pixels located in the display; defined to prevent casting the void pointer in the display surface to a uint32_t pointer when drawing pixels.

	SDL_Rect displayBounds;  // The rectangle bounding the pixels which are actually displayed.

	int pxIdx; // Location of the "scanning beam" 

};

