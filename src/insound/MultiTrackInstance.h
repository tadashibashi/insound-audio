#pragma once

namespace Insound::Audio
{
    /**
     * Driver for a MultiTrackDescription
     */
    class MultiTrackInstance
    {
    public:
        MultiTrackInstance();
        ~MultiTrackInstance();

        void update();
    private:
        class Impl;
        Impl *m;
    };
}
