add_subdirectory(insound)
add_subdirectory(bindings)
add_subdirectory(tools)

if (INSOUND_BUILD_TESTS)
    add_subdirectory(test/cpp)
endif()

# Make text file embeddable
function(make_includable INPUT_FILE OUTPUT_FILE)
    file(READ ${INPUT_FILE} CONTENT)
    set(DELIMETER "__c++_include__")
    set(CONTENT "R\"${DELIMETER}(\n${CONTENT})${DELIMETER}\"")

    file(WRITE ${OUTPUT_FILE} "${CONTENT}")
endfunction()

make_includable(${CMAKE_SOURCE_DIR}/src/insound/lua/driver.lua
    ${CMAKE_SOURCE_DIR}/src/insound/lua/embed/driver.lua.h)
