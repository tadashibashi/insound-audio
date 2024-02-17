#pragma once

namespace Insound::Audio
{
    class TempoTrack
    {

    };

    class SignatureTrack
    {

    };

    /**
     * One stereo audio track
     */
    class AudioTrack
    {

    };

    /**
     * Describes a linear sequence of events in an entire session
     */
    class Timeline
    {
    public:
        [[nodiscard]]
        const TempoTrack *tempos() const { return &m_tempos; }
        [[nodiscard]]
        TempoTrack *tempos() { return &m_tempos; }

        [[nodiscard]]
        const SignatureTrack *signatures() const { return &m_signatures; }
        [[nodiscard]]
        SignatureTrack *signatures() { return &m_signatures; };

    private:
        TempoTrack m_tempos;
        SignatureTrack m_signatures;
    };

    /**
     * Describes an event that happens on a timeline
     */
    class AudioEvent
    {
    public:
        unsigned long long position() const;

    };

    /**
     * Equivalent of a DAW track lane
     */
    class TrackDescription
    {
        // various commands

    };

    class EventInstance
    {
    public:
        EventInstance();
        ~EventInstance();

        void load();

    };
}
