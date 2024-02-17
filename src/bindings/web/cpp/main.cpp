#include <sol/sol.hpp>
#include <insound/lua/embed/driver.lua.h>
#include <iostream>

// Emscripten modules needs a main function
int main()
{
    sol::state lua;
    lua.open_libraries(
        sol::lib::base,
        sol::lib::coroutine,
        sol::lib::math,
        sol::lib::string,
        sol::lib::table,
        sol::lib::utf8);

    std::string_view view((const char *)luaDriver, sizeof(luaDriver) / sizeof(luaDriver[0]));

    std::cout << "Running bytecode script\n";
    auto result = lua.safe_script(view);

    if (result.valid())
    {
        std::cout << "Getting test_val from lua state\n";
        std::cout << "test val, should be 12: " << lua["test_val"].get<int>() << '\n';
    }
    else
    {
        sol::error err = result;
        std::cerr << err.what() << '\n';
    }

    return 0;
}
