#pragma once

namespace Insound
{
    class NumberParam
    {
    public:
        NumberParam() : min(), max(), step(), value() { }
        NumberParam(int min, int max, int def) :
            min(min), max(max), step(1), value(def) { }
        NumberParam(float min, float max, float step,
            float def) : min(min), max(max), step(step), value(def) { }

        float min, max, step, value;
    };
}
