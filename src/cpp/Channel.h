#pragma once

// Forward declarations
namespace FMOD
{
    class Sound;
    class System;
    class ChannelControl;
    class ChannelGroup;
}

#include <string>
#include <string_view>
#include <vector>

namespace Insound
{
    /**
     * Wrapper around an FMOD::ChannelControl object
     */
    class Channel
    {
    public:
        Channel(FMOD::Sound *sound, FMOD::ChannelGroup *group, FMOD::System *system, int index = -1);
        Channel(std::string_view name, FMOD::System *system, int index = -1);

        Channel(Channel &other) = delete;
        Channel &operator=(Channel &other) = delete;

        Channel(Channel &&other);
        Channel &operator=(Channel &&other);


        ~Channel();

        Channel &fade(float from, float to, float seconds);
        Channel &fadeTo(float vol, float seconds);
        Channel &looping(bool set);

        /**
         * Set paused status
         * @param  set     - set value of pause
         * @param  seconds - number of seconds to fade in/out
         * @return reference to this channel for chaining.
         */
        Channel &paused(bool value, float seconds = 0);
        // Only available if underlying context is an FMOD::Channel. Throws if
        // this is not the case.
        Channel &position(float seconds);
        Channel &volume(float val);


        [[nodiscard]]
        float fadeLevel(bool final=true) const;

        [[nodiscard]]
        bool looping() const;

        // Only available if underlying context is an FMOD::Channel. Thorws if
        // this is not the case.
        [[nodiscard]]
        float position() const;

        [[nodiscard]]
        bool paused() const;

        [[nodiscard]]
        float volume() const;

        [[nodiscard]]
        bool isGroup() const { return m_isGroup; }

        FMOD::ChannelControl *raw() const { return chan; }

        int index() const { return m_index; }
    private:
        FMOD::ChannelControl *chan;
        float lastFadePoint;
        int samplerate;
        bool m_isGroup;
        bool m_isPaused;

        // channel number in a track set
        int m_index;

    };
}
