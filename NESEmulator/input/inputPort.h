#pragma once
/* inputPort.h
	A standard input port on the NES. Acts as a middleman between
the controller and the NES's databus.
*/

#include <stdint.h>

class StandardController;

class InputPort {
public:
	InputPort();
	InputPort(StandardController* controller_to_attach);
	~InputPort();

	void attachController(StandardController* controller);
	void deattachController();

	// Sets the latch value associated w/ the controller--- if any is attached.
	void setLatch(bool val);

	// Reads the input associated w/ the controller AND clocks it.
	uint8_t readAndClock();

private:
	StandardController* controller;
};