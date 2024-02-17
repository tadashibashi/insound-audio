# Tools to assist OS build process
project(insound-tools)

if (NOT EMSCRIPTEN) # no need to build to webassembly
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
    add_subdirectory(luac)
endif()
