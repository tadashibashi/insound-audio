#include "EventDescription.h"
#include "EventDescription.Impl.h"

namespace Insound::Audio
{
    EventDescription::EventDescription(FMOD::System *sys) : m(new Impl(sys))
    { }

    EventDescription::~EventDescription()
    {
        delete m;
    }

    bool EventDescription::isFSB() const
    {
        return m->isFSB();
    }

    bool EventDescription::isLoaded() const
    {
        return m->isLoaded();
    }

    void EventDescription::unload()
    {
        m->unload();
    }

    void EventDescription::loadFSB(void *data, size_t size)
    {
        m->loadFSB(data, size);
    }
}
