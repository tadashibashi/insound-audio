#include "MultiTrackAudio.h"
#include "Channel.h"
#include "common.h"
#include <insound/Int24.h>
#include "insound/AudioEngine.h"
#include "SyncPointMgr.h"

#include <fmod.hpp>
#include <fmod_common.h>
#include <fmod_dsp.h>
#include <fmod_dsp_effects.h>
#include <fmod_errors.h>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <functional>
#include <limits>
#include <utility>
#include <vector>

namespace Insound
{
    static std::map<FMOD::Sound *, std::vector<float>> pcmData;

    struct MultiTrackAudio::Impl
    {
    public:
        Impl(FMOD::System *sys) :
            sounds(), chans(), fsb(),
            main(sys), points(), syncpointCallback(), endCallback()
        {

        }

        ~Impl()
        {
            chans.clear();
            main.release();

            if (fsb)
                fsb->release();

            // Any left-over sounds (covered in the MainTrackAudio destructor,
            // but left here for good measure)
            for (auto &sound : sounds)
            {
                sound->release();
            }
        }

        std::vector<FMOD::Sound *> sounds;
        std::vector<Channel> chans;
        FMOD::Sound *fsb;

        Channel main;
        SyncPointMgr points;
        std::function<void(const std::string &, double, int)> syncpointCallback;
        std::function<void()> endCallback;
    };


    MultiTrackAudio::MultiTrackAudio(FMOD::System *sys)
        : m(new Impl(sys))
    {

    }


    MultiTrackAudio::~MultiTrackAudio()
    {
        clear();
        delete m;
    }


    void MultiTrackAudio::fadeTo(float to, float seconds)
    {
        m->main.fadeTo(to, seconds);
    }


    void MultiTrackAudio::fadeChannelTo(int ch, float to, float seconds)
    {
        m->chans.at(ch).fadeTo(to, seconds);
    }


    float MultiTrackAudio::channelFadeLevel(int ch, bool final) const
    {
        return m->chans.at(ch).fadeLevel(final);
    }


    float MultiTrackAudio::fadeLevel(bool final) const
    {
        return m->main.fadeLevel(final);
    }


    void MultiTrackAudio::pause(bool value, float seconds)
    {
        for (auto &chan : m->chans)
        {
            chan.pause(value, seconds);
        }
    }


    bool MultiTrackAudio::paused() const
    {
        if (!isLoaded()) return false;
        
        return m->chans.at(0).paused();
    }


    void MultiTrackAudio::position(double seconds)
    {
        auto len = length();

        if (seconds >= len)
            seconds = len-.00001;

        const auto samplerate = this->samplerate();

        for (auto &chan : m->chans)
            chan.ch_positionSamples(seconds * samplerate);
    }


    double MultiTrackAudio::position() const
    {
        if (m->chans.empty()) return 0;

        // get first channel, assuming each is synced
        return m->chans[0].ch_positionSamples() / samplerate();
    }


    double MultiTrackAudio::length() const
    {
        auto numSounds = m->sounds.size();

        if (numSounds == 0) return 0;

        unsigned int maxLength = 0;
        for (int i = 0; i < numSounds; ++i)
        {
            const auto sound = m->sounds[i];

            unsigned int length;
            checkResult( sound->getLength(&length, FMOD_TIMEUNIT_PCM) );

            if (length > maxLength)
                maxLength = length;
        }

        return maxLength / samplerate();
    }


    float MultiTrackAudio::audibility() const
    {
        return m->main.audibility();
    }


    void MultiTrackAudio::clear()
    {
        m->chans.clear();
        m->points.clear();
        m->syncpointCallback =
            std::function<void(const std::string &, double, int)>{};

        // Free pcm data
        for (auto *sound : m->sounds)
        {
            pcmData.erase(sound);
        }

        // Release bank
        if (m->fsb)
        {
            m->fsb->release();
            m->fsb = nullptr;
        }
        else if (!m->sounds.empty()) // otherwise release individual sounds
        {
            for (auto *sound : m->sounds)
            {
                auto result = sound->release();
                if (result != FMOD_OK)
                {
                    std::cerr << "Error while releasing sound : "
                        << FMOD_ErrorString(result) << "\n";
                }
            }
        }

        m->sounds.clear();
    }


    bool MultiTrackAudio::isLoaded() const
    {
        return static_cast<bool>(!m->sounds.empty());
    }

    static FMOD_RESULT F_CALLBACK channelCallback(
        FMOD_CHANNELCONTROL *chanCtrl,
        FMOD_CHANNELCONTROL_TYPE chanType,
        FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType,
        void *commanddata1,
        void *commanddata2)
    {
        // Get `MultiTrackAudio` object
        auto chan = (FMOD::ChannelControl *)chanCtrl;
        MultiTrackAudio *track;

        auto result = chan->getUserData((void **)&track);
        if (result != FMOD_OK)
            return result;

        // Determine action based on callback type
        switch (callbackType)
        {
        case FMOD_CHANNELCONTROL_CALLBACK_SYNCPOINT:
            {
                auto pointIndex = (int)(uintptr_t)commanddata1;
                const auto &callback = track->getSyncPointCallback();

                // Invoke callback, if any was set
                if (callback)
                    callback(
                        track->getSyncPointLabel(pointIndex).data(),
                        track->getSyncPointOffsetSeconds(pointIndex),
                        pointIndex
                    );

                break;
            }

        case FMOD_CHANNELCONTROL_CALLBACK_END:
            {
                const auto &callback = track->getEndCallback();
                if (callback)
                    callback();

                break;
            }

        default: break;
        }

        return FMOD_OK;
    }

    size_t MultiTrackAudio::getSyncPointCount() const
    {
        return m->points.size();
    }

    bool MultiTrackAudio::getSyncPointsEmpty() const
    {
        return m->points.empty();
    }

    std::string_view MultiTrackAudio::getSyncPointLabel(size_t i) const
    {
        return m->points.getLabel(i);
    }

    double MultiTrackAudio::getSyncPointOffsetMS(size_t i) const
    {
        return m->points.getOffsetMS(i);
    }

    double MultiTrackAudio::getSyncPointOffsetSeconds(size_t i) const
    {
        return (double)m->points.getOffsetSeconds(i);
    }


    static FMOD_RESULT F_CALL pcmReadCallback(FMOD_SOUND *pSnd, void *data,
        unsigned int datalen)
    {
        // Get callback objects
        auto sound = (FMOD::Sound *)pSnd;

        // Get audio format info
        FMOD_SOUND_TYPE type;
        FMOD_SOUND_FORMAT format;
        int bits;
        checkResult(sound->getFormat(&type, &format, nullptr, &bits));

        std::vector<float> res;

        // parse sample data based on format data
        switch(bits)
        {
        case 8:
            {
                res.reserve(datalen);
                for (size_t i = 0; i < datalen; ++i)
                {
                    auto val = *((uint8_t *)data + i);
                    res.push_back( (double)val / std::numeric_limits<uint8_t>::max() * 2.0 - 1.0);
                }
                break;
            }

        case 16:
            {
                auto len = datalen / 2;
                res.reserve(len);
                for (size_t i = 0; i < len; ++i)
                {
                    auto val = *((int16_t *)data + i);
                    res.push_back( (float)val / (std::numeric_limits<int16_t>::max()) );
                }
                break;
            }
        case 24:
            {
                auto len = datalen / 3;
                res.reserve(len);
                for (size_t i = 0; i < len; ++i)
                {
                    auto val = ((Int24 *)data + i)->value;
                    res.push_back( (float)val / 8'388'607.0 );
                }
                break;
            }
        case 32:
            {
                if (format == FMOD_SOUND_FORMAT_PCMFLOAT)
                {
                    auto len = datalen / sizeof(float);
                    res.reserve(len);
                    for (size_t i = 0; i < len; ++i)
                    {
                        auto val = *((float *)data + i);
                        res.push_back((double)val / std::numeric_limits<float>::max());
                    }
                }
                else
                {
                    auto len = datalen / 4;
                    res.reserve(len);
                    for (size_t i = 0; i < len; ++i)
                    {
                        auto val = *((int32_t *)data + i);
                        res.push_back( (double)val / std::numeric_limits<int32_t>::max());
                    }
                }

                break;
            }
        default:
            return FMOD_ERR_FORMAT;
        }

        // push to track pcm data
        pcmData.emplace(sound, res);
        return FMOD_OK;
    }

    uintptr_t MultiTrackAudio::loadSound(const char *data, size_t bytelength)
    {
        try {
            FMOD::System *sys;
            checkResult( m->main.raw()->getSystemObject(&sys) );

            // Set relevant info to load the fsb
            auto exinfo{FMOD_CREATESOUNDEXINFO()};
            std::memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
            exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
            exinfo.length = bytelength;
            exinfo.pcmreadcallback = pcmReadCallback;

            // add sound to the existing sounds and set its position accordingly
            FMOD::Sound *sound;
            checkResult( sys->createSound(data,
                FMOD_OPENMEMORY | FMOD_LOOP_NORMAL | FMOD_ACCURATETIME | FMOD_CREATESAMPLE,
                &exinfo,
                &sound)
            );

            SyncPointMgr points(sound);

            // Check if this is to be the first sound
            if (m->fsb || m->sounds.empty()) // fsb means it will be later unloaded, otherwise, checks for empty sounds
            {
                // set loop info from markers
                auto loopStart = points.getOffsetPCM("LoopStart");
                auto loopEnd = points.getOffsetPCM("LoopEnd");
                bool didAlterLoop = false;

                if (!loopStart)
                {
                    loopStart.emplace(0);
                    sound->addSyncPoint(0, FMOD_TIMEUNIT_PCM, "LoopStart",
                        nullptr);
                    didAlterLoop = true;
                }

                if (!loopEnd)
                {
                    unsigned int length;
                    checkResult(sound->getLength(&length, FMOD_TIMEUNIT_PCM));

                    loopEnd.emplace(length);
                    sound->addSyncPoint(length, FMOD_TIMEUNIT_PCM, "LoopEnd",
                        nullptr);

                    didAlterLoop = true;
                }

                checkResult( sound->setLoopPoints(
                    loopStart.value(), FMOD_TIMEUNIT_PCM,
                    loopEnd.value(), FMOD_TIMEUNIT_PCM) );

                if (didAlterLoop)
                    points.load(sound);
            }
            else
            {
                // set loop position from other sounds
                unsigned int loopstart, loopend;
                checkResult( m->sounds[0]->getLoopPoints(
                    &loopstart, FMOD_TIMEUNIT_PCM,
                    &loopend, FMOD_TIMEUNIT_PCM) );

                checkResult( sound->setLoopPoints(
                    loopstart, FMOD_TIMEUNIT_PCM,
                    loopend, FMOD_TIMEUNIT_PCM) );
            }

            // done, commit results
            if (m->fsb)
            {
                clear();
            }

            auto chanSize = m->chans.size();

            auto &chan = m->chans.emplace_back(sound, (FMOD::ChannelGroup *)m->main.raw(),
                sys);

            if (chanSize == 0)
            {
                m->points.swap(points); // only set points of first track

                checkResult( chan.raw()->setUserData(this));
                checkResult( chan.raw()->setCallback(channelCallback) );
            }
            else
            {
                // set channel track position to others' position
                auto seconds = m->chans[0].ch_position();
                chan.ch_position(seconds);
            }

            m->sounds.emplace_back(sound);

            pause(true, 0); // pause, wait for user to trigger start

            return (uintptr_t)sound;
        }
        catch(...)
        {
            // clear other sounds since it's considered a "failed bank"
            this->clear();
            throw;
        }
    }

    void MultiTrackAudio::loadFsb(const char *data, size_t bytelength)
    {
        // Set relevant info to load the fsb
        auto exinfo{FMOD_CREATESOUNDEXINFO()};
        std::memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
        exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        exinfo.length = bytelength;
        exinfo.pcmreadcallback = pcmReadCallback;

        // Load the sound bank via system object
        FMOD::System *sys;
        checkResult( m->main.raw()->getSystemObject(&sys) );

        FMOD::Sound *snd;
        checkResult( sys->createSound(data,
            FMOD_OPENMEMORY_POINT | FMOD_LOOP_NORMAL |
                FMOD_CREATESAMPLE | FMOD_NONBLOCKING,
            &exinfo, &snd)
        );

        // Ensure there is at least one sound in the bank
        int numSubSounds;
        checkResult( snd->getNumSubSounds(&numSubSounds) );
        if (numSubSounds == 0)
            throw std::runtime_error("No subsounds in the fsbank file.");

        // Populate sync point container with first subsound
        FMOD::Sound *firstSound;
        checkResult( snd->getSubSound(0, &firstSound) );

        SyncPointMgr syncPoints(firstSound);

        unsigned int length;
        checkResult( firstSound->getLength(&length, FMOD_TIMEUNIT_PCM) );
        if (length == 0)
            throw std::runtime_error("Invalid subsound, 0 length.");

        // Find loop start / end points if they exist
        auto loopstart = syncPoints.getOffsetPCM("LoopStart");
        auto loopend = syncPoints.getOffsetPCM("LoopEnd");

        // If loop start or loop end were not found, add it automatically
        bool didSetLoop = false;
        if (!loopstart)
        {
            checkResult( firstSound->addSyncPoint(0, FMOD_TIMEUNIT_PCM,
                "LoopStart", nullptr) );
            loopstart.emplace(0);
            didSetLoop = true;
        }

        if (!loopend)
        {
            checkResult( firstSound->addSyncPoint(length,
                FMOD_TIMEUNIT_PCM, "LoopEnd", nullptr) );
            loopend.emplace(length);
            didSetLoop = true;
        }

        // Update sync points manager if any were added
        if (didSetLoop)
            syncPoints = SyncPointMgr(firstSound);

        // Validate loop points
        if (loopend.value() < loopstart.value())
            throw std::runtime_error("LoopStart comes after LoopEnd.");

        // Set loop points on each sound, emplacing them into a Channel vector
        std::vector<Channel> chans;
        std::vector<FMOD::Sound *> sounds;
        for (int i = 0; i < numSubSounds; ++i)
        {
            FMOD::Sound *subsound;
            checkResult( snd->getSubSound(i, &subsound) );

            // set loop points
            checkResult(
                subsound->setLoopPoints(
                    loopstart.value(), FMOD_TIMEUNIT_PCM,
                    loopend.value(), FMOD_TIMEUNIT_PCM)
            );

            // create the channel wrapper object from the subsound
            chans.emplace_back(subsound,
                (FMOD::ChannelGroup *)m->main.raw(), sys);
            sounds.emplace_back(subsound);
        }

        // Set channel callback on first channel
        if (!chans.empty())
        {
            checkResult( chans[0].raw()->setUserData(this));
            checkResult( chans[0].raw()->setCallback(channelCallback) );
        }

        // Success, clear any prior internals then commit changes
        clear();
        m->chans.swap(chans);
        m->fsb = snd;
        m->sounds.swap(sounds);
        std::swap(m->points, syncPoints);

        pause(true, 0); // pause, wait for user to trigger start
    }


    void MultiTrackAudio::mainVolume(double vol)
    {
        m->main.volume(vol);
    }


    double MultiTrackAudio::mainVolume() const
    {
        return m->main.volume();
    }


    void MultiTrackAudio::channelVolume(int ch, double vol)
    {
        m->chans.at(ch).volume(vol);
    }


    double MultiTrackAudio::channelVolume(int ch) const
    {
        return m->chans.at(ch).volume();
    }


    int MultiTrackAudio::channelCount() const
    {
        return m->chans.size();
    }


    void MultiTrackAudio::setSyncPointCallback(
        std::function<void(const std::string &, double, int)> callback)
    {
        m->syncpointCallback = std::move(callback);
    }


    const std::function<void(const std::string &, double, int)> &
    MultiTrackAudio::getSyncPointCallback() const
    {
        return m->syncpointCallback;
    }


    bool MultiTrackAudio::addSyncPointMS(const std::string &name,
        double offset)
    {
        try {
            m->points.emplace(name.data(), offset * .001 * samplerate(),
                FMOD_TIMEUNIT_PCM);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }


    bool MultiTrackAudio::editSyncPointMS(size_t i, const std::string &name,
        double offset)
    {
        try {
            m->points.replace(i, name, offset * .001 * samplerate(),
                FMOD_TIMEUNIT_PCM);
            return true;
        }
        catch(...)
        {
            return false;
        }
    }


    bool MultiTrackAudio::deleteSyncPoint(size_t i)
    {
        try {
            m->points.deleteSyncPoint(i);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }


    void MultiTrackAudio::setEndCallback(
        std::function<void()> callback)
    {
        m->endCallback = std::move(callback);
    }


    const std::function<void()> &
    MultiTrackAudio::getEndCallback() const
    {
        return m->endCallback;
    }


    void MultiTrackAudio::channelReverbLevel(int ch, float level)
    {
        m->chans.at(ch).reverbLevel(level);
    }

    float MultiTrackAudio::channelReverbLevel(int ch) const
    {
        return m->chans.at(ch).reverbLevel();
    }

    void MultiTrackAudio::mainReverbLevel(float level)
    {
        m->main.reverbLevel(level);
    }

    float MultiTrackAudio::mainReverbLevel() const
    {
        return m->main.reverbLevel();
    }

    void MultiTrackAudio::loopMilliseconds(double loopstart, double loopend)
    {
        loopSeconds(loopstart * .001, loopend * .001);
    }

    void MultiTrackAudio::loopSeconds(double loopstart, double loopend)
    {
        if (m->sounds.empty()) return;

        float samplerate;
        checkResult(m->sounds[0]->getDefaults(&samplerate, nullptr));

        unsigned startpcm = samplerate * loopstart;
        unsigned endpcm = samplerate * loopend;

        loopSamples(startpcm, endpcm);
    }

    void MultiTrackAudio::loopSamples(unsigned loopstart, unsigned loopend)
    {
        if (m->sounds.empty()) return;

        // Clamp loop points
        unsigned lengthpcm;
        checkResult(m->sounds[0]->getLength(&lengthpcm, FMOD_TIMEUNIT_PCM));

        if (loopend >= lengthpcm)
            loopend = lengthpcm - 1;
        if (loopend == 0)
            loopend = 1;
        if (loopstart > loopend)
            loopstart = loopend - 1;

        // Set points
        for (auto &ch : m->chans)
        {
            ch.ch_loopPCM(loopstart, loopend);
        }
    }

    LoopInfo<double> MultiTrackAudio::loopMilliseconds() const
    {
        const auto loop =  m->chans.at(0).ch_loopPCM();
        const double rate = samplerate();

        return {
            .start=loop.start / rate * 1000.0,
            .end=loop.end / rate * 1000.0,
        };
    }

    LoopInfo<double> MultiTrackAudio::loopSeconds() const
    {
        const auto loop =  m->chans.at(0).ch_loopPCM();
        const double rate = samplerate();

        return {
            .start=loop.start / rate,
            .end=loop.end / rate,
        };
    }

    LoopInfo<unsigned> MultiTrackAudio::loopSamples() const
    {
        return m->chans.at(0).ch_loopPCM();
    }

    Channel &MultiTrackAudio::channel(int ch)
    {
        return m->chans.at(ch);
    }

    const Channel &MultiTrackAudio::channel(int ch) const
    {
        return m->chans.at(ch);
    }

    Channel &MultiTrackAudio::main()
    {
        return m->main;
    }

    const Channel &MultiTrackAudio::main() const
    {
        return m->main;
    }

    const std::vector<float> &MultiTrackAudio::getSampleData(size_t index) const
    {
        auto sound = m->sounds.at(index);

        auto it = pcmData.find(sound);
        if (it == pcmData.end())
        {
            throw std::runtime_error("Sound does not exist in pcmData list, "
                "cannot get its sample data");
        }

        return it->second;
    }

    float MultiTrackAudio::samplerate() const
    {
        float freq;
        checkResult(m->sounds.at(0)->getDefaults(&freq, nullptr));
        return freq;
    }
}
