#pragma once

// Forward declarations
#include "insound/LoopInfo.h"
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
         * Set paused status
         * @param  set     - set value of pause
         * @param  seconds - number of seconds to fade in/out
         * @return reference to this channel for chaining.
         */
        Channel &pause(bool value, float seconds = 0);

        Channel &volume(float val);

        Channel &panLeft(float value);

        /**
         * Get the left pan value
         *
         * @return value: 1 = fully left, 0 = fully right, .5 = middle
         */
        [[nodiscard]]
        float panLeft() const;

        /**
         * Set right pan value.
         * @param value: 1 = fully right, 0 = fully left, .5 = middle
         *
         * @return reference to this channel for chaining.
         */
        Channel &panRight(float value);

        /**
         * Get the right pan value.
         *
         * @return value: 1 = fully right, 0 = fully left, .5 = middle
         */
        [[nodiscard]]
        float panRight() const;

        /**
         * Set both pan values at once
         * @param  left  : 1 = fully left, 0 = fully right, .5 = middle
         * @param  right : 1 = fully right, 0 = fully left, .5 = middle
         *
         * @return reference to this channel for chaining.
         */
        Channel &pan(float left, float right);

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

        Channel &ch_loopSeconds(double loopstart, double loopend);
        Channel &ch_loopSamples(unsigned loopstart, unsigned loopend);

        [[nodiscard]]
        LoopInfo<double> ch_loopSeconds() const;
        [[nodiscard]]
        LoopInfo<unsigned> ch_loopSamples() const;

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

        float m_leftPan;
        float m_rightPan;
    };
}
