#pragma once

#include "exptimer.h"
#include "defs.h"


//////////////////////////////////////////////////////////////////////////
// 时间：[2018/4/11]
//
//  相对时间表达式：
//	R = 1; P = 20m; Q = 22m; C = 100;
//		R: 相对于1（TASK_EXEC_AFTERSYSBOOT）
//  	P : 20minutes后执行任务 P[1s~24h]
//  	Q : 然后每个22minutes执行一次 Q[1s~24h]
//  	C : 执行100次后停止，C[1~86400]
//
//////////////////////////////////////////////////////////////////////////

namespace cron {

class RelateTimer : public ExpTimer {
public:
    RelateTimer(const mstring& exp);
    ~RelateTimer();

    virtual mstring description() override;

    virtual bool parse() override;

    virtual TimerRunningStatus getNextExecTimeFrom(__inout dt::time& tm_test,
            __out dt::time& tm_exec, __out int32_t &period_s) override;

    uint32_t execSpanSeconds() const {
        return helper::howManySecs(span_, span_unit_);
    }

    uint32_t execSpanSeconds2()const {
        return helper::howManySecs(span2_, span2_unit_);
    }

    // 执行第一次后是否再间隔执行
    bool isExecSpan2()const {
        return (span2_ > 0);
    }

    bool isExecCount()const {
        // -1 是无限次，0 是不执行， >0 是具体次
        return (exec_count_ == -1) || exec_count_ > 0;
    }

protected:
    TimerRunningStatus _CheckWith(const dt::time& tm_start, const dt::time& tm_test,
                                     __out dt::time& tm_exec, __out int32_t &period_s);

    virtual TimerBehavior onFired(OnTimeoutCallback cb, void *d) override;

private:
    // 相对于ENUM_TASK_EXECFLAG所指示的任务的
    // 发生后多少时间执行第一次任务，单位换算为秒
    // 此时间限制为24小时之内
    uint32_t span_ = 0;		// 间隔时间或间隔日期
    char span_unit_ = 0;	// 原始单位s/m/h
    // 相对于第一次任务执行后多少时间执行
    uint32_t span2_ = 0;	// 第二个时间间隔
    char span2_unit_ = 0;
    int exec_count_ = 0;	// 第一次执行后，再执行的次数
    uint32_t exec_count_already_ = 0;	// 已经执行的次数
    TimerExecType eflag_exec_ = TimerExecType::kNotSet;
};

}
