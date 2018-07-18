#include "stdafx.h"
#include "timerinterface.h"

namespace timer {
static MyTimerMan<MyTimer, 2, 50> _s_man_;

int set(uint32_t due, uint32_t period, OnTimeoutCallback cb, void* p) {
    return _s_man_.timer(due, period, cb, p);
}

void stop(int id) {
    _s_man_.stopTimer(id);
}

void stop() {
    _s_man_.stop();
}

}
