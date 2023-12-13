project(insound-audio-lib)

file(GLOB_RECURSE ${PROJECT_NAME}_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

if (NOT INSOUND_ENVIRONMENT)
    set (INSOUND_ENVIRONMENT "web")
endif()

if (NOT INSOUND_MODULE_NAME)
    set (INSOUND_MODULE_NAME "AudioModule")
endif()

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE fmod PUBLIC sol2::sol2)
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/lua

    PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

# Set emscripten compiler flags
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    target_link_options(${PROJECT_NAME} PUBLIC
        -sMODULARIZE -lembind -sNO_DYNAMIC_EXECUTION=1
        -sEXPORT_NAME=${INSOUND_MODULE_NAME}
        -sASSERTIONS=1 -sNO_DISABLE_EXCEPTION_CATCHING=1 -O0
        -sFILESYSTEM=0 -fexceptions
        -sWASM_BIGINT=1 -sENVIRONMENT=${INSOUND_ENVIRONMENT}
        -sALLOW_MEMORY_GROWTH=1
    )
    target_compile_options(${PROJECT_NAME} PUBLIC
        -O0 -fexceptions
    )
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DINS_DEBUG=1)
else()
    target_link_options(${PROJECT_NAME} PUBLIC
        -sMODULARIZE -lembind -sNO_DYNAMIC_EXECUTION=1
        -sEXPORT_NAME=${INSOUND_MODULE_NAME}
        -sASSERTIONS=1 -sNO_DISABLE_EXCEPTION_CATCHING=1 -Os
        -sFILESYSTEM=0 -flto
        -sWASM_BIGINT=1 -sENVIRONMENT=${INSOUND_ENVIRONMENT}
        -sALLOW_MEMORY_GROWTH=1
    )
    target_compile_options(${PROJECT_NAME} PUBLIC
        -flto -Os
    )
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DINS_DEBUG=0)
endif()
