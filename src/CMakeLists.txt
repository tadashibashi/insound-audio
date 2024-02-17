add_subdirectory(insound)

if (INSOUND_BUILD_TESTS)
    add_subdirectory(test/cpp)
endif()

project(insound-audio)

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE insound-audio-lib)

# Make text file embeddable
function(make_includable INPUT_FILE OUTPUT_FILE)
    file(READ ${INPUT_FILE} CONTENT)
    set(DELIMETER "__c++_include__")
    set(CONTENT "R\"${DELIMETER}(\n${CONTENT})${DELIMETER}\"")

    file(WRITE ${OUTPUT_FILE} "${CONTENT}")
endfunction()

make_includable(${CMAKE_SOURCE_DIR}/src/lua/driver.lua
    ${CMAKE_SOURCE_DIR}/src/insound/embed/driver.lua.h)
