if (EMSCRIPTEN)
    project(insound-audio-bindings-web)

    add_executable(${PROJECT_NAME}
            cpp/main.cpp cpp/bindings.cpp)

    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

    target_link_libraries(${PROJECT_NAME} PRIVATE insound-audio-lib)
endif()
