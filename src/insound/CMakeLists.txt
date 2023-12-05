project(insound-audio-lib)

file(GLOB_RECURSE ${PROJECT_NAME}_SRC *.cpp)

option(INSOUND_ENVIRONMENT "Target environment for code. E.g. web, node" web)

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})
target_link_libraries(${PROJECT_NAME} PUBLIC fmod sol2::sol2)
target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/lib/lua

    PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

# Set emscripten compiler flags

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    target_link_options(${PROJECT_NAME} PUBLIC
        -sMODULARIZE -lembind -sNO_DYNAMIC_EXECUTION=1
        -sEXPORT_NAME="AudioModule"
        -sASSERTIONS=1 -sNO_DISABLE_EXCEPTION_CATCHING=1 -O0
        -sFILESYSTEM=0 -fexceptions
        -sWASM_BIGINT=1 -sENVIRONMENT=${INSOUND_ENVIRONMENT}
        -sALLOW_MEMORY_GROWTH=1
    )
    target_compile_options(${PROJECT_NAME} PUBLIC
        -O0 -fexceptions
    )
else()
    target_link_options(${PROJECT_NAME} PUBLIC
        -sMODULARIZE -lembind -sNO_DYNAMIC_EXECUTION=1
        -sEXPORT_NAME="AudioModule"
        -sASSERTIONS=0 -sNO_DISABLE_EXCEPTION_CATCHING=0 -Os
        -sFILESYSTEM=0 -flto
        -sWASM_BIGINT=1 -sENVIRONMENT=${INSOUND_ENVIRONMENT}
        -sALLOW_MEMORY_GROWTH=1
    )
    target_compile_options(${PROJECT_NAME} PUBLIC
        -flto -Os
    )
endif()

