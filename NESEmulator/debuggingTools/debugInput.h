#pragma once

#include "../input/input.h"

// Extends the Input base class with some extra methods to simulate key actions.
class DebugInput : public Input {
public:
	DebugInput();
	~DebugInput();

	// Manually sets a key state.
	void setKeyState(SDL_Scancode key, KeyState state);
private:
};