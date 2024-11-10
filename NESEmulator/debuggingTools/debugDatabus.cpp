#include "debugDatabus.h"
#include "../6502Chip/CPU.h"
#include <iostream>

DebugDatabus::DebugDatabus() : DataBus() {}
DebugDatabus::DebugDatabus(Memory* memory) : DataBus(memory) {}
DebugDatabus::~DebugDatabus() {}

void DebugDatabus::undoMemAction(bool supressWarning) {
	if (this->memOps.size() == 0) {  // Only try to undo an action if there are any operations in the stack.
		if (!supressWarning) {
			std::cout << "Warning: attempting to undo operation when there are no operations to undo; nothing has been done." << std::endl;
		}
	} else {  // Undoing an action.
		// First, get the last action's inerse.
		DatabusAction lastAction = this->memOps.top();
		this->memOps.pop();
		// Then get it's inverse.
		DatabusAction inverseAct = lastAction.getInverseAction();
		// Finally, perform this action.
		this->performMemAction(inverseAct);
	}
}

uint8_t DebugDatabus::read(uint16_t address, bool pushAction) {
	uint8_t data = DataBus::read(address);
	if (pushAction) {
		this->memOps.push(DatabusAction(address, data));
	}
	return data;
}

uint8_t DebugDatabus::write(uint16_t address, uint8_t value, bool pushAction) {
	uint8_t oldValue = DataBus::read(address);
	if (pushAction) {
		this->memOps.push(DatabusAction(address, value, oldValue));
	}
	DataBus::write(address, value);
	return value;
}

uint8_t DebugDatabus::performMemAction(DatabusAction action) {
	if (action.isRead) {  // Nothing interesting happens w/ read operations.
		return action.value;
	} else {  // With a read action, however, we send a write command to the databus. We do not use this-> because we don't want to add this action to the action stack.
		DataBus::write(action.address, action.value);
	}

	return 0;
}
