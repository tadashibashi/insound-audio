#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <iostream>
const std::string driverScript =
#include "embed/driver.lua.h"
;

int main()
{
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::coroutine,
        sol::lib::string, sol::lib::table, sol::lib::utf8);
    auto result = lua.script(driverScript);

    if (!result.valid())
    {
        sol::error e = result;
        std::cerr << "Failed to load lua driver: "<< e.what() << '\n';
        return -1;
    }

    try {
        auto load_script = lua.get<sol::protected_function>("load_script");
        if (load_script.valid())
        {
            auto result2 = load_script(
                R"Lua(
                    local volume = 0

                    function on_init()
                        print("Initializing Lua Script.")
                        print("Running sandbox with " .. _VERSION)
                    end

                    function on_load()
                        print("Loaded file")
                    end
                )Lua"
            );

            if (!result2.valid())
            {
                sol::error e = result2;
                std::cerr << "Error: " << e.what() << '\n';
            }
        }
    }
    catch (...)
    {
        std::cout << "Unknown Error while running user script." << '\n';
        throw;
    }



    return 0;
}
