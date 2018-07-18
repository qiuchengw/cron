#pragma once

#include "exptimer.h"
#include "defs.h"

//////////////////////////////////////////////////////////////////////////
// 时间：[2018/4/11]
//
//  绝对时间表达式：
//  A = d; S = d; E = d; P = d; T = d1, d2, d3...; X = s;
//
//  A: 执行时间标志ENUM_TASK_EXECFLAG
//
//  	S, E : 起止日期，任务的有效期
//
//  	P : 单个INT值
//  	间隔日期：A = TASK_EXEC_ATDAILY有效，单位天。 比如每隔3天 P = 3;
//
//  Q: 单个INT值
//  	提前天数：A = AUTOTASK_EXEC_ATYEARDAY有效，单位天，比如提前3天
//
//  	T : 执行时间点，一个或多个INT值，此值可解析为绝对时间
//  	比如一天中的13：30，14：30，16：20。。。
//
//  	X : 1个多个整数值，以‘, ’分隔，具体含义由A决定
//  	1 > A = TASK_EXEC_ATDATE, X为多个日期点，可解析为日期
//  	2 > A = TASK_EXEC_ATDAILY, X为一个单个值，保存P间隔的天数
//  	3 > A = TASK_EXEC_ATMONTHDAY, X一个整数值代表为月的日号
//  	此数值为哪一天可执行，使用位标示，共32位，最低位为1号..
//  	4 > A = TASK_EXEC_ATWEEKDAY, X包含一个整数值，
//  	此值表示哪一天（星期日到星期六）可执行，使用位标识表示，ENUM_NTH_WEEKDAY
//  	5 > A = AUTOTASK_EXEC_ATYEARDAY，X为一个整数值，代表[月 - 日]
//  	每年的某个日期 2011~2020 的[11 / 11]，可以有一个提前量。类似于生日提醒
//////////////////////////////////////////////////////////////////////////

class AbsTimer : public ExpTimer {
public:
    AbsTimer(const mstring& exp);
    ~AbsTimer();

    ExpTimerRunningStatus nextExecDate(__inout uint32_t& dwDate);

    virtual bool parse() override;

    virtual mstring whenDoString() override;

    virtual void execTimeSpot(std::vector<dt::time>& vTimes) override;

    virtual  ExpTimerRunningStatus getNextExecTimeFrom(__inout dt::time& tmTest,
            __out dt::time& tmExec, __out int32_t &period_s) override;

protected:
    ExpTimerRunningStatus nextRightTimeFrom(__in const dt::time&tmTest,
                                            __in const dt::time& tmExec, __inout uint32_t &dwNextExecTime);

private:
    uint32_t			span_;		// 间隔时间或间隔日期

    //----------------------------------------
    IntArray		arx_;		// 执行日期点，需配合ENUM_TASK_EXECFLAG才能确定其值的意义
    IntArray		ar_time_;	// 执行时间点，
    //-----------------------------------------------
    // 在一天中的 [03:00 ~ 20:00] 每隔 30m 执行一次
    uint16_t			t_begin_;	// 执行时间
    uint16_t			t_end_;
};

