// main.cpp : Defines the entry point for the application.
//

#include "NESEmulator.h"

// TODO: 
// - Debug interrupts
// - Debug cycle len related code (there are some bugs present relating to it).

#include "input/cmdInput.h"
#include "debuggingTools/debugDatabus.h"
#include "debuggingTools/NESDebug.h"
#include "debuggingTools/CPUAnalyzer.h"

#include "debuggingTools/basicDebugSuite.hpp"

#undef main  // Deals w/ the definition of main in SDL.
int main() {
	debuggingSuite();
	return 0;
}