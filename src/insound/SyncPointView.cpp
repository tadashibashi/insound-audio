#include "SyncPointView.h"

#include <insound/common.h>

#include <fmod.hpp>
#include <fmod_common.h>

#include <iostream>
#include <stdexcept>

namespace Insound
{
    SyncPointView::SyncPointView()
        : m_label(), m_pnt(), m_snd(), m_index(-1) { }

    SyncPointView::SyncPointView(FMOD::Sound *snd)
        : m_label(), m_pnt(), m_snd(snd), m_index()
    {
        load(snd);
    }

    SyncPointView::SyncPointView(FMOD::Sound *snd, size_t index)
        : m_label(), m_pnt(), m_snd(snd), m_index()
    {
        this->index(index);
    }

    SyncPointView::SyncPointView(SyncPointView &other) : m_label(),
        m_index(other.m_index), m_pnt(other.m_pnt), m_snd(other.m_snd)
    {
        std::strcpy(m_label, other.m_label);
    }

    SyncPointView &SyncPointView::operator=(SyncPointView &other)
    {
        m_index = other.m_index;
        m_pnt = other.m_pnt;
        m_snd = other.m_snd;
        std::strcpy(m_label, other.m_label);

        return *this;
    }

    bool SyncPointView::load(FMOD::Sound *snd)
    {
        try {
            int numsyncpoints;
            checkResult(snd->getNumSyncPoints(&numsyncpoints));
            if (numsyncpoints == 0)
                m_index = -1;
            else
                index(0);

            return true;
        }
        catch(const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            return false;
        }
    }

    size_t SyncPointView::size() const
    {
        int numSyncPoints;
        checkResult(m_snd->getNumSyncPoints(&numSyncPoints));

        return (size_t)numSyncPoints;
    }

    bool SyncPointView::empty() const
    {
        return size() == 0;
    }

    void SyncPointView::index(size_t index)
    {
        if (index >= size())
        {
            std::out_of_range("SyncPointView index is out of range");
        }

        if (m_index == index)
            return;

        FMOD_SYNCPOINT *pnt;
        checkResult(m_snd->getSyncPoint(index, &pnt));
        m_label[0] = 0;
        m_pnt = pnt;
        m_index = index;
    }

    std::string_view SyncPointView::label() const
    {
        if (m_index < 0)
            std::runtime_error("SyncPointView is either empty or not loaded.");

        if (m_label[0] == 0)
        {
            checkResult(m_snd->getSyncPointInfo(m_pnt, m_label,
                SyncPointNameLength, nullptr, 0));
        }

        return m_label;
    }

    unsigned int SyncPointView::offset(TimeUnit unit) const
    {
        if (m_index < 0)
            std::runtime_error("SyncPointView is either empty or not loaded.");

        unsigned offset;

        FMOD_TIMEUNIT funit;

        switch(unit)
        {
        case TimeUnit::Milliseconds:
            funit = FMOD_TIMEUNIT_MS;
            break;
        case TimeUnit::Samples:
            funit = FMOD_TIMEUNIT_PCM;
            break;
        }

        checkResult(m_snd->getSyncPointInfo(m_pnt, nullptr, 0, &offset,
            funit));

        return offset;
    }

    SyncPointView &SyncPointView::operator++()
    {
        index(index() + 1);
        return *this;
    }

    SyncPointView SyncPointView::operator++(int)
    {
        auto point = *this;
        operator++();
        return point;
    }

    } // namespace Insound
