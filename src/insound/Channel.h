#pragma once

// Forward declarations
#include <insound/LoopInfo.h>
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
        // Create channel from sound
        Channel(FMOD::Sound *sound, FMOD::ChannelGroup *group, FMOD::System *system);
        // Create channel group
        explicit Channel(FMOD::System *system);
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
         * @param clock    - when to schedule fade in parent DSP clock units.
         *                   0 is null, which will use the current clock
         * @return reference to this object for chaining.
         */
        Channel &fade(float from, float to, float seconds, unsigned long long clock = 0);

        /**
         * Fade from current fade level to another over a period of time
         * @param  vol     - destination fade level
         * @param  seconds - time to transition in seconds
         * @param clock    - when to schedule fade, in parent DSP clock units
         * @return reference to this object for chaining.
         */
        Channel &fadeTo(float vol, float seconds, unsigned long long clock = 0);

        /**
         * Set paused status
         * @param  set        - set value of pause
         * @param  seconds    - if performFade is true: number of seconds to fade in/out
         *                      if performFade is false: number of seconds to delay pause set
         * @param performFade - whether to fade pause by `seconds`, or if false,
         *                    delays pause set by `seconds`
         * @param clock       - DSP clock of parent ChannelGroup, when to
         *                      schedule the pause. 0 is null, which will use
         *                      current clock.
         * @return reference to this channel for chaining.
         */
        Channel &pause(bool value, float seconds = 0, bool performFade = true, unsigned long long clock=0);

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
        Channel &ch_positionSamples(unsigned int samples);

        /**
         * Get the Channel position in samples.
         * If there is no underlying FMOD::Channel, (e.g. if it's an
         * FMOD::ChannelGroup) this function will throw.
         */
        [[nodiscard]]
        unsigned int ch_positionSamples() const;

        Channel &ch_loopMS(unsigned loopstart, unsigned loopend);
        Channel &ch_loopPCM(unsigned loopstart, unsigned loopend);

        [[nodiscard]]
        LoopInfo<unsigned> ch_loopMS() const;
        [[nodiscard]]
        LoopInfo<unsigned> ch_loopPCM() const;

        /**
         * Set the reverb send level.
         *
         * @param  level - reverb level to set
         * @return reference to this object for chaining.
         */
        Channel &reverbLevel(float level);



        /**
         * Get the current channel fade level
         * @param final - whether to show calculated result (when true), or last set value (when false)
         * @param targetClock - the clock point at which to check, if 0, it uses the current clock
         */
        [[nodiscard]]
        float fadeLevel(bool final=true, unsigned long long targetClock=0) const;

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

        /**
         * Get the current channel track position in seconds. If this Channel
         * does not contain an underlying FMOD::Channel, it will throw
         */
        [[nodiscard]]
        float ch_position() const;

        /**
         * Release or cleanup internals
         */
        void release();

        [[nodiscard]]
        bool isMaster() const { return m_isMaster; }

        [[nodiscard]]
        float audibility() const;
    private:
        FMOD::ChannelControl *chan;
        float lastFadePoint;
        int samplerate;
        bool m_isGroup;
        bool m_isPaused;
        bool m_isMaster;

        // channel number in a track set
        int m_index;

        float m_leftPan;
        float m_rightPan;
    };
}
