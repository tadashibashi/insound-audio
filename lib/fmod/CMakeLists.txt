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

add_library(${PROJECT_NAME} INTERFACE)

target_link_options(${PROJECT_NAME} INTERFACE
    -sEXPORTED_RUNTIME_METHODS=cwrap,setValue,getValue
    -sEXPORTED_FUNCTIONS=_main,_free,_malloc
    -sNO_DYNAMIC_EXECUTION=1
    -sALLOW_MEMORY_GROWTH=1
)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    target_link_libraries(${PROJECT_NAME} INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/fmodL_wasm.a)
else()
    target_link_libraries(${PROJECT_NAME} INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/fmod_wasm.a)
endif()

target_include_directories(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
