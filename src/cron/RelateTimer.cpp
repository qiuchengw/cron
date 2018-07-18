
#include "relatetimer.h"


namespace cron {

RelateTimer::RelateTimer(const mstring& exp)
    :ExpTimer(exp, ExpTimerType::kTimerTypeRelate) {
}

RelateTimer::~RelateTimer() {
}

mstring RelateTimer::whenDoString() {
    mstring when_des, time_part;
    when_des.Format("在<b .yellow>[%s] [%d][%s]</b>之后",
                    getExecFlagText(eflag_exec_), span_, QHelper::GetTimeUnitString(span_unit_));
    // then every
    if (isExecSpan2()) {
        time_part.Format("然后每 <b .yellow>[%d][%s]</b>执行", span2_, QHelper::GetTimeUnitString(span2_unit_));
        when_des += "<br/>" + time_part;
        // after x times stop
        if (isExecCount()) {
            time_part.Format("在 <b .yellow>[%d]</b> 次后停止", exec_count_);
            when_des += "<br />" + time_part;
        }
    }
    return when_des;
}

bool RelateTimer::parse() {
    if (exp().IsEmpty())
        return false;

    mstring sExpTest = exp();
    wchar_t cProp;
    mstring sValue;
    while (!sExpTest.IsEmpty()) {
        if (!_parse_prop_val(sExpTest, cProp, sValue))
            return false;
        switch (cProp) {
        case L'R': {
            eflag_exec_ = (ExpTimerExecFlag)std::stol(sValue);
            break;
        }
        case L'P': {
            if (!_parse_span_time(sValue, span_unit_, span_))
                return false;
            break;
        }
        case L'Q': { // 第二个时间间隔
            if (!_parse_span_time(sValue, span2_unit_, span2_))
                return false;
            break;
        }
        case L'C': { // 执行次数
            exec_count_ = std::stol(sValue);
            break;
        }
        default:
            return false;
        }
    }
    return true;
}

ExpTimerRunningStatus RelateTimer::_CheckWith(
    const dt::time& tm_start, const dt::time& tm_test, __out dt::time& tm_exec, __out int32_t &period_s) {
    if (tm_test >= life_end_) {
        return AUTOTASK_RUNNING_STATUS_OVERDUE;
    }
    period_s = execSpanSeconds2();
    // 第一次执行时间是：
    dt::time tm_first_exec = tm_start + dt::secs(execSpanSeconds());

    // 错过了第一次执行时间
    if (tm_first_exec <= tm_test) {
        if (!isExecSpan2()) {
            // 错过第一次执行时间，并且非多次执行
            if (AUTOTASK_EXEC_AFTERSYSBOOT == eflag_exec_)
                return AUTOTASK_RUNNING_STATUS_UNTILNEXTSYSREBOOT; // 等待系统重启
            else if (AUTOTASK_EXEC_AFTERMINDERSTART == eflag_exec_)
                return AUTOTASK_RUNNING_STATUS_UNTILNEXTMINDERREBOOT; // 等待程序重启
            else // 非多次可执行，过期
                return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC;
        }

        // 多次间隔执行
        // 自从第一次可执行时间到tmTest已经过去了多长时间
        int64_t dTotalSeconds = dt::total_seconds(tm_test - tm_first_exec);
        // 在过去的这么长时间里可以执行多少次？
        exec_count_already_ = dTotalSeconds / execSpanSeconds2(); //执行次数
        if (isExecCount() && (exec_count_already_ >= exec_count_)) {
            // 可执行次数已经超过了总共需要执行的次数
            if (AUTOTASK_EXEC_AFTERSYSBOOT == eflag_exec_)
                return AUTOTASK_RUNNING_STATUS_UNTILNEXTSYSREBOOT; // 等待系统重启
            else if (AUTOTASK_EXEC_AFTERMINDERSTART == eflag_exec_)
                return AUTOTASK_RUNNING_STATUS_UNTILNEXTMINDERREBOOT; // 等待程序重启
            else // 非多次可执行，过期
                return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC;
        } else {
            // 可执行次数还没有超过了总共需要执行的次数
            tm_exec = tm_first_exec + dt::secs((exec_count_already_ + 1) * execSpanSeconds2());
            if (tm_exec >= life_end_) {
                return AUTOTASK_RUNNING_STATUS_OVERDUE;
            }
            return AUTOTASK_RUNNING_STATUS_OK;
        }
    } else {
        tm_exec = tm_first_exec;
        return AUTOTASK_RUNNING_STATUS_OK;
    }
}

RelateTimer::EnumTimerFiredFlag RelateTimer::onTimerFired(OnTimeoutCallback cb, void *d) {
    cb(d);

    exec_count_already_++;
    if (isExecCount() && exec_count_already_ >= exec_count_) {
        // 需要停止定时器了
        return EnumTimerFiredFlag::kTimerStop;
    }
    return EnumTimerFiredFlag::kTimerContinue;
}

// tmTest 将被调整，毫秒级别将会忽略置为0
ExpTimerRunningStatus RelateTimer::getNextExecTimeFrom(
    __inout dt::time& tmTest,__out dt::time& tmExec,__out int32_t &period_s) {
    if (tmTest >= life_end_) {
        return AUTOTASK_RUNNING_STATUS_OVERDUE;	// 过期
    }

    switch (eflag_exec_) {
    case AUTOTASK_EXEC_AFTERSYSBOOT: {	//= 0x00000001,	// 系统启动
        // return _RelateTime_CheckWith(QProcessMan::GetSystemStartupTime(),tmTest,tmExec, period_s);
        break;
    }
    case AUTOTASK_EXEC_AFTERMINDERSTART: {	// = 0x00000004,// 本程序启动
        //return _RelateTime_CheckWith(QProcessMan::GetCurrentProcessStartupTime(),tmTest,tmExec, period_s);
        break;
    }
    /*	case TASK_EXEC_AFTERPROGSTART:	// = 0x00000008,// 外部程序启动
    {
    QTime tmProgStart;
    if (QProcessMgr::IsExeRun(m_sXFiledExp,tmProgStart))
    {
    return _RelateTime_CheckWith(tmProgStart,tmTest,tmExec);
    }
    return TASK_RUNNING_STATUS_BASEDONEXETERNALPROG;
    }
    case TASK_EXEC_AFTERPROGEXIT:	// = 0x00000010,// 外部程序退出
    {
    return TASK_RUNNING_STATUS_BASEDONEXETERNALPROG;
    }*/
    case AUTOTASK_EXEC_AFTERTASKSTART: {	// = 0x00000002,	// 任务启动
        return _CheckWith(life_begin_, tmTest, tmExec, period_s);
    }
    }
    ASSERT(false);
    return AUTOTASK_RUNNING_STATUS_BADTIMER;
}

}
