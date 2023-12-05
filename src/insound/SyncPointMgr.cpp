#include "SyncPointMgr.h"
#include "common.h"
#include "fmod_common.h"
#include <fmod.hpp>

namespace Insound
{
    SyncPointMgr::SyncPointMgr() : m_sound(), m_points() { }

    SyncPointMgr::SyncPointMgr(FMOD::Sound *sound)
        : m_sound(), m_points()
    {
        load(sound);
    }

    void SyncPointMgr::load(FMOD::Sound *sound)
    {
        int numSyncPoints;
        checkResult( sound->getNumSyncPoints(&numSyncPoints) );

        std::vector<SyncPoint> points;
        points.reserve(numSyncPoints);

        for (int i = 0; i < numSyncPoints; ++i)
        {
            FMOD_SYNCPOINT *fPoint;
            checkResult( sound->getSyncPoint(i, &fPoint) );
            char buffer[256];
            checkResult( sound->getSyncPointInfo(fPoint, buffer, 256,
                nullptr, 0));
            points.emplace_back(buffer, fPoint);
        }

        m_points.swap(points);
        m_sound = sound;
    }

    void SyncPointMgr::clear()
    {
        m_sound = nullptr;
        m_points.clear();
    }

    const std::string &SyncPointMgr::getLabel(size_t i) const
    {
        return m_points.at(i).label();
    }

    unsigned int SyncPointMgr::getOffsetSamples(size_t i) const
    {
        unsigned int offset;
        checkResult( m_sound->getSyncPointInfo(m_points.at(i).point(),
            nullptr, 0, &offset, FMOD_TIMEUNIT_PCM));
        return offset;
    }

    std::optional<unsigned int>
    SyncPointMgr::getOffsetSamples(std::string_view label) const
    {
        size_t index = 0;
        for (auto &point : m_points)
        {
            if (compareCaseInsensitive(point.label(), label))
                return getOffsetSamples(index);

            ++index;
        }

        return {};
    }

    unsigned int SyncPointMgr::getOffsetMS(size_t i) const
    {
        unsigned int offset;
        checkResult( m_sound->getSyncPointInfo(m_points.at(i).point(),
            nullptr, 0, &offset, FMOD_TIMEUNIT_MS));
        return offset;
    }

    std::optional<unsigned int>
    SyncPointMgr::getOffsetMS(std::string_view label) const
    {
        size_t index = 0;
        for (auto &point : m_points)
        {
            if (point.label() == label)
                return getOffsetMS(index);

            ++index;
        }

        return {};
    }

    double SyncPointMgr::getOffsetSeconds(size_t i) const
    {
        return (double)getOffsetMS(i) * .001;
    }

    std::optional<double>
    SyncPointMgr::getOffsetSeconds(std::string_view label) const
    {
        size_t index = 0;
        for (auto &point : m_points)
        {
            if (point.label() == label)
                return getOffsetSeconds(index);

            ++index;
        }

        return {};
    }

    size_t SyncPointMgr::size() const
    {
        return m_points.size();
    }



}
