#pragma once



namespace FMOD
{
    class System;
}

namespace Insound::Audio
{
    class Bus;

    class System
    {
    public:
        System();
        ~System();

        // Ideally this should not be depended upon
        [[nodiscard]]
        FMOD::System *handle() { return m_sys; }
        [[nodiscard]]
        const FMOD::System *handle() const { return m_sys; }

        bool init();

        /**
         * Closes the System object, making all further calls to System invalid
         * until the next call to `init` occurs.
         */
        void close();

        /**
         * Resumes system mixing and access to audio hardware
         */
        void resume();

        /**
         * Suspend system mixing and access to audio hardware while maintaining
         * internal state.
         */
        void suspend();

        void update();

        /**
         * Create a new bus
         * @param output - bus to output to
         *
         */
        Bus *createBus(Bus *output);
        bool deleteBus(Bus *bus);

        /**
         * Get the system master bus
         */
        [[nodiscard]]
        Bus *masterBus() { return m_master; }

        [[nodiscard]]
        const Bus *masterBus() const { return m_master; }

    private:
        FMOD::System *m_sys;
        Bus *m_master;
    };
}
