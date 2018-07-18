#include "abstimer.h"

namespace cron {

AbsTimer::AbsTimer(const mstring& exp)
    :ExpTimer(exp, TimerType::kTypeAbs) {
}

AbsTimer::~AbsTimer() {
}

bool AbsTimer::parse() {
    if (exp().IsEmpty())
        return false;

    mstring tmp_exp = exp(), val;
    char prop;
    while (!tmp_exp.IsEmpty()) {
        if (!_parse_prop_val(tmp_exp, prop, val))
            return false;
        switch (prop) {
        case L'A': {
            eflag_exec_ = (ExpTimerExecFlag)std::stol(val);
            break;
        }
        case L'X': {
            if (!_parse_to_array(val, arx_))
                return false;
            std::stable_sort(arx_.begin(), arx_.end());
            break;
        }
        case L'S': { // 时间
            t_begin_ = std::stol(val);
            break;
        }
        case L'E': { // 时间
            t_end_ = std::stol(val);
            break;
        }
        case L'Q': { // 时间
            span_ = std::stol(val);
            break;
        }
        case L'P': { // 间隔时间执行
            if (!_parse_to_array(val, arx_))
                return false;
            if (arx_[0] < 1)
                return false;
            break;
        }
        case L'T': { // 时间点执行
            if (!_parse_to_array(val, ar_time_))
                return false;
            // #ifdef _DEBUG
            // 				QTime tmT;
            // 				for (int i = 0; i < m_arTime.size(); i++)
            // 				{
            // 					tmT = QTime::ParseTime(m_arTime[i]);
            // 					TRACE(tmT.Format("%H:%M:%S\n"));
            // 				}
            // #endif
            // 从小到大排序
            std::stable_sort(ar_time_.begin(), ar_time_.end());
            break;
        }
        default:
            return false;
        }
    }
    return true;
}

// AtDate
ExpTimerRunningStatus AbsTimer::nextExecDate(__inout uint32_t& the_date) {
    if (the_date > dt::make_uint_date(life_end_)) {
        return AUTOTASK_RUNNING_STATUS_OVERDUE;	// 过期
    }

    if (arx_.size() < 1) {
        return AUTOTASK_RUNNING_STATUS_BADTIMER;
    }

    switch (eflag_exec_) {
    case AUTOTASK_EXEC_ATYEARDAY: {
        // 需要测试的执行日期
        dt::time t_test = dt::parse_uint_date(the_date);

        std::tm tmp_test = dt::to_calendar_time(t_test);
        for (int year = tmp_test.tm_year - 1; year < tmp_test.tm_year + 2; ++year) {
            // 设定的执行点
            dt::time t_exec = dt::parse_uint_date(arx_[0]);
            std::tm tmp_exec = dt::to_calendar_time(t_exec);
            dt::set_date(t_exec, year, tmp_exec.tm_mon, tmp_exec.tm_mday);
            // 执行点减去提前量的执行时间
            dt::time t_adv = t_exec;
            t_adv -= date::days(span_);

            // 如果执行时间甚至还没有到提前量的执行时间，那么返回提前量处的执行时间
            if ((dt::compare_date(t_adv, t_test) >= 0) && (dt::compare_date(t_adv, lifeEnd()) <= 0)) {
                the_date = dt::make_uint_date(t_adv);
                return AUTOTASK_RUNNING_STATUS_OK;
            }

            // 如果测试时间点落入[提前点，执行点]之间，那么返回这个测试时间点
            if ((dt::compare_date(t_test, t_adv) >= 0) && (dt::compare_date(t_test, t_exec) <= 0)) {
                the_date = dt::make_uint_date(t_test);
                return AUTOTASK_RUNNING_STATUS_OK;
            }
        }
        return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC;
    }

    case AUTOTASK_EXEC_ATDATE: {
        int idx = arx_.find_first_lgoreq(the_date);
        if (-1 != idx) {
            the_date = arx_[idx];
            return AUTOTASK_RUNNING_STATUS_OK;
        }
        return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC; // 无机会再执行
    }

    case AUTOTASK_EXEC_ATDAILY: {
        ASSERT(arx_.size() == 1);
        auto tsDate = dt::parse_uint_date(the_date) - dt::parse_uint_date(dt::make_uint_date(life_begin_));
        uint32_t xDay = dt::total_days(tsDate) % arx_[0]; // 还有几天下次执行
        if (xDay > 0) {
            dt::time tmNextExecDate = dt::parse_uint_date(the_date) + date::days(xDay);
            if (dt::compare_date(tmNextExecDate, life_end_) > 0) // 计算出来的时间大于生命期之后
                return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC;	// 无机会执行了。
            the_date = dt::make_uint_date(tmNextExecDate);
            return AUTOTASK_RUNNING_STATUS_OK;
        } else if (0 == xDay) { //dwDate 本身即为执行日期
            return AUTOTASK_RUNNING_STATUS_OK;
        }
        // 不应该执行到这儿
        return AUTOTASK_RUNNING_STATUS_APPERROR;
    }
    case AUTOTASK_EXEC_ATWEEKDAY: {
        // 星期x执行
        ASSERT(arx_.size() == 1);
        if (0 == arx_[0]) // 必须至少有一个工作日是可以执行的
            return AUTOTASK_RUNNING_STATUS_BADTIMER;
        dt::time tmTest = dt::parse_uint_date(the_date);
        for (int iTestCount = 0; iTestCount < 7; iTestCount++) {
            // 测试7天之内的执行情况,
            std::tm tmp_test = dt::to_calendar_time(tmTest);
            if (arx_[0] & (0x01 << (tmp_test.tm_wday - 1))) { // 0-sunday,1-monday...6-saturday
                the_date = dt::make_uint_date(tmTest);
                return AUTOTASK_RUNNING_STATUS_OK;
            }
            tmTest += date::days(1);
            if (dt::compare_date(tmTest, life_end_) > 0) {
                return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC;
            }
        }
        // 不应该执行到这儿,当前的设计是一周之内必有一天是可以执行的
        return AUTOTASK_RUNNING_STATUS_APPERROR;
    }
    case AUTOTASK_EXEC_ATMONTHDAY: {
        ASSERT(arx_.size() == 1);
        if (0 == arx_[0]) // 必须至少有一个工作日是可以执行的
            return AUTOTASK_RUNNING_STATUS_BADTIMER;
        dt::time tmTest = dt::parse_uint_date(the_date);
        for (int iTestCount = 0; iTestCount < 31; iTestCount++) {
            std::tm tmp_test = dt::to_calendar_time(tmTest);
            if (arx_[0] & (0x1 << (tmp_test.tm_mday))) {
                the_date = dt::make_uint_date(tmTest);
                return AUTOTASK_RUNNING_STATUS_OK;
            }
            tmTest += date::days(1); // 下一天
            if (dt::compare_date(tmTest, life_end_) > 0) {
                return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC;
            }
        }
        // 不应该执行到这儿，因为在31天之内，必有一天是可以执行的
        return AUTOTASK_RUNNING_STATUS_APPERROR;
    }
    default: {
        ASSERT(false);
        return AUTOTASK_RUNNING_STATUS_APPERROR;
    }
    }
    return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC;
}

mstring AbsTimer::description() {
    mstring when_des;
    mstring time_part;

    dt::time tm_test;
    for (int i = 0; i < ar_time_.size(); ++i) {
        tm_test = dt::parse_uint_time(ar_time_[i]);
        time_part += (dt::format_time(tm_test));
    }
    time_part.TrimRight(L',');

    switch (eflag_exec_) {
    case AUTOTASK_EXEC_ATYEARDAY: {
        tm_test = dt::parse_uint_date(arx_[0]);
        mstring tmp;
        if (0 != span_) {
            tmp.Format(" - 提前%d天", span_);
        }
        mstring date_part;
        std::tm t = dt::to_calendar_time(tm_test);
        date_part.Format("%d/%d%s", t.tm_mon, t.tm_mday, tmp);
        when_des.Format("在 <b .yellow>[%s] [%s]</b>", date_part, time_part);
        break;
    }
    case AUTOTASK_EXEC_ATDATE: {
        tm_test = dt::parse_uint_date(arx_[0]);
        when_des.Format("在 <b .yellow>[%s] [%s]</b>",
                        dt::format_date(tm_test), time_part);
        break;
    }
    case AUTOTASK_EXEC_ATDAILY: {
        ASSERT(arx_.size() == 1);
        when_des.Format("每 <b .yellow>[%d]</b> 天的 <b .yellow>[%s]</b>",
                        arx_[0], time_part);
        break;
    }
    case AUTOTASK_EXEC_ATWEEKDAY: {
        // 星期x执行
        ASSERT(arx_.size() == 1);
        mstring sWeekdays, sTemp;
        for (int iWeekday = 0; iWeekday < 7; iWeekday++) {
            // 测试7天之内的执行情况,
            if (arx_[0] & (0x01 << iWeekday)) { // 0-sunday,1-monday...6-saturday
                sTemp.Format("%d,", iWeekday);
                sWeekdays += sTemp;
            }
        }
        sWeekdays = sWeekdays.Left(sWeekdays.GetLength() - 1);
        when_des.Format("<b .yellow>星期[%s]</b>(0-周日,...,6-周六)的 [%s]<br />", sWeekdays, time_part);
        break;
    }
    case AUTOTASK_EXEC_ATMONTHDAY: {
        ASSERT(arx_.size() == 1);
        mstring days, tmp;
        for (int iDay = 0; iDay < 31; iDay++) {
            if (arx_[0] & (0x1 << iDay)) {
                tmp.Format("%d,", iDay);
                days += tmp;
            }
        }
        days = days.Left(days.GetLength() - 1);
        when_des.Format("每 <b .yellow>月[%s]</b> 的 [%s]", days, time_part);
        break;
    }
    }
    return when_des;
}


void AbsTimer::execTimeSpot(std::vector<dt::time>& vTimes) {
    vTimes.clear();
    for (int i = 0; i < ar_time_.size(); ++i) {
        vTimes.push_back(dt::parse_uint_time(ar_time_[i]));
    }
}

// 如果tmExec的日期大于tmTest的日期，tmNext设定为m_arTime[0],返回TASK_RUNNING_STATUS_OK
// 如果tmExec的日期等于tmTest的日期，tmNext设定为大于等于tmTest.MakeTime()的值，
//		如果m_arTime中包含这样的时间，返回TASK_RUNNING_STATUS_OK
//		如果不包含这样的时间，返回TASK_RUNNING_STATUS_TIMENOTMATCH
// 如果tmExec的日期小于tmTest的日期，则是逻辑错误发生，返回TASK_RUNNING_STATUS_OVERDUE
ExpTimerRunningStatus AbsTimer::nextRightTimeFrom(__in const dt::time&tmTest,
        __in const dt::time& tm_exec, __inout uint32_t &dwNextExecTime) {
    ASSERT(ar_time_.size());
    if (ar_time_.size()) {
        // 绝对时间值
        int iCmp = dt::compare_date(tm_exec, tmTest);
        if (iCmp > 0) {
            dwNextExecTime = ar_time_[0];
            return AUTOTASK_RUNNING_STATUS_OK;
        } else if (0 == iCmp) {
            int idx = ar_time_.find_first_lgoreq(dt::make_uint_time(tmTest));
            if (-1 != idx) {
                dwNextExecTime = ar_time_[idx];
                return AUTOTASK_RUNNING_STATUS_OK;
            }
            return AUTOTASK_RUNNING_STATUS_TIMENOTMATCH;
        } else {
            return AUTOTASK_RUNNING_STATUS_OVERDUE;
        }
    }
    // 这儿不应该被执行到
    ASSERT(false);
    return AUTOTASK_RUNNING_STATUS_BADTIMER;
}

// tmTest 将被调整，毫秒级别将会忽略置为0
ExpTimerRunningStatus AbsTimer::getNextExecTimeFrom( __inout dt::time& tm_test,
        __out dt::time& tm_exec, __out int32_t &period_s) {
    period_s = 0;

    if (tm_test >= life_end_) {
        return AUTOTASK_RUNNING_STATUS_OVERDUE;	// 过期
    }

    uint32_t next_exec_date, next_exec_time;
    ExpTimerRunningStatus status;
    dt::time tmTempTest = tm_exec = tm_test;
    while (true) {
        next_exec_date = dt::make_uint_date(tm_exec);
        status = nextExecDate(next_exec_date); // 执行日期
        if (AUTOTASK_RUNNING_STATUS_OK == status) {
            tm_exec = dt::parse_uint_date(next_exec_date);
            status = nextRightTimeFrom(tmTempTest, tm_exec, next_exec_time);
            if (AUTOTASK_RUNNING_STATUS_OK == status) {
                // 执行时间
                tm_exec = dt::combine_date_time(next_exec_date, next_exec_time);
                if (tm_exec > life_end_) // 必须检查合成的时间是否超过了任务周期
                    return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC;
                else
                    return AUTOTASK_RUNNING_STATUS_OK;
            } else if (AUTOTASK_RUNNING_STATUS_TIMENOTMATCH == status) {
                // 说明当前测试的日期是不可能执行的，只能比今天晚的日期执行
                // 这时时间可以是最小的
                tm_exec += date::days(1);
                dt::set_time(tm_exec, 0, 0, 0);
                dt::set_time(tmTempTest, 0, 0, 0);
                continue;
            } else {
                return status;
            }
        }
        return status;
    }
    return AUTOTASK_RUNNING_STATUS_BADTIMER;
}

}
