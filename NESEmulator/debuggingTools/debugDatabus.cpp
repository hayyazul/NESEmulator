#include "debugDatabus.h"
#include "../6502Chip/CPU.h"
#include <iostream>
#include <minmax.h>

DebugDatabus::DebugDatabus() : NESDatabus(), recordActions(false) {
	this->clearRecordedActions();
}
DebugDatabus::DebugDatabus(Memory* memory) : NESDatabus(memory), recordActions(false) {
	this->clearRecordedActions();
}
DebugDatabus::~DebugDatabus() {}

void DebugDatabus::setRecordActions(bool record) {
	this->recordActions = record;
}

bool DebugDatabus::getRecordActions() const {
	return this->recordActions;
}

void DebugDatabus::clearRecordedActions() {
	this->memOps = std::stack<DatabusAction>();
}

unsigned int DebugDatabus::getNumActions() const {
	return this->memOps.size();
}

bool DebugDatabus::undoMemAction(bool supressWarning) {
	if (this->memOps.size() == 0) {  // Only try to undo an action if there are any operations in the stack.
		if (!supressWarning) {
			std::cout << "Warning: attempting to undo operation when there are no operations to undo; nothing has been done." << std::endl;
		}
		return false;
	} else {  // Undoing an action.
		// First, get the last action's inerse.
		DatabusAction lastAction = this->memOps.top();
		this->memOps.pop();
		// Then get it's inverse.
		DatabusAction inverseAct = lastAction.getInverseAction();
		// Finally, perform this action.
		this->performMemAction(inverseAct);
		return true;
	}
}

uint8_t DebugDatabus::read(uint16_t address) {
	uint8_t data = NESDatabus::read(address);
	if (this->recordActions) {
		this->memOps.push(DatabusAction(address, data));
	}
	return data;
}

uint8_t DebugDatabus::write(uint16_t address, uint8_t value) {
	uint8_t oldValue = NESDatabus::write(address, value);
	if (this->recordActions) {
		this->memOps.push(DatabusAction(address, value, oldValue));
	}
	return value;
}

uint8_t DebugDatabus::performMemAction(DatabusAction action) {
	if (action.isRead) {  // Nothing interesting happens w/ read operations.
		return action.value;
	} else {  // With a read action, however, we send a write command to the databus. We do not use this-> because we don't want to add this action to the action stack.
		NESDatabus::write(action.address, action.value);
	}

	return 0;
}

void displayMemDump(std::vector<uint8_t>& dump, uint16_t startAddr, uint16_t endAddr, unsigned int bytesPerRow) {
	unsigned int rowCount = ceil(static_cast<double>(endAddr - startAddr) / bytesPerRow);
	if (endAddr > startAddr) {
		for (unsigned int i = 0; i < rowCount; ++i) {
			displayMemDumpLine(dump, startAddr, endAddr, i, bytesPerRow);
		}
	}
}

void displayMemDumpLine(std::vector<uint8_t>& dump, uint16_t startAddr, uint16_t endAddr, unsigned int row, unsigned int bytesPerRow) {
	std::cout << displayHex(startAddr + row * bytesPerRow, 4) << " | ";
	unsigned int rowSize = min((endAddr - startAddr) - row * bytesPerRow, bytesPerRow);  // This gives you how many bytes are left to display in total (not just for this row).
	
	for (unsigned int i = 0; i <= rowSize; ++i) {
		std::cout << displayHex(dump.at(i + row * bytesPerRow), 2) << ' ';
	}
	std::cout << std::endl;
}
