/* controller.h
	Standard NES Controller emulation. I might create an ABC for 
all NES controllers later.
*/
#pragma once

#include <stdint.h>

class Input;

// 8-bit shift register associated w/ the StandardController.
class _4021 {
public:
	_4021();
	~_4021();

	// Clocks the shift unit if ctrl only if false (serial mode); replaces Q1 w/ serial_in.
	void clock(bool serial_in);

	// Returns output for Q8, Q7, or Q6 (see 4021 on nesdev for more info).
	// Any other values return false by default.
	bool readOut(int out) const;

	void setCtrl(bool ctrl);  // Sets the parallel(true) / serial(false) control latch.
	bool getCtrl() const;  // Gets the state of ctrl; this is useful for making sure 

	// Sets the register bits to the given bits ONLY IF ctrl is true (parallel mode).
	void setRegister(uint8_t bits);

private:

	// Parallel(true) / Serial(false) control.
	// Former latches inputs(A, up, etc.) into the bits of the register.
	// Latter allows the usage of clock, which left-shifts the register.
	// If the appropriate ctrl value is not the current one, no effect occurs.
	bool ctrl;  
	uint8_t bits;
};

// Standard NES controller class; reads an Input class for some hard-coded input codes.
class StandardController {
public:
	StandardController();
	~StandardController();

	// Reads in the input and sets input_values given that input.
	void readInput(Input& input);

	// Updates the 4021 shift register w/ the internal input_values.
	void update4021();

	// Clocks the internal 4021 shift register; the serial input given to the 4021 is always 1.
	void clock();

	// Sets the latch (in the 4021) to the given value.
	void setLatch(bool value);

	bool getData() const;  // Gets the Q8 bit value.
private:
	uint8_t input_values;  // Values indicating whether a button has been pressed, translated into a bitstring.
	_4021 shift_register;  // Shift register; used to transfer and shift input data (as a bitstring) in a specific way.
};