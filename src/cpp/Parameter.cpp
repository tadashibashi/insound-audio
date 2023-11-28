#include "Parameter.h"

#include <utility>

namespace Insound
{
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

    void ParameterMgr::set(size_t i, std::string_view label)
    {
        auto val = m_params.at(i).labels()[label].value;
        set(i, val);
    }

    void ParameterMgr::set(std::string_view name, float value)
    {
        set(findParam(name), value);
    }

    void ParameterMgr::set(std::string_view name, std::string_view label)
    {
        auto index = findParam(name);
        auto val = m_params[index].labels()[label].value;
        set(index, val);
    }

    float ParameterMgr::get(size_t i) const
    {
        return m_params.at(i).getFloat();
    }

    float ParameterMgr::getInitValue(size_t i) const
    {
        return m_params.at(i).getInitValue();
    }

    float ParameterMgr::get(std::string_view name) const
    {
        return m_params[findParam(name)].getFloat();
    }

    float ParameterMgr::getInitValue(std::string_view name) const
    {
        return m_params[findParam(name)].getInitValue();
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

    void ParameterMgr::resetAll()
    {
        size_t i = 0;
        for (auto &p : m_params)
        {
            auto initVal = p.getInitValue();
            p.setValue(initVal);

            if (m_callback)
                m_callback(p.getName(), i, initVal);

            ++i;
        }
    }

    void ParameterMgr::reset(size_t i)
    {
        auto &p = m_params.at(i);
        auto initVal = p.getInitValue();

        p.setValue(initVal);

        if (m_callback)
            m_callback(p.getName(), i, initVal);
    }

    void ParameterMgr::reset(std::string_view name)
    {
        auto index = findParam(name);
        auto initVal = m_params[index].getInitValue();
        set(index, initVal);
    }

    int ParameterMgr::findParam(std::string_view name) const
    {
        int i = 0;
        for (auto &p : m_params)
        {
            if (p.getName() == name)
                return i;
            ++i;
        }

        throw std::runtime_error("Param with name \"" + std::string(name) +
            "\" does not exist in this container");
    }
}
