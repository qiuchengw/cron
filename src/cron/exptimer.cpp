#include "exptimer.h"
#include "abstimer.h"
#include "relatetimer.h"

namespace cron {

ExpTimer::ExpTimer(const mstring& exp, TimerType t)
    :exp_(exp), type_(t) {

	// 默认设置生命期为一个比较长的时间
	if (dt::parse(life_end_, "3999-1-1")) {
		setLife(dt::now(), life_end_);
	}
}

ExpTimer::~ExpTimer() {
}

ExpTimer* ExpTimer::create(const mstring& sexp) {
	// 要去除exp中的所有空格
	mstring exp = sexp;
	exp.Trim();
	exp.Remove(' ');
	exp.Remove('\t');

	if (exp.IsEmpty()) {
        return nullptr;
    }


    ExpTimer* t = nullptr;
    switch (exp.GetAt(0)) {
    case 'R':
        t = new RelateTimer(exp);
		break;

    case 'A':
        t = new AbsTimer(exp);
		break;
    }

	if (t) {
		if (!t->parse()) {
			delete t;
			t = nullptr;
		}
	}
    return t;
}

void ExpTimer::stop() {
    if (timer_id_ > 0) {
        timer::stop(timer_id_);
        timer_id_ = 0;
    }
}

ExpTimer::TimerBehavior ExpTimer::onFired(OnTimeoutCallback cb, void *d) {
    cb(d);

    // 刷新timer
    return TimerBehavior::kRefresh;
}

bool ExpTimer::setTimer(int64_t due, int64_t period, OnTimeoutCallback cb, void* d) {
    stop();

    timer_id_ = timer::set(due, period, [=](void*) {
        switch (this->onFired(cb, d)) {
        case TimerBehavior::kContinue:
            break;

        case TimerBehavior::kStop:
            stop();
            break;

        case TimerBehavior::kRefresh:
			auto tm_now = dt::now();	// gcc 需要这个临时变量，vs不需要
			startFrom(tm_now, cb, d);
            break;
        };
    }, d);
    return timer_id_ > 0;
}

bool ExpTimer::_parse_to_array(__inout mstring& exp, __out Ints & ar) {
    ar.clear();
    int idx;
    while (!exp.IsEmpty()) {
        idx = exp.Find(',');
        if (idx != -1) {
            ar.push_back(std::stol(exp.Mid(0, idx)));
            exp = exp.Mid(idx + 1);
        } else {
            ar.push_back(std::stol(exp));
            break;
        }
    }
    return ar.size() > 0;
}

bool ExpTimer::_parse_span_time(__in const mstring &exp, __out char& unit, __out uint32_t& span) {
    int len = exp.GetLength();
    unit = tolower(exp.back());
    span = std::stol(exp.Mid(0, len - 1));
    switch (unit) {
    case 'm':
    case 'M':
    case 's':
    case 'S':
    case 'H':
    case 'h':
        return true;
    }
    ASSERT(false);
    return false;
}

TimerRunningStatus ExpTimer::startFrom(dt::time &begin, OnTimeoutCallback cb, void* d) {
    if (started()) {
        return TimerRunningStatus::kOk;
    }

    // 先停掉原来的
    stop();

    int64_t secs_to_exec;
    TimerRunningStatus status;
    while (true) {
        int32_t period_s = 0;
        status = getNextExecTimeFrom(begin, next_exec_, period_s);
        if (status != TimerRunningStatus::kOk) {
            break;
        }
        secs_to_exec = dt::total_seconds(next_exec_ - dt::now());
        if (secs_to_exec <= 0) {
            // 往后找
            begin = next_exec_ + dt::secs(1);
            continue;
        }

        if (cb) {
            if (setTimer(secs_to_exec * 1000, period_s * 1000, cb, d)) {
                return TimerRunningStatus::kAppError;
            }
        }
        break;
    }
    return status;
}
}
