#pragma once
#include <string>
#include <sol/forward.hpp>

namespace Insound::Scripting {
    struct Marker
    {
        std::string name;
        double seconds;

        /**
         * Inject the class definition into a Lua table.
         *
         * @param name  - class name
         * @param table - the table to inject the definition to
         */
        static void inject(const std::string &name, sol::table &table);
    };
}
