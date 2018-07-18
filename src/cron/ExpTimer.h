#pragma once

#include "defs.h"
#include "timerinterface.h"

enum class ExpTimerType {
    kTimerTypeInvalid = 0,
    kTimerTypeAbs,
    kTimerTypeRelate,
};

#define MONTH_LAST_DAY 32

enum ENUM_NTH_WEEK {
    FIRST_WEEK = 0x1,
    SECOND_WEEK = 0x2,
    THIRD_WEEK = 0x4,
    FORTH_WEEK = 0x8,
    FIVETH_WEEK = 0x10,
};

enum ENUM_NTH_WEEKDAY {
    SUNDAY = 0x1,
    MONDAY = 0x2,
    TUESDAY = 0x4,
    WEDNESDAY = 0x8,
    THURSDAY = 0x10,
    FRIDAY = 0x20,
    SATURDAY = 0x40,
};


//////////////////////////////////////////////////////////////////////////
// 时间：[2018/4/11]
// 使用方式：
//	1,create
//	2,parse
//	3,startFrom
//////////////////////////////////////////////////////////////////////////
class TimerInterface;
class ExpTimer {
protected:
    enum class EnumTimerFiredFlag {
        kTimerStop = 1,
        kTimerContinue,
        kTimerRefresh,
    };
public:
    ExpTimer(const mstring& exp = "", ExpTimerType t = ExpTimerType::kTimerTypeInvalid);
    ~ExpTimer();

    // 创建一个实例
    static ExpTimer* create(const mstring& exp);

    void stop();

    // 启动
    ExpTimerRunningStatus startFrom(dt::time &tmBegin, OnTimeoutCallback cb, void* d);

    // 何时做
    virtual mstring whenDoString() = 0;

    // 执行时间点
    virtual void execTimeSpot(std::vector<dt::time>& vTimes) {
    }

    inline bool setLifeTime(dt::time tmLifeBegin, dt::time tmLifeEnd) {
        if (tmLifeEnd <= tmLifeBegin) {
            return false;
        }
        life_begin_ = tmLifeBegin;
        life_end_ = tmLifeEnd;
        return true;
    }

    inline dt::time lifeBegin() {
        return life_begin_;
    }

    inline dt::time lifeEnd() {
        return life_end_;
    }

    inline bool started()const {
        return timer_id_ > 0;
    }

    inline mstring exp()const {
        return exp_;
    }

protected:
    // 解析
    virtual bool parse() = 0;

    virtual EnumTimerFiredFlag onTimerFired(OnTimeoutCallback cb, void *d);

    // tmTest之后的下次执行时间
    // tmTest 将被调整，毫秒级别将会忽略置为0
    // period_s 间隔执行的周期
    virtual  ExpTimerRunningStatus getNextExecTimeFrom(__inout dt::time& tmTest, __out dt::time& tmExec, __out int32_t &period_s) = 0;

    bool setTimer(int64_t due, int64_t period, OnTimeoutCallback cb, void* d);

protected:
    // sExp: r=1;b=2;c=3
    // result:
    //	sExp = b=2;c=3;
    //	sProp = r;
    //	sValue = 1;
    bool _parse_prop_val(__inout mstring&sExp, __out wchar_t& cProp, __out mstring& sValue) {
        int idx = sExp.Find(L'=');
        if ((-1 == idx) || (idx != 1) || ((idx = sExp.Find(L';')) == -1)) {
            return false;
        }
        cProp = sExp[0];
        sValue = sExp.Mid(2, idx - 2);
        sExp = sExp.Mid(idx + 1);
        return true;
    }

    // 2,4,8,9
    // 以 英文‘,’ 分隔的数字字符，转换到整数值
    bool _parse_to_array(__inout mstring& sExp, __out IntArray & ar);

    // 200[s|m|h]
    // 200m = 200 * 60s;  cUnit = m;
    // 200h = 200 * 3600s	cUnit = h;
    // 200s = 200s;		cUnit = s;
    bool _parse_span_time(__in const mstring &sExp, __out wchar_t& cUnit, __out uint32_t& dwSpan);

protected:
    ExpTimerExecFlag eflag_exec_;
    dt::time			life_begin_;	// 任务的生效时间
    dt::time			life_end_;	// 任务过期时间

private:
    int timer_id_ = 0;
    ExpTimerType type_ = ExpTimerType::kTimerTypeInvalid;
    mstring exp_;
    dt::time		tm_next_exec_;//	下次执行时间
};

