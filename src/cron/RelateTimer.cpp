#include "relatetimer.h"
#include "dict_zh.h"

namespace cron {

RelateTimer::RelateTimer(const mstring& exp)
    :ExpTimer(exp, TimerType::kTypeRelate) {
}

RelateTimer::~RelateTimer() {
}

mstring RelateTimer::description() {
    mstring when_des, time_part;
    when_des.Format("在[%s] [%d][%s]之后",
                    dict::getExecFlagText(eflag_exec_), span_, dict::timeUnitStr(span_unit_));
    // then every
    if (isExecSpan2()) {
        time_part.Format("然后每[%d][%s]执行", span2_, dict::timeUnitStr(span2_unit_));
        when_des += time_part;
        // after x times stop
        if (isExecCount()) {
            time_part.Format("在[%d] 次后停止", exec_count_);
            when_des += time_part;
        }
    }
    return when_des;
}

bool RelateTimer::parse() {
    if (exp().IsEmpty())
        return false;

    mstring sExpTest = exp();
    char prop;
    mstring val;
    while (!sExpTest.IsEmpty()) {
        if (!_parse_prop_val(sExpTest, prop, val))
            return false;
        switch (prop) {
        case 'R': {
            eflag_exec_ = (TimerExecType)std::stol(val);
            break;
        }
        case 'P': {
            if (!_parse_span_time(val, span_unit_, span_))
                return false;
            break;
        }
        case 'Q': { // 第二个时间间隔
            if (!_parse_span_time(val, span2_unit_, span2_))
                return false;
            break;
        }
        case 'C': { // 执行次数
            exec_count_ = std::stol(val);
            break;
        }
        default:
            return false;
        }
    }
    return true;
}

TimerRunningStatus RelateTimer::_CheckWith(
    const dt::time& tm_start, const dt::time& tm_test, __out dt::time& tm_exec, __out int32_t &period_s) {
    if (tm_test >= life_end_) {
        return TimerRunningStatus::kOverdue;
    }
    period_s = execSpanSeconds2();
    // 第一次执行时间是：
    dt::time tm_first_exec = tm_start + dt::secs(execSpanSeconds());

    // 错过了第一次执行时间
    if (tm_first_exec <= tm_test) {
        if (!isExecSpan2()) {
            // 错过第一次执行时间，并且非多次执行
            if (TimerExecType::kAfterSysBoot == eflag_exec_)
                return TimerRunningStatus::kUntilNextSysReboot; // 等待系统重启
            else if (TimerExecType::kAfterAppStart == eflag_exec_)
                return TimerRunningStatus::kUntilNextAppReboot; // 等待程序重启
            else // 非多次可执行，过期
                return TimerRunningStatus::kNoChanceExec;
        }

        // 多次间隔执行
        // 自从第一次可执行时间到tm_test已经过去了多长时间
        int64_t dTotalSeconds = dt::total_seconds(tm_test - tm_first_exec);
        // 在过去的这么长时间里可以执行多少次？
        exec_count_already_ = dTotalSeconds / execSpanSeconds2(); //执行次数
        if (isExecCount() && (exec_count_already_ >= exec_count_)) {
            // 可执行次数已经超过了总共需要执行的次数
            if (TimerExecType::kAfterSysBoot == eflag_exec_)
                return TimerRunningStatus::kUntilNextSysReboot; // 等待系统重启
            else if (TimerExecType::kAfterAppStart == eflag_exec_)
                return TimerRunningStatus::kUntilNextAppReboot; // 等待程序重启
            else // 非多次可执行，过期
                return TimerRunningStatus::kNoChanceExec;
        } else {
            // 可执行次数还没有超过了总共需要执行的次数
            tm_exec = tm_first_exec + dt::secs((exec_count_already_ + 1) * execSpanSeconds2());
            if (tm_exec >= life_end_) {
                return TimerRunningStatus::kOverdue;
            }
            return TimerRunningStatus::kOk;
        }
    } else {
        tm_exec = tm_first_exec;
        return TimerRunningStatus::kOk;
    }
}

RelateTimer::TimerBehavior RelateTimer::onFired(OnTimeoutCallback cb, void *d) {
    cb(d);

    exec_count_already_++;
    if (isExecCount() && exec_count_already_ >= exec_count_) {
        // 需要停止定时器了
        return TimerBehavior::kStop;
    }
    return TimerBehavior::kContinue;
}

// tm_test 将被调整，毫秒级别将会忽略置为0
TimerRunningStatus RelateTimer::getNextExecTimeFrom(
    __inout dt::time& tm_test,__out dt::time& tm_exec,__out int32_t &period_s) {
    if (tm_test >= life_end_) {
        return TimerRunningStatus::kOverdue;	// 过期
    }

    switch (eflag_exec_) {
    case TimerExecType::kAfterSysBoot: {	//= 0x00000001,	// 系统启动
        // return _RelateTime_CheckWith(QProcessMan::GetSystemStartupTime(),tm_test,tm_exec, period_s);
        break;
    }
    case TimerExecType::kAfterAppStart: {	// = 0x00000004,// 本程序启动
        //return _RelateTime_CheckWith(QProcessMan::GetCurrentProcessStartupTime(),tm_test,tm_exec, period_s);
        break;
    }
    /*	case TASK_EXEC_AFTERPROGSTART:	// = 0x00000008,// 外部程序启动
    {
    QTime tmProgStart;
    if (QProcessMgr::IsExeRun(m_sXFiledExp,tmProgStart))
    {
    return _RelateTime_CheckWith(tmProgStart,tm_test,tm_exec);
    }
    return TASK_RUNNING_STATUS_BASEDONEXETERNALPROG;
    }
    case TASK_EXEC_AFTERPROGEXIT:	// = 0x00000010,// 外部程序退出
    {
    return TASK_RUNNING_STATUS_BASEDONEXETERNALPROG;
    }*/
    case TimerExecType::kAfterTimerStart: {	// = 0x00000002,	// 任务启动
        return _CheckWith(life_begin_, tm_test, tm_exec, period_s);
    }
    }
    ASSERT(false);
    return TimerRunningStatus::kBadTimer;
}

}
