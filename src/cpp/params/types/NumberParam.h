#pragma once

namespace Insound
{
    class NumberParam
    {
    public:
        NumberParam() : m_min(), m_max(), m_step(), m_default() { }
        NumberParam(int min, int max, int def) :
            m_min(min), m_max(max), m_step(1), m_default(def) { }
        NumberParam(float min, float max, float step,
            float def) : m_min(min), m_max(max), m_step(step), m_default(def) { }

        [[nodiscard]]
        float min() const { return m_min; }
        [[nodiscard]]
        float max() const { return m_max; }
        [[nodiscard]]
        float step() const { return m_step; }
        [[nodiscard]]
        float defaultValue() const { return m_default; }

    public:
        float m_min, m_max, m_step, m_default;
    };
}
