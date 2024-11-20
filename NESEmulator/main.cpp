// main.cpp : Defines the entry point for the application.
//

#include "NESEmulator.h"

// TODO: 
// - Study PPU

#include "input/cmdInput.h"
#include "debuggingTools/debugDatabus.h"
#include "debuggingTools/NESDebug.h"
#include "debuggingTools/CPUAnalyzer.h"

#include "debuggingTools/basicDebugSuite.hpp"
#include "loadingData/parseLogData.h"

#include "databus/nesDatabus.h"
#include "memory/cartridgeData.h"
#include "memory/ram.h"

#undef main  // Deals w/ the definition of main in SDL.
int main() {
	debuggingSuite();
	return 0;
}