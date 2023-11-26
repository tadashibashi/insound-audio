cmake_minimum_required(VERSION 3.18.4)
project(insound-audio)

# Ensure Emscripten-only compilations
if (NOT EMSCRIPTEN)
    message (FATAL_ERROR "Insound Audio Engine only supports Emscripten")
endif()

set (CMAKE_CXX_STANDARD 20)

add_subdirectory(lib)

file(GLOB_RECURSE ${PROJECT_NAME}_SRC src/cpp/*.cpp)
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE fmod sol2::sol2)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/lib/lua)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    target_link_options(${PROJECT_NAME} PRIVATE
        -sMODULARIZE -lembind -sNO_DYNAMIC_EXECUTION=1
        -sEXPORT_NAME="AudioModule"
        -sASSERTIONS=1 -sNO_DISABLE_EXCEPTION_CATCHING=1 -O0
        -sFILESYSTEM=0 -fexceptions
        -sWASM_BIGINT=1 -sENVIRONMENT=web -sALLOW_MEMORY_GROWTH=1
    )
    target_compile_options(${PROJECT_NAME} PRIVATE
        -O0 -fexceptions
    )
else()
    target_link_options(${PROJECT_NAME} PRIVATE
        -sMODULARIZE -lembind -sNO_DYNAMIC_EXECUTION=1
        -sEXPORT_NAME="AudioModule"
        -sASSERTIONS=0 -sNO_DISABLE_EXCEPTION_CATCHING=0 -Os
        -sFILESYSTEM=0 -flto
        -sWASM_BIGINT=1 -sENVIRONMENT=web -sALLOW_MEMORY_GROWTH=1
    )
    target_compile_options(${PROJECT_NAME} PRIVATE
        -flto -Os
    )
endif()

# Make scripts embeddable
function(make_includable INPUT_FILE OUTPUT_FILE)
    file(READ ${INPUT_FILE} CONTENT)
    set(DELIMETER "__c++_include__")
    set(CONTENT "R\"${DELIMETER}(\n${CONTENT})${DELIMETER}\"")

    file(WRITE ${OUTPUT_FILE} "${CONTENT}")
endfunction()

make_includable(${CMAKE_SOURCE_DIR}/src/lua/driver.lua
    ${CMAKE_SOURCE_DIR}/src/cpp/embed/driver.lua.h)
