# CMakeList.txt : CMake project for NESEmulator, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (NESEmulator "NESEmulator.cpp" "NESEmulator.h"  "6502Chip/CPU.h" "6502Chip/CPU.cpp" "databus/databus.h" "databus/databus.cpp" "instructions/instructions.h" "instructions/instructions.cpp" "memory/memory.h" "memory/memory.cpp" "main.cpp" "loadingData/parseNESFiles.h" "loadingData/parseNESFiles.cpp" "debuggingTools/NESDebug.h" "debuggingTools/NESDebug.cpp" "input/input.h" "input/input.cpp" "input/cmdInput.h" "input/cmdInput.cpp" "debuggingTools/debugDatabus.h" "debuggingTools/debugDatabus.cpp" "debuggingTools/CPUAnalyzer.cpp"    "memory/ram.h" "memory/ram.cpp" "memory/cartridgeData.h" "memory/cartridgeData.cpp" "databus/nesDatabus.h" "databus/nesDatabus.cpp" "databus/ppuDatabus.h" "databus/ppuDatabus.cpp" "ppu/ppu.h" "ppu/ppu.cpp"   "debuggingTools/PPUDebug.h" "debuggingTools/PPUDebug.cpp" "graphics/graphics.h" "graphics/graphics.cpp" "debuggingTools/debugDisplays/tableDisplayer.h" "debuggingTools/debugDisplays/tableDisplayer.cpp" "DMA/directMemoryAccess.h" "DMA/directMemoryAccess.cpp" "debuggingTools/debugDisplays/paletteDisplayer.h" "debuggingTools/debugDisplays/paletteDisplayer.cpp" "loadingData/loadPalette.cpp" "memory/secondaryOAM.h" "memory/secondaryOAM.cpp" "debuggingTools/debugSuiteInput.h" "debuggingTools/debugSuiteInput.cpp" "debuggingTools/suites/generalDebugSuite.h" "debuggingTools/suites/generalDebugSuite.cpp")
file(REMOVE REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/testROMS" )
file(COPY "testROMS" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(REMOVE REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/resourceFiles" )
file(COPY "resourceFiles" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(SDL2_DIR "C:/vclib/SDL2-2.30.1/cmake")
set(SDL2_IMAGE_DIR "C:/vclib/SDL2_image-devel-2.8.2-VC/SDL2_image-2.8.2/cmake")  

find_package(SDL2 REQUIRED)
find_package(SDL2_IMAGE REQUIRED)     

# Note: I modified the cmake file for SDL2 Image by setting SDL2_IMAGE_INCLUDE_DIR and SDL2_IMAGE_LIBRARY to the appropriate values in the file.
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_IMAGE_INCLUDE_DIR})

target_link_libraries(NESEmulator ${SDL2_LIBRARIES})
target_link_libraries(NESEmulator ${SDL2_IMAGE_LIBRARY})


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET NESEmulator PROPERTY CXX_STANDARD 20)
endif()
