// debugDatabus.h - A databus extended w/ debugger features, including a memory, a way to undo operations, etc.

#include <stdint.h>
#include <stack>
#include "../databus/databus.h"

// A small container which has some info on a given action; this is a command pattern.
struct DatabusAction {
	bool isRead;  // false = Write; true = Read 
	uint16_t address; 
	uint8_t value;  // Value read for Read, value written for Write.
	uint8_t oldValue;  // Value present before Write. Same as value for Read.

	DatabusAction() {};
	DatabusAction(uint16_t address, uint8_t valueRead) : isRead(true), address(address), value(valueRead), oldValue(valueRead) {};
	DatabusAction(uint16_t address, uint8_t valueWritten, uint8_t oldValue) : isRead(false), address(address), value(valueWritten), oldValue(oldValue) {};
	// Returns the action which when done "undos" this action
	DatabusAction getInverseAction() {
		if (isRead) {
			return DatabusAction(this->address, this->value);
		} else {
			return DatabusAction(this->address, this->oldValue, this->value);
		}
	}
};

class DebugDatabus : public DataBus {
public:
	DebugDatabus();
	DebugDatabus(Memory* memory);
	~DebugDatabus();

	// Undos an action performed.
	void undoMemAction(bool supressWarning=true);
	
	// Inherited memory operations; this time, what they do is recorded.
	// pushAction being true will result in the current memory operation being pushed onto the stack of memory operations.
	uint8_t read(uint16_t address, bool pushAction=true);
	uint8_t write(uint16_t address, uint8_t value, bool pushAction=true);  // Returns the value just written.

private:
	
	// Performs a given DatabusAction. This function may act as both a read or a write. Does not affect the stack of operations.
	uint8_t performMemAction(DatabusAction action);

	std::stack<DatabusAction> memOps;  // A history of dataoperations performed

};