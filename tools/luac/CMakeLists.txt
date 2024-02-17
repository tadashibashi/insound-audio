if (NOT EMSCIPTEN)
    # Fake luac using lua and sol
    project(luac)

    add_executable(luac
            main.cpp
    )

    target_link_libraries(luac PRIVATE lua sol2::sol2)
endif()