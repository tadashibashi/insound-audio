#pragma once
#include <cstddef>
#include <string_view>

struct FMOD_SYNCPOINT;
namespace FMOD
{
    class Sound;
}

namespace Insound
{
    const int SyncPointNameLength = 256;

    enum class TimeUnit : int
    {
        Samples,
        Milliseconds,
    };

    class SyncPointView
    {
    public:
        // ----- constructors -------------------------------------------------

        SyncPointView();
        explicit SyncPointView(FMOD::Sound *snd);
        SyncPointView(FMOD::Sound *snd, size_t index);

        // ----- copying ------------------------------------------------------

        SyncPointView(SyncPointView &other);
        SyncPointView &operator=(SyncPointView &other);

        bool load(FMOD::Sound *snd);

        /**
         * Get the number of syncpoints in the loaded sound.
         */
        [[nodiscard]]
        size_t size() const;

        /**
         * Check this before initial query to see if there are any sync points
         */
        [[nodiscard]]
        bool empty() const;

        /**
         * Get the current index of the syncpoint this object is viewing.
         */
        [[nodiscard]]
        int index() const { return m_index; }

        [[nodiscard]]
        std::string_view label() const;

        void index(size_t index);
        void index(std::string_view label);


        void emplace(std::string_view name, unsigned offset,
            TimeUnit timeUnit);


        [[nodiscard]]
        unsigned int offset(TimeUnit unit = TimeUnit::Samples) const;

        SyncPointView &operator++();
        SyncPointView operator++(int);
    private:
        mutable char m_label[SyncPointNameLength];

        FMOD_SYNCPOINT *m_pnt;
        FMOD::Sound *m_snd;

        int m_index;
    };
}

