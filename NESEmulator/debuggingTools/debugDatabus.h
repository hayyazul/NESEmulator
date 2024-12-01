// debugDatabus.h - A databus extended w/ debugger features, including a memory, a way to undo operations, etc. Meant to be controlled
#pragma once
#include <stdint.h>
#include <stack>
#include <vector>
#include "../databus/databus.h"
#include "../databus/nesDatabus.h"

// A small container which has some info on a given action; this is a command pattern.
struct DatabusAction {
	bool isRead;  // false = Write; true = Read
	bool isPPUAction;  // Whether the databus affected a PPU register.
	uint16_t address; 
	uint8_t value;  // Value read for Read, value written for Write.
	uint8_t oldValue;  // Value present before Write. Same as value for Read.

	DatabusAction() {};
	DatabusAction(uint16_t address, uint8_t valueRead) : isRead(true), address(address), value(valueRead), oldValue(valueRead) {
		this->isPPUAction = address >= 0x2000 && address <= 0x2008;  // TODO: include registers located at 0x400n.
	};
	DatabusAction(uint16_t address, uint8_t valueWritten, uint8_t oldValue) : isRead(false), address(address), value(valueWritten), oldValue(oldValue) {
		this->isPPUAction = address >= 0x2000 && address <= 0x2008;  // TODO: include registers located at 0x400n.
	};
	
	// Returns the action which when done "undos" this action
	DatabusAction getInverseAction() {
		if (isRead) {
			return DatabusAction(this->address, this->value);
		} else {
			return DatabusAction(this->address, this->oldValue, this->value);
		}
	}
};

class DebugDatabus : public NESDatabus {  // NOTE: This seems a bit finicky; it's difficult to inherit and replace the vanilla Databus.
public:
	DebugDatabus();
	DebugDatabus(Memory* memory);
	~DebugDatabus();

	// Sets this->recordActions to the value given.
	void setRecordActions(bool record);
	bool getRecordActions() const;  // Gets this->recordActions
	void clearRecordedActions();
	unsigned int getNumActions() const;  
	// Undos an action performed. Returns whether the undo was successful.
	bool undoMemAction(bool supressWarning=true);

	const std::stack<DatabusAction>* getMemOps() const;
	
	// Inherited memory operations; this time, what they do is recorded (assuming recordActions is true).
	uint8_t read(uint16_t address) override;
	uint8_t write(uint16_t address, uint8_t value) override;

private:
	bool recordActions;  // Whether to record actions or not.
	std::stack<DatabusAction> memOps;  // A history of databus action performed
	
	// Performs a given DatabusAction. This function may act as both a read or a write. Does not affect the stack of operations.
	uint8_t performMemAction(DatabusAction action);
};

// Displays a dump of memory
void displayMemDump(std::vector<uint8_t>& dump, uint16_t startAddr, uint16_t endAddr, unsigned int bytesPerRow=16);

void displayMemDumpLine(std::vector<uint8_t>& dump, uint16_t startAddr, uint16_t endAddr, unsigned int row, unsigned int bytesPerRow);