#include "stdafx.h"
#include "exptimer.h"
#include "abstimer.h"
#include "relatetimer.h"

ExpTimer::ExpTimer(const mstring& exp, ExpTimerType t)
    :exp_(exp), type_(t) {
}

ExpTimer::~ExpTimer() {
}

ExpTimer* ExpTimer::create(const mstring& exp) {
    if (exp.IsEmpty()) {
        return nullptr;
    }

    ExpTimer* t = nullptr;
    switch (exp.GetAt(0)) {
    case L'R':
        t = new RelateTimer(exp);
        break;

    case L'A':
        t = new AbsTimer(exp);
        break;
    }
    if (t) {
        t->parse();
    }
    return t;
}

void ExpTimer::stop() {
    if (timer_id_ > 0) {
        timer::stop(timer_id_);
        timer_id_ = 0;
    }
}

ExpTimer::EnumTimerFiredFlag ExpTimer::onTimerFired(OnTimeoutCallback cb, void *d) {
    cb(d);

    // 刷新timer
    return EnumTimerFiredFlag::kTimerRefresh;
}

bool ExpTimer::setTimer(int64_t due, int64_t period, OnTimeoutCallback cb, void* d) {
    stop();

    timer_id_ = timer::set(due, period, [=](void*) {
        switch (this->onTimerFired(cb, d)) {
        case EnumTimerFiredFlag::kTimerContinue:
            break;

        case EnumTimerFiredFlag::kTimerStop:
            stop();
            break;

        case EnumTimerFiredFlag::kTimerRefresh:
            startFrom(dt::now(), cb, d);
            break;
        };
    }, d);
    return timer_id_ > 0;
}

bool ExpTimer::_parse_to_array(__inout mstring& sExp, __out IntArray & ar) {
    ar.clear();
    int idx;
    while (!sExp.IsEmpty()) {
        idx = sExp.Find(L',');
        if (idx != -1) {
            ar.push_back(std::stol(sExp.Mid(0, idx)));
            sExp = sExp.Mid(idx + 1);
        } else {
            ar.push_back(std::stol(sExp));
            break;
        }
    }
    return ar.size() > 0;
}

bool ExpTimer::_parse_span_time(__in const mstring &sExp, __out wchar_t& cUnit, __out uint32_t& dwSpan) {
    int len = sExp.GetLength();
    cUnit = tolower(sExp.back());
    dwSpan = std::stol(sExp.Mid(0, len - 1));
    switch (cUnit) {
    case L'm':
    case L'M':
    case L's':
    case L'S':
    case L'H':
    case L'h':
        return true;
    }
    ASSERT(false);
    return false;
}

ExpTimerRunningStatus ExpTimer::startFrom(dt::time &tmBegin, OnTimeoutCallback cb, void* d) {
    if (started()) {
        return AUTOTASK_RUNNING_STATUS_OK;
    }

    // 先停掉原来的
    stop();

    int64_t lHowLongToExec;
    ExpTimerRunningStatus eStatus;
    while (true) {
        int32_t period_s = 0;
        eStatus = getNextExecTimeFrom(tmBegin, tm_next_exec_, period_s);
        if (eStatus != AUTOTASK_RUNNING_STATUS_OK) {
            break;
        }
        lHowLongToExec = dt::total_seconds(tm_next_exec_ - dt::now());
        if (lHowLongToExec <= 0) {
            // 往后找
            tmBegin = tm_next_exec_ + dt::secs(1);
            continue;
        }

        if (cb) {
            if (setTimer(lHowLongToExec * 1000, period_s * 1000, cb, d)) {
                return AUTOTASK_RUNNING_STATUS_APPERROR;
            }
        }
        break;
    }
    return eStatus;
}
