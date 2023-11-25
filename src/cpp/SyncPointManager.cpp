#include "SyncPointManager.h"
#include "common.h"
#include "fmod_common.h"
#include <fmod.hpp>

namespace Insound
{
    SyncPointManager::SyncPointManager() : m_sound(), m_points() { }

    SyncPointManager::SyncPointManager(FMOD::Sound *sound)
        : m_sound(), m_points()
    {
        load(sound);
    }

    void SyncPointManager::load(FMOD::Sound *sound)
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

    void SyncPointManager::clear()
    {
        m_sound = nullptr;
        m_points.clear();
    }

    std::string_view SyncPointManager::getLabel(size_t i) const
    {
        return m_points.at(i).label();
    }

    unsigned int SyncPointManager::getOffsetSamples(size_t i) const
    {
        unsigned int offset;
        checkResult( m_sound->getSyncPointInfo(m_points.at(i).point(),
            nullptr, 0, &offset, FMOD_TIMEUNIT_PCM));
        return offset;
    }

    std::optional<unsigned int>
    SyncPointManager::getOffsetSamples(std::string_view label) const
    {
        size_t index = 0;
        for (auto &point : m_points)
        {
            if (point.label() == label)
                return getOffsetSamples(index);

            ++index;
        }

        return {};
    }

    unsigned int SyncPointManager::getOffsetMS(size_t i) const
    {
        unsigned int offset;
        checkResult( m_sound->getSyncPointInfo(m_points.at(i).point(),
            nullptr, 0, &offset, FMOD_TIMEUNIT_MS));
        return offset;
    }

    std::optional<unsigned int>
    SyncPointManager::getOffsetMS(std::string_view label) const
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

    double SyncPointManager::getOffsetSeconds(size_t i) const
    {
        return (double)getOffsetMS(i) * .001;
    }

    std::optional<double>
    SyncPointManager::getOffsetSeconds(std::string_view label) const
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

    size_t SyncPointManager::size() const
    {
        return m_points.size();
    }



}
