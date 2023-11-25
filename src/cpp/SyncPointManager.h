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

        std::string_view label() const { return m_label.data(); }
        FMOD_SYNCPOINT *point() const { return m_point; }
    private:
        std::string m_label;
        FMOD_SYNCPOINT *m_point;
    };

    class SyncPointManager
    {
    public:
        SyncPointManager();
        explicit SyncPointManager(FMOD::Sound *sound);

        [[nodiscard]]
        std::string_view getLabel(size_t i) const;

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
    private:
        // Sync point data
        std::vector<SyncPoint> m_points;

        // Sound to check points off of, not owned by this SyncPointManager.
        FMOD::Sound *m_sound;
    };
}
