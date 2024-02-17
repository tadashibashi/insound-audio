project(insound-audio-lib)

file(GLOB_RECURSE ${PROJECT_NAME}_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE fmod PUBLIC sol2::sol2 lua)
target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

# Flags for building on the web via Emscripten
if (EMSCRIPTEN)
    if (NOT INSOUND_ENVIRONMENT)
        set (INSOUND_ENVIRONMENT "web")
    endif()

    if (NOT INSOUND_MODULE_NAME)
        set (INSOUND_MODULE_NAME "AudioModule")
    endif()

    # Set emscripten compiler flags
    if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
        target_link_options(${PROJECT_NAME} PUBLIC
            -sMODULARIZE -lembind -sNO_DYNAMIC_EXECUTION=1
            -sEXPORT_NAME=${INSOUND_MODULE_NAME}
            -sASSERTIONS=1 -O0
            -fwasm-exceptions
            -sENVIRONMENT=${INSOUND_ENVIRONMENT}
            -sALLOW_MEMORY_GROWTH=1
            -sINITIAL_MEMORY=536870912
            -sFORCE_FILESYSTEM=1
        )
        target_compile_options(${PROJECT_NAME} PUBLIC
            -O0 -fwasm-exceptions
        )
        target_compile_definitions(${PROJECT_NAME} PUBLIC INS_DEBUG=1)
    else()
        target_link_options(${PROJECT_NAME} PUBLIC
            -sMODULARIZE -lembind -sNO_DYNAMIC_EXECUTION=1
            -sEXPORT_NAME=${INSOUND_MODULE_NAME}
            -sASSERTIONS=1 -Os
            -flto
            -sENVIRONMENT=${INSOUND_ENVIRONMENT}
            -sALLOW_MEMORY_GROWTH=1 -fwasm-exceptions
            -sINITIAL_MEMORY=536870912
            -sFORCE_FILESYSTEM=1
        )
        target_compile_options(${PROJECT_NAME} PUBLIC
            -flto -Os -fwasm-exceptions
        )
        target_compile_definitions(${PROJECT_NAME} PUBLIC INS_DEBUG=0)
    endif()

endif()