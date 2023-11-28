#include "Parameter.h"

#include <utility>

namespace Insound
{
    ParameterMgr::ParameterMgr()
    {

    }

    Parameter &ParameterMgr::operator[](std::string_view name)
    {
        for (auto &param : m_params)
        {
            if (param.getName() == name)
                return param;
        }

        throw std::runtime_error("Param with name \"" + std::string(name) +
            "\" does not exist in this container.");
    }

    const Parameter &ParameterMgr::operator[](std::string_view name) const
    {
        for (auto &param : m_params)
        {
            if (param.getName() == name)
                return param;
        }

        throw std::runtime_error("Param with name \"" + std::string(name) +
            "\" does not exist in this container.");
    }

    void ParameterMgr::set(size_t i, float value)
    {
        auto &p = m_params.at(i);

        if (p.getFloat() == value)
            return;

        p.setValue(value);

        if (m_callback)
            m_callback(p.getName(), i, value);
    }

    void ParameterMgr::set(std::string_view name, float value)
    {
        size_t i = 0;
        for (auto &p : m_params)
        {
            auto pName = p.getName();
            if (pName == name)
            {
                if (p.getFloat() == value)
                    return;
                p.setValue(value);

                if (m_callback)
                    m_callback(pName, i, value);
                return;
            }

            ++i;
        }

        throw std::runtime_error("Param with name \"" + std::string(name) +
            "\" does not exist in this container.");
    }

    float ParameterMgr::get(size_t i) const
    {
        return m_params.at(i).getFloat();
    }

    float ParameterMgr::get(std::string_view name) const
    {
        for (auto &p : m_params)
        {
            if (p.getName() == name)
                return p.getFloat();
        }

        throw std::runtime_error("Param with name \"" + std::string(name) +
            "\" does not exist in this container.");
    }

    void ParameterMgr::setCallback(Callback callback)
    {
        m_callback.swap(callback);
    }

    const ParameterMgr::Callback &
    ParameterMgr::getCallback() const
    {
        return m_callback;
    }
}
