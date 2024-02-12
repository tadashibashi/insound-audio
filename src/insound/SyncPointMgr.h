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
    /**
     * Used to get and set syncpoints in an FMOD::Sound object
     */
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
        unsigned int getOffsetPCM(size_t i) const;
        [[nodiscard]]
        std::optional<unsigned int> getOffsetPCM(std::string_view label) const;
        [[nodiscard]]
        double getOffsetMS(size_t i) const;
        [[nodiscard]]
        std::optional<double> getOffsetMS(std::string_view label) const;
        [[nodiscard]]
        double getOffsetSeconds(size_t i) const;
        [[nodiscard]]
        std::optional<double> getOffsetSeconds(std::string_view label) const;

        void replace(size_t i, std::string_view label, unsigned offset, int fmodTimeUnit);

        void deleteSyncPoint(size_t i);

        /**
         * Find index of a sync point with the provided label
         * @param  label - label of syncpoint to find the index of
         * @return       index of label or -1 if not found
         */
        int findIndex(std::string_view label);

        /**
         * Emplace a new sync point to the manager
         * @param  label        name of the sync point
         * @param  offset       time offset
         * @param  fmodTimeUnit FMOD_TIMEUINT_ to use for time offset
         * @return              the new SyncPoint object reference.
         */
        SyncPoint &emplace(std::string_view label, unsigned int offset,
            int fmodTimeUnit);

        void swap(SyncPointMgr &other);
    private:
        [[nodiscard]]
        float getSampleRate() const;

        // Sync point data
        std::vector<SyncPoint> m_points;

        // Sound to check points off of, not owned by this SyncPointManager.
        FMOD::Sound *m_sound;
    };
}
