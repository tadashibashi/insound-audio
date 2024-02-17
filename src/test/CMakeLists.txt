project(insound-audio-test)

add_subdirectory(lib/Catch2)

file(GLOB_RECURSE TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(${PROJECT_NAME} ${TEST_SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE Catch2::Catch2 insound-audio-lib)
