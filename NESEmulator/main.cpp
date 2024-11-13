// main.cpp : Defines the entry point for the application.
//

#include "NESEmulator.h"

// TODO: 
// - Debug interrupts
// - Implement extra-cycle instructions (i.e., when a page boundary is crossed for some instructions, add a cycle).

#include "input/cmdInput.h"
#include "debuggingTools/debugDatabus.h"
#include "debuggingTools/NESDebug.h"
#include "debuggingTools/CPUAnalyzer.h"

#undef main  // Deals w/ the definition of main in SDL.
int main() {
	return 0;
}