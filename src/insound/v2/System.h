#pragma once

namespace Insound::Audio
{
    class System
    {
    public:
        System();
        ~System();

    private:
        class Impl;
        Impl *m;
    };
}
