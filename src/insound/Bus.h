#pragma once

// forward declaration
namespace FMOD
{
    class ChannelGroup;
    class System;
}

namespace Insound::Audio
{
    class System;

    /**
     * Audio bus for multiple channels to output to
     * Wraps an FMOD::ChannelGroup
     */
    class Bus
    {
    public:
        // Bus should be referenced only
        Bus() = delete;
        Bus(const Bus &) = delete;
        Bus(Bus &&) = delete;
        Bus &operator=(const Bus &) = delete;
        Bus &operator=(Bus &&) = delete;

        /**
         * Attach another bus to input into this one
         */
        void addInput(Bus *bus);

        /**
         * Set this bus's output
         * @param bus - bus to set as the output to this one
         */
        void setOutput(Bus *bus);

        [[nodiscard]]
        unsigned long long clock() const;

        [[nodiscard]]
        unsigned long long parentClock() const;

        [[nodiscard]]
        float volume() const;
        [[nodiscard]]
        float panLeft() const { return m_panLeft; }
        [[nodiscard]]
        float panRight() const { return m_panRight; }

        void volume(float level);
        void panLeft(float level);
        void panRight(float level);

        /**
         * Whether this bus represents the master output bus
         */
        [[nodiscard]]
        bool isMaster() const;

        /**
         * Handle to the raw underlying FMOD object
         */
        const FMOD::ChannelGroup *handle() { return m_group; }

    private:
        friend class System;
        /**
         * Create a Bus with a new ChannelGroup
         */
        explicit Bus(System *sys);

        /**
         * Create a Bus with an existing ChannelGroup
         */
        explicit Bus(FMOD::ChannelGroup *group);

        ~Bus();

        FMOD::ChannelGroup *m_group;
        float m_panLeft;
        float m_panRight;
    };
}
