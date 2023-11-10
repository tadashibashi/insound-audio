# ---------------------------------------------------------------------------- #
# Targetable library for fmod with includes and libs set
# Example:
#   target_link_libraries( ${PROJECT_NAME} PRIVATE fmod )
# Platforms supported:
# - WebAssembly
# Make sure to add the Emscripten.cmake toolchain file in the configuration
# step, e.g.:
# cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=<path/to/Emscripten.cmake>
project(fmod)


# Flags necessary for FMOD Module
set (CMAKE_EXE_LINKER_FLAGS )

add_library(${PROJECT_NAME} INTERFACE)

target_link_options(${PROJECT_NAME} INTERFACE "-sEXPORTED_RUNTIME_METHODS=cwrap,setValue,getValue")
target_link_libraries(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/fmod_wasm.a)

target_include_directories(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

