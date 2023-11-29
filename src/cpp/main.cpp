#include "LuaDriver.h"
#include <iostream>

int main()
{
    Insound::LuaDriver driver;
    auto result = driver.load(
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

    if (!result)
    {
        std::cerr << driver.getError() << '\n';
        return -1;
    }

    result = driver.doInit();
    if (!result)
    {
        std::cerr << "Init error: " << driver.getError() << '\n';
        return -1;
    }

    return 0;
}
