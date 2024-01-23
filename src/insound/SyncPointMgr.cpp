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
            checkResult( sound->getSyncPointInfo(fPoint, buffer, 255,
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

    unsigned int SyncPointMgr::getOffsetPCM(size_t i) const
    {
        unsigned int offset;
        checkResult( m_sound->getSyncPointInfo(m_points.at(i).point(),
            nullptr, 0, &offset, FMOD_TIMEUNIT_PCM));
        return offset;
    }

    std::optional<unsigned int>
    SyncPointMgr::getOffsetPCM(std::string_view label) const
    {
        size_t index = 0;
        for (auto &point : m_points)
        {
            if (compareCaseInsensitive(point.label(), label))
                return getOffsetPCM(index);

            ++index;
        }

        return {};
    }

    double SyncPointMgr::getOffsetMS(size_t i) const
    {
        return getOffsetSeconds(i) * 1000.0;
    }

    std::optional<double>
    SyncPointMgr::getOffsetMS(std::string_view label) const
    {
        // find index of point with label
        size_t index = 0;
        for (auto &point : m_points)
        {
            if (point.label() == label)
                return getOffsetMS(index);

            ++index;
        }

        // label could not be found
        return {};
    }

    double SyncPointMgr::getOffsetSeconds(size_t i) const
    {
        return (double)getOffsetPCM(i) / (double)getSampleRate();
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

    bool SyncPointMgr::empty() const
    {
        return m_points.empty();
    }

    int SyncPointMgr::findIndex(std::string_view label)
    {
        int i = 0;
        for (auto &point : m_points)
        {
            if (point.label() == label)
            {
                return i;
            }

            ++i;
        }

        return -1;
    }

    void SyncPointMgr::replace(size_t i, std::string_view label, unsigned offset, int fmodTimeUnit)
    {
        checkResult(m_sound->deleteSyncPoint(m_points[i].point()));
        m_points.erase(m_points.begin() + i);

        emplace(label, offset, fmodTimeUnit);
    }

    void SyncPointMgr::deleteSyncPoint(size_t i)
    {
        checkResult(m_sound->deleteSyncPoint(m_points.at(i).point()));
        m_points.erase(m_points.begin() + i);
    }

    SyncPoint &SyncPointMgr::emplace(std::string_view label,
        unsigned int offset, int unit)
    {
        FMOD_SYNCPOINT *point = nullptr;
        for (size_t i = 0, size=m_points.size(); i < size; ++i)
        {
            unsigned int checkOffset;
            checkResult(m_sound->getSyncPointInfo(m_points[i].point(), nullptr,
                0, &checkOffset, unit));

            // Don't add duplicates, just return the point found
            // This functionality covers case where user adds multiple sounds
            // with the same syncpoint marker info
            if (checkOffset == offset && m_points[i].label() == label)
                return m_points[i];

            if (checkOffset > offset)
            {
                checkResult(m_sound->addSyncPoint(offset, unit, label.data(),
                    &point));
                return *m_points.insert(m_points.begin() + i,
                    SyncPoint{label, point});
            }
        }

        checkResult(m_sound->addSyncPoint(offset, unit, label.data(), &point));
        return m_points.emplace_back(label, point);
    }

    void SyncPointMgr::swap(SyncPointMgr &other)
    {
        m_points.swap(other.m_points);

        // swap sound objects
        auto tempSound = other.m_sound;
        other.m_sound = m_sound;
        m_sound = tempSound;
    }

    float SyncPointMgr::getSampleRate() const
    {
        float samplerate;
        checkResult(m_sound->getDefaults(&samplerate, nullptr));

        return samplerate;
    }
}
