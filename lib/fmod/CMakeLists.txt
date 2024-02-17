# ---------------------------------------------------------------------------- #
# Targetable library for fmod with includes and libs set
# Example:
#   target_link_libraries( ${PROJECT_NAME} PRIVATE fmod )
# Platforms supported:
# - MacOS
# - WebAssembly
# Make sure to add the Emscripten.cmake toolchain file in the configuration
# step, e.g.:
# cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=<path/to/Emscripten.cmake>
project(fmod)

add_library(${PROJECT_NAME} INTERFACE)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(DEBUG TRUE)
else()
    set(DEBUG FALSE)
endif()

if (EMSCRIPTEN)
    target_link_options(${PROJECT_NAME} INTERFACE
        -sEXPORTED_RUNTIME_METHODS=cwrap,setValue,getValue
        -sEXPORTED_FUNCTIONS=_main,_free,_malloc
        -sNO_DYNAMIC_EXECUTION=1
        -sALLOW_MEMORY_GROWTH=1
    )

    if (DEBUG)
        target_link_libraries(${PROJECT_NAME} INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/web/fmodL_wasm.a)
    else()
        target_link_libraries(${PROJECT_NAME} INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/web/fmod_wasm.a)
    endif()

elseif(APPLE) # Mac
    if (DEBUG)
        target_link_libraries(${PROJECT_NAME} INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/mac/libfmodL.dylib)
    else()
        target_link_libraries(${PROJECT_NAME} INTERFACE
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/mac/libfmod.dylib)
    endif()
elseif(UNIX) # Non-apple Unix (Linux only)
    if (DEBUG)
        # TODO: cpu processor check for both x86_64 and arm64
        target_link_libraries(${PROJECT_NAME} INTERFACE
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/x86_64/libfmod.so.13.20)
    else()
        target_link_libraries(${PROJECT_NAME} INTERFACE
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/x86_64/libfmodL.so.13.20)
    endif()
elseif(MSVC) # Windows (Microsoft Visual C++ only)
    if (DEBUG)
        target_link_libraries(${PROJECT_NAME} INTERFACE
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/fmod_vc.lib)
    else()
        target_link_libraries(${PROJECT_NAME} INTERFACE
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/fmodL_vc.lib)
    endif()
endif()


target_include_directories(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
