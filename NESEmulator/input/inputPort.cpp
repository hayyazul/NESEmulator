#include "inputPort.h"

#include "controller.h"

InputPort::InputPort() : controller(nullptr) {}
InputPort::InputPort(StandardController* controller_to_attach) : controller(controller_to_attach) {}
InputPort::~InputPort() {}

void InputPort::attachController(StandardController* controller) {
	this->controller = controller;
}

void InputPort::deattachController() {
	this->controller == nullptr;
}

void InputPort::setLatch(bool val) {
	if (this->controller == nullptr) return;  // Don't attempt to set the latch of a non-existent controller.
	this->controller->setLatch(val);
}

uint8_t InputPort::readAndClock() {
	if (this->controller == nullptr) return 0;  // Don't attempt to access a non-existent controller.
	uint8_t val = this->controller->getData();
	this->controller->clock();
	return val;
}

