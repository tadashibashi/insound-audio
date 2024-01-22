#include "Marker.h"
#include "lua.hpp"

#include <sol/property.hpp>


namespace Insound::Scripting
{
    void Marker::inject(const std::string &typeName, sol::table &table)
    {
        sol::usertype<Marker> type = table.new_usertype<Marker>(typeName,
            sol::constructors<Marker()>());
        type["name"] = sol::readonly_property(&Marker::name);
        type["position"] = sol::readonly_property(&Marker::position);
        //type["setName"] = sol::member_function_wrapper()
    }
}
