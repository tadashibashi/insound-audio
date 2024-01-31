#include "LuaError.h"
#include "LuaDriver.h"

#include <sol/state.hpp>

Insound::LuaError::LuaError(LuaDriver *driver, std::string message) :
    sol::error(""), m_lineNumber(), m_message()
{
    auto state = driver->context().lua_state();
    lua_Debug ar;
    lua_getstack(state, 1, &ar);
    lua_getinfo(state, "nSl", &ar);

    m_lineNumber = ar.currentline;
    m_message = "[]:" + std::to_string(ar.currentline) + ":" + message;
}
