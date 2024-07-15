cmake_minimum_required(VERSION 3.10)

project(HandmadeHero)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define source files
set(SOURCES
    src/win32/win32-handmade-hero.cpp
    src/win32/win32-input.cpp
    src/win32/win32-file-io.cpp
    src/win32/win32-sound.cpp
    src/win32/win32-clock.cpp
    src/win32/win32-display.cpp
    src/handmade-hero/handmade-hero.cpp)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Set compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE DEV=1 DEBUG=1)

# Set compile options
target_compile_options(
  ${PROJECT_NAME}
  PRIVATE /nologo
          /Oi
          /GR-
          /EHa-
          /MT
          /Gm-
          /Od
          /W4
          /WX
          /wd4201
          /wd4127
          /wd4100
          /FC
          /Z7)

# Set linker options
target_link_options(${PROJECT_NAME} PRIVATE /opt:ref)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE user32.lib gdi32.lib xinput.lib
                                              winmm.lib)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                                 "${CMAKE_BINARY_DIR}/bin")

# Generate PDB file
set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES COMPILE_PDB_NAME ${PROJECT_NAME} COMPILE_PDB_OUTPUT_DIRECTORY
                                              "${CMAKE_BINARY_DIR}/bin")

# Set map file output
set_target_properties(
  ${PROJECT_NAME} PROPERTIES LINK_FLAGS
                             "/MAP:${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME}.map")
