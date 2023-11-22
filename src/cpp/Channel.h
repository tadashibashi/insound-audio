#pragma once

// Forward declaration
namespace FMOD
{
    class Sound;
    class System;
    class ChannelControl;
    class ChannelGroup;
}

namespace Insound
{
    /**
     * Wrapper around an FMOD::ChannelControl object
     */
    class Channel
    {
    public:
        Channel(FMOD::Sound *sound, FMOD::ChannelGroup *group, FMOD::System *system);
        explicit Channel(FMOD::ChannelGroup *chan);

        ~Channel();

        Channel &fade(float from, float to, float seconds);
        Channel &fadeTo(float vol, float seconds);
        Channel &looping(bool set);
        Channel &paused(bool set);
        // Only available if underlying context is an FMOD::Channel. Throws if
        // this is not the case.
        Channel &position(float seconds);
        Channel &volume(float val);


        [[nodiscard]]
        float fadeLevel() const;

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

        FMOD::ChannelControl *raw() const { return chan; }
    private:
        FMOD::ChannelControl *chan;
        float lastFadePoint;
        int samplerate;
        bool isGroup;
    };
}
