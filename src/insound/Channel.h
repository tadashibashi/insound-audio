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

        /**
         * Set the fade level from one to another over a period of time
         * @param  from    - origin fade level
         * @param  to      - destination fade level
         * @param  seconds - time to transition in seconds
         * @return reference to this object for chaining.
         */
        Channel &fade(float from, float to, float seconds);

        /**
         * Fade from current fade level to another over a period of time
         * @param  vol     - destination fade level
         * @param  seconds - time to transition in seconds
         * @return reference to this object for chaining.
         */
        Channel &fadeTo(float vol, float seconds);

        /**
         * Set whether the channel should loop
         * @param  setLooping - whether channel should loop
         * @return reference to this object for chaining.
         */
        Channel &looping(bool setLooping);

        /**
         * Set paused status
         * @param  set     - set value of pause
         * @param  seconds - number of seconds to fade in/out
         * @return reference to this channel for chaining.
         */
        Channel &pause(bool value, float seconds = 0);

        Channel &volume(float val);

        /**
         * Set the output channel group - only available if this is an
         * FMOD::Channel. Check that `isGroup` is `false`.
         *
         * @param  group Channel object to set
         * @return reference to this object for chaining.
         */
        Channel &ch_group(Channel &group);

        /**
         * Set the seek position of the Channel.
         *
         * Only available if underlying context is an FMOD::Channel. Throws if
         * this is not the case. Check that `isGroup` is `false`.
         *
         * @param seconds - time to seek to in seconds
         * @return reference to this object for chaining.
         */
        Channel &ch_position(float seconds);

        /**
         * Set the reverb send level.
         *
         * @param  level - reverb level to set
         * @return reference to this object for chaining.
         */
        Channel &reverbLevel(float level);



        /**
         * Get the current channel fade level
         */
        [[nodiscard]]
        float fadeLevel(bool final=true) const;

        /**
         * Get whether the channel is set to looping
         */
        [[nodiscard]]
        bool looping() const;

        /**
         * Get whether the channel is paused
         */
        [[nodiscard]]
        bool paused() const;

        /**
         * Get the channel volume level
         */
        [[nodiscard]]
        float volume() const;

        /**
         * Get the reverb send level.
         */
        [[nodiscard]]
        float reverbLevel() const;

        /**
         * Get whether this is a channel group (bus).
         * False means underlying object is an FMOD::Channel.
         */
        [[nodiscard]]
        bool isGroup() const { return m_isGroup; }

        /**
         * Get the raw FMOD::ChannelControl object
         */
        [[nodiscard]]
        FMOD::ChannelControl *raw() const { return chan; }

        /**
         * Get the user-assigned index value (not the FMOD index)
         */
        [[nodiscard]]
        int index() const { return m_index; }

        /**
         * Get the ChannelGroup object this outputs to
         * @return Gets the associated group object
         */
        [[nodiscard]]
        Channel *group() const;
        // Only available if underlying context is an FMOD::Channel. Thorws if
        // this is not the case.
        [[nodiscard]]
        float ch_position() const;

        /**
         * Release or cleanup internals
         */
        void release();
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
