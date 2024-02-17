add_subdirectory(fmod)
add_subdirectory(sol2)

# Lua library
add_library(lua lua/onelua.c)
target_compile_definitions(lua PRIVATE -DMAKE_LIB)
target_include_directories(lua PUBLIC lua)
