#pragma once

#include <insound/math/Vec2.h>

namespace Insound::Math
{
    /**
     * Base interface for a curve object
     */
    class Curve
    {
    public:
        /** Get the value of a curve at a particular point
         * @param t - progress on curve, must be between 0 and 1, inclusive
         */
        [[nodiscard]]
        virtual Vec2f value(float t) const = 0;

        /**
         * Number of points in the curve
         */
        [[nodiscard]]
        virtual int pointCount() const = 0;

        /**
         * Set a point in the curve
         * @param index - point index, range: (0-pointCount]
         * @param p     - value to set
         */
        virtual void setPoint(int index, Vec2f p) = 0;

        /**
         * Get a point's value
         * @param index - point index, range: (0-pointCount]
         */
        [[nodiscard]]
        virtual Vec2f getPoint(int index) const = 0;
    };


}
