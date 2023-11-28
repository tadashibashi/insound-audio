#pragma once

// Forward declarations
namespace FMOD
{
    class ChannelControl;
    class ChannelGroup;
    class Channel;
    class Sound;
    class System;
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
        // Create from channel group
        explicit Channel(FMOD::ChannelGroup *group);

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

        Channel &volume(float val);

        /**
         * Set the output channel group - only available if this is an
         * FMOD::Channel
         *
         * @param  group Channel object to set
         * @return reference to this object for chaining.
         */
        Channel &ch_group(Channel &group);

        // Only available if underlying context is an FMOD::Channel. Throws if
        // this is not the case.
        Channel &ch_position(float seconds);



        [[nodiscard]]
        float fadeLevel(bool final=true) const;

        [[nodiscard]]
        bool looping() const;

        [[nodiscard]]
        bool paused() const;

        [[nodiscard]]
        float volume() const;

        [[nodiscard]]
        bool isGroup() const { return m_isGroup; }

        FMOD::ChannelControl *raw() const { return chan; }

        int index() const { return m_index; }

        /**
         * Get the ChannelGroup object this outputs to
         * @return Gets the associated group object
         */
        Channel *group() const;
        // Only available if underlying context is an FMOD::Channel. Thorws if
        // this is not the case.
        [[nodiscard]]
        float ch_position() const;
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
