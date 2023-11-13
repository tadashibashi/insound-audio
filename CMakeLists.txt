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
target_link_libraries(${PROJECT_NAME} PRIVATE fmod)
target_link_options(${PROJECT_NAME} PRIVATE
    -sMODULARIZE -sEXPORT_ES6=1 -lembind -sNO_DYNAMIC_EXECUTION=1
    -sASSERTIONS=1 -sNO_DISABLE_EXCEPTION_CATCHING=1
)
