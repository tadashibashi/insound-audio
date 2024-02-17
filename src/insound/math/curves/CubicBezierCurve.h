#pragma once

#include "Curve.h"

namespace Insound::Math
{
    /**
     * Three-point bezier curve
     */
    class CubicBezierCurve : public Curve
    {
    public:
        CubicBezierCurve() : m_points{} { }

        CubicBezierCurve(Vec2f start, Vec2f end, Vec2f control1, Vec2f control2)
            : m_points{start, control1, control2, end}
        {}

        CubicBezierCurve(Vec2f start, Vec2f end)
        {}

        void setPoint(int index, Vec2f p) override
        {
            m_points[index] = p;
        }

        [[nodiscard]]
        Vec2f getPoint(int index) const override
        {
            return m_points[index];
        }

        [[nodiscard]]
        int pointCount() const override { return 4; }

        /**
         * Get the value of a specific point on the curve
         * @param  t - must be 0 <= t <= 1
         */
        [[nodiscard]]
        Vec2f value(float t) const override;

    private:
        Vec2f m_points[4];
    };
}
