if (NOT EMSCIPTEN)
    # Fake luac using lua and sol
    project(luac)

    add_executable(luac
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    )

    target_link_libraries(luac PRIVATE lua sol2::sol2)
endif()