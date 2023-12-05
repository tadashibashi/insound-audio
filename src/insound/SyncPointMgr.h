#pragma once

#include <optional>
#include <map>
#include <string>
#include <string_view>
#include <vector>

struct FMOD_SYNCPOINT;
namespace FMOD
{
    class Sound;
}

namespace Insound
{


    struct SyncPoint
    {
    public:
        SyncPoint(std::string_view label, FMOD_SYNCPOINT *point) :
            m_label(label), m_point(point) { }

        const std::string &label() const { return m_label; }
        FMOD_SYNCPOINT *point() const { return m_point; }
    private:
        std::string m_label;
        FMOD_SYNCPOINT *m_point;
    };

    class SyncPointMgr
    {
    public:
        SyncPointMgr();
        explicit SyncPointMgr(FMOD::Sound *sound);

        [[nodiscard]]
        const std::string &getLabel(size_t i) const;

        void load(FMOD::Sound *sound);

        /**
         * Clear internals
         */
        void clear();

        /**
         * Get the number of syncpoints
         */
        [[nodiscard]]
        size_t size() const;

        [[nodiscard]]
        bool empty() const;

        [[nodiscard]]
        unsigned int getOffsetSamples(size_t i) const;
        [[nodiscard]]
        std::optional<unsigned int> getOffsetSamples(std::string_view label) const;
        [[nodiscard]]
        unsigned int getOffsetMS(size_t i) const;
        [[nodiscard]]
        std::optional<unsigned int> getOffsetMS(std::string_view label) const;
        [[nodiscard]]
        double getOffsetSeconds(size_t i) const;
        [[nodiscard]]
        std::optional<double> getOffsetSeconds(std::string_view label) const;

        /**
         * Emplace a new sync point to the manager
         * @param  label        name of the sync point
         * @param  offset       time offset
         * @param  fmodTimeUnit FMOD_TIMEUINT_ to use for time offset
         * @return              the new SyncPoint object reference.
         */
        SyncPoint &emplace(std::string_view label, unsigned int offset,
            int fmodTimeUnit);
    private:
        // Sync point data
        std::vector<SyncPoint> m_points;

        // Sound to check points off of, not owned by this SyncPointManager.
        FMOD::Sound *m_sound;
    };
}
