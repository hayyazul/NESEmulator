# CMakeList.txt : CMake project for NESEmulator, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (NESEmulator "NESEmulator.cpp" "NESEmulator.h"  "6502Chip/CPU.h" "6502Chip/CPU.cpp" "databus/databus.h" "databus/databus.cpp" "instructions/instructions.h" "instructions/instructions.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET NESEmulator PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
