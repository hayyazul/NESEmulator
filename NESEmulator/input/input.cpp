#include "input.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

Input::Input(): quit(false) {
}

Input::~Input() {
}

bool Input::updateInput() {
    // BUG NOTE: For some reason, SDL_PollEvent seems incapable of recording more than 2 simulatenous key presses. This is annoying, but can be worked with for the time being. 
    // Some fault may lie in the laptop, as show by testing w/ an online keyboard rollover test, but that same test also reveals this function is still flawed somehow.
    
    this->staticKeyUpdate();  // First, do the update which will always happen.
	// Then see what the user has input.
    bool inputRecieved = false;
	SDL_Event event;
    while (SDL_PollEvent(&event)) {
        inputRecieved = true;
        switch (event.type) {
        case(SDL_KEYDOWN):
            this->updateKeyState(event.key.keysym.scancode, true);

            break;
        case(SDL_KEYUP):
            this->updateKeyState(event.key.keysym.scancode, false);
            break;
        case(SDL_QUIT):
            this->quit = true;
            break;
        default:
            inputRecieved = false;
            break;
            // NOTE: May or may not add mouse support.
        }
    };

    return inputRecieved;
}

KeyState Input::getKeyState(SDL_Scancode key) const {
    if (!this->keyStates.contains(key)) return NA;  // If the keystate has not been recorded yet at all, return NA.
    return this->keyStates.at(key);
}

void Input::printKeyStates(KeyState excludedState) {
	// This is used to translate from numeric code to the actual letters of the button pressed.
    std::map<SDL_Scancode, std::string> scancodeToStr = {
        {SDL_SCANCODE_0, "0"},
        {SDL_SCANCODE_1, "1"},
        {SDL_SCANCODE_2, "2"},
        {SDL_SCANCODE_3, "3"},
        {SDL_SCANCODE_4, "4"},
        {SDL_SCANCODE_5, "5"},
        {SDL_SCANCODE_6, "6"},
        {SDL_SCANCODE_7, "7"},
        {SDL_SCANCODE_8, "8"},
        {SDL_SCANCODE_9, "9"},
        {SDL_SCANCODE_A, "a"},
        {SDL_SCANCODE_B, "b"},
        {SDL_SCANCODE_C, "c"},
        {SDL_SCANCODE_D, "d"},
        {SDL_SCANCODE_E, "e"},
        {SDL_SCANCODE_F, "f"},
        {SDL_SCANCODE_G, "g"},
        {SDL_SCANCODE_H, "h"},
        {SDL_SCANCODE_I, "i"},
        {SDL_SCANCODE_J, "j"},
        {SDL_SCANCODE_K, "k"},
        {SDL_SCANCODE_L, "l"},
        {SDL_SCANCODE_M, "m"},
        {SDL_SCANCODE_N, "n"},
        {SDL_SCANCODE_O, "o"},
        {SDL_SCANCODE_P, "p"},
        {SDL_SCANCODE_Q, "q"},
        {SDL_SCANCODE_R, "r"},
        {SDL_SCANCODE_S, "s"},
        {SDL_SCANCODE_T, "t"},
        {SDL_SCANCODE_U, "u"},
        {SDL_SCANCODE_V, "v"},
        {SDL_SCANCODE_W, "w"},
        {SDL_SCANCODE_X, "x"},
        {SDL_SCANCODE_Y, "y"},
        {SDL_SCANCODE_Z, "z"},
        {SDL_SCANCODE_SPACE, "Space"},
        {SDL_SCANCODE_RETURN, "Enter"},
        {SDL_SCANCODE_ESCAPE, "Escape"},
        {SDL_SCANCODE_TAB, "Tab"},
        {SDL_SCANCODE_BACKSPACE, "Backspace"},
        {SDL_SCANCODE_DELETE, "Delete"},
        {SDL_SCANCODE_LEFT, "Left Arrow"},
        {SDL_SCANCODE_RIGHT, "Right Arrow"},
        {SDL_SCANCODE_UP, "Up Arrow"},
        {SDL_SCANCODE_DOWN, "Down Arrow"},
        {SDL_SCANCODE_LSHIFT, "Left Shift"},
        {SDL_SCANCODE_RSHIFT, "Right Shift"},
        {SDL_SCANCODE_LCTRL, "Left Control"},
        {SDL_SCANCODE_RCTRL, "Right Control"},
        {SDL_SCANCODE_LALT, "Left Alt"},
        {SDL_SCANCODE_RALT, "Right Alt"},
        {SDL_SCANCODE_CAPSLOCK, "Caps Lock"},
        {SDL_SCANCODE_F1, "F1"},
        {SDL_SCANCODE_F2, "F2"},
        {SDL_SCANCODE_F3, "F3"},
        {SDL_SCANCODE_F4, "F4"},
        {SDL_SCANCODE_F5, "F5"},
        {SDL_SCANCODE_F6, "F6"},
        {SDL_SCANCODE_F7, "F7"},
        {SDL_SCANCODE_F8, "F8"},
        {SDL_SCANCODE_F9, "F9"},
        {SDL_SCANCODE_F10, "F10"},
        {SDL_SCANCODE_F11, "F11"},
        {SDL_SCANCODE_F12, "F12"},
        {SDL_SCANCODE_PRINTSCREEN, "Print Screen"},
        {SDL_SCANCODE_SCROLLLOCK, "Scroll Lock"},
        {SDL_SCANCODE_PAUSE, "Pause"},
        {SDL_SCANCODE_INSERT, "Insert"},
        {SDL_SCANCODE_HOME, "Home"},
        {SDL_SCANCODE_END, "End"},
        {SDL_SCANCODE_PAGEUP, "Page Up"},
        {SDL_SCANCODE_PAGEDOWN, "Page Down"},
        {SDL_SCANCODE_LGUI, "Left GUI"},
        {SDL_SCANCODE_RGUI, "Right GUI"},
        {SDL_SCANCODE_MENU, "Menu"}
    };
    std::map<KeyState, std::string> keystateToStr = {
        {NEUTRAL, "Neutral"},
        {HELD, "Held"},
        {PRESSED, "Pressed"},
        {RELEASED, "Released"}
    };
	for (auto& [key, val] : this->keyStates) {  // 15 is the length of the largest string + 1 in the above list.
        if (val == excludedState) {
            continue;
        }
        std::cout << std::left << std::setw(14) << scancodeToStr[key] << ": " << keystateToStr[val] << std::endl;
	}
}

void Input::staticKeyUpdate() {
	// Update every key state; if it is current pressed, make it held.
	// If it is released, make it neutral.
	// The values for a given key will be overwritten by updateKeyState 
	// if the function affects that same key.
	for (auto& [key, val] : this->keyStates) {
		val = val == PRESSED ? HELD : val;
		val = val == RELEASED ? NEUTRAL : val;
	}
}

bool Input::getQuit() {
    return this->quit;
}

void Input::updateKeyState(SDL_Scancode key, bool pressed) {
    this->keyStates[key] = pressed && this->keyStates[key] != HELD ? PRESSED : this->keyStates[key];
    this->keyStates[key] = !pressed && this->keyStates[key] != NEUTRAL ? RELEASED : this->keyStates[key];
}
