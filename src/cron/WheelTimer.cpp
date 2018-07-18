#include "stdafx.h"

// mtimer.cpp : 定义 DLL 应用程序的导出函数。
//

// cpp file //////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS
#include "WheelTimer.h"

#ifdef _MSC_VER
# include <sys/timeb.h>
#else
# include <sys/time.h>
#endif

#define TVN_BITS 6
#define TVR_BITS 8
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)
#define OFFSET(N) (TVR_SIZE + (N) *TVN_SIZE)
#define INDEX(V, N) ((V >> (TVR_BITS + (N) *TVN_BITS)) & TVN_MASK)

//////////////////////////////////////////////////////////////////////////
// Timer

WheelTimer::WheelTimer(WheelTimerManager& manager)
    : manager_(manager)
    , vecIndex_(-1) {
}

WheelTimer::~WheelTimer() {
    Stop();
}

void WheelTimer::Stop() {
    if (vecIndex_ != -1) {
        manager_.RemoveTimer(this);
        vecIndex_ = -1;
    }
}

void WheelTimer::OnTimer(unsigned long long now) {
    if (timerType_ == WheelTimer::CIRCLE) {
        expires_ = interval_ + now;
        manager_.AddTimer(this);
    } else {
        vecIndex_ = -1;
    }
    timerFun_(data_);
}

//////////////////////////////////////////////////////////////////////////
// TimerManager

WheelTimerManager::WheelTimerManager() {
    tvec_.resize(TVR_SIZE + 4 * TVN_SIZE);
    checkTime_ = GetCurrentMillisecs();
}

void WheelTimerManager::AddTimer(WheelTimer* timer) {
    unsigned long long expires = timer->expires_;
    unsigned long long idx = expires - checkTime_;

    if (idx < TVR_SIZE) {
        timer->vecIndex_ = expires & TVR_MASK;
    } else if (idx < 1 << (TVR_BITS + TVN_BITS)) {
        timer->vecIndex_ = OFFSET(0) + INDEX(expires, 0);
    } else if (idx < 1 << (TVR_BITS + 2 * TVN_BITS)) {
        timer->vecIndex_ = OFFSET(1) + INDEX(expires, 1);
    } else if (idx < 1 << (TVR_BITS + 3 * TVN_BITS)) {
        timer->vecIndex_ = OFFSET(2) + INDEX(expires, 2);
    } else if ((long long)idx < 0) {
        timer->vecIndex_ = checkTime_ & TVR_MASK;
    } else {
        if (idx > 0xffffffffUL) {
            idx = 0xffffffffUL;
            expires = idx + checkTime_;
        }
        timer->vecIndex_ = OFFSET(3) + INDEX(expires, 3);
    }

    TimeList& tlist = tvec_[timer->vecIndex_];
    tlist.push_back(timer);
    timer->itr_ = tlist.end();
    --timer->itr_;
}

void WheelTimerManager::RemoveTimer(WheelTimer* timer) {
    TimeList& tlist = tvec_[timer->vecIndex_];
    tlist.erase(timer->itr_);
}

void WheelTimerManager::DetectTimers() {
    if (tvec_.empty()) {
        return;
    }

    unsigned long long now = GetCurrentMillisecs();
    while (checkTime_ <= now) {
        int index = checkTime_ & TVR_MASK;
        if (!index &&
                !Cascade(OFFSET(0), INDEX(checkTime_, 0)) &&
                !Cascade(OFFSET(1), INDEX(checkTime_, 1)) &&
                !Cascade(OFFSET(2), INDEX(checkTime_, 2))) {
            Cascade(OFFSET(3), INDEX(checkTime_, 3));
        }
        ++checkTime_;

        if (tvec_.empty()) {
            break;
        }

        TimeList& tlist = tvec_[index];
        TimeList temp;
        temp.splice(temp.end(), tlist);
        for (TimeList::iterator itr = temp.begin(); itr != temp.end(); ++itr) {
            (*itr)->OnTimer(now);
        }
    }
}

int WheelTimerManager::Cascade(int offset, int index) {
    TimeList& tlist = tvec_[offset + index];
    TimeList temp;
    temp.splice(temp.end(), tlist);

    for (TimeList::iterator itr = temp.begin(); itr != temp.end(); ++itr) {
        AddTimer(*itr);
    }

    return index;
}

unsigned long long WheelTimerManager::GetCurrentMillisecs() {
#ifdef _MSC_VER
    _timeb timebuffer;
    _ftime(&timebuffer);
    unsigned long long ret = timebuffer.time;
    ret = ret * 1000 + timebuffer.millitm;
    return ret;
#else
    timeval tv;
    ::gettimeofday(&tv, 0);
    unsigned long long ret = tv.tv_sec;
    return ret * 1000 + tv.tv_usec / 1000;
#endif
}
