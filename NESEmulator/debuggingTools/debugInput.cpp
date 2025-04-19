#include "debugInput.h"

DebugInput::DebugInput() {}

DebugInput::~DebugInput() {}

void DebugInput::setKeyState(SDL_Scancode key, KeyState state) {
	this->keyStates[key] = state;
}
