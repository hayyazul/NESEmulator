#include "controller.h"

#include "../globals/helpers.hpp"
#include "../input/input.h"

#include <SDL.h>
#include <map>

_4021::_4021() : ctrl(true), bits(0x00) {}
_4021::~_4021() {}

void _4021::clock(bool serial_in) {
	// If ctrl is false, left-shift it and replace Q1 (bit 0 of bits) w/ serial_in.
	auto a = this->bits << 1;
	auto b = a | serial_in;
	if (!ctrl) this->bits = (this->bits << 1) | serial_in;
}

bool _4021::readOut(int out) const {
	if (this->bits != 0b1111'1111) {
		int i = 0;
	}

	// If out is in range, then get the bit.
	if (6 <= out && out <= 8) return getBit(this->bits, out - 1);
	return false;  // Return false by default if otherwise.
}

void _4021::setCtrl(bool ctrl) {
	this->ctrl = ctrl;
}

void _4021::setRegister(uint8_t bits) {
	if (ctrl) {
		this->bits = bits;
	}
}

StandardController::StandardController() : input_values(0xff), shift_register() {}

StandardController::~StandardController() {}

void StandardController::readInput(Input& input) {
	// On the controller, 1 indicates a button is UP, and 0 indicates a button is DOWN; this is inverted by the NES.
	
	// Maps SDL_Scancodes to NES buttons stored as a byte w/ 1 bit toggled.
	const std::map<SDL_Scancode, uint8_t> KEY_BUTTON_MAP = { { SDL_SCANCODE_RIGHT, 0b0000'0001 },  
 															 { SDL_SCANCODE_LEFT,  0b0000'0010 },
 															 { SDL_SCANCODE_DOWN,  0b0000'0100 },
															 { SDL_SCANCODE_UP,    0b0000'1000 },
															 { SDL_SCANCODE_W,     0b0001'0000 },   // Start
															 { SDL_SCANCODE_Q,     0b0010'0000 },   // Select
															 { SDL_SCANCODE_S,     0b0100'0000 },   // B
															 { SDL_SCANCODE_A,     0b1000'0000 } }; // A
	this->input_values = 0x00;  // The ORed bits of the input according to the map.

	// For every key being checked, if it is down or held, OR it with the byte indicating which keys are held.
	for (auto const& [key, val] : KEY_BUTTON_MAP) {
		KeyState key_state = input.getKeyState(key);
		if (key_state == PRESSED || key_state == HELD) {
			this->input_values |= val;
		}
	}
}

void StandardController::update4021() {
	this->shift_register.setRegister(~this->input_values);
}

void StandardController::clock() {
	this->shift_register.clock(1);
}

void StandardController::setLatch(bool value) {
	this->shift_register.setCtrl(value);
}

bool StandardController::getData() const {
	return this->shift_register.readOut(8);
}
