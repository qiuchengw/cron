#pragma once

#include <list>
#include <vector>

#include "stdstring.h"
typedef CStdStr<char> mstring;
typedef std::vector<mstring> StringArray;


#ifndef INVALID_ID
#define INVALID_ID (-1)
#endif

#ifndef _HasFlag
#	define _HasFlag(l,f) ((l) & (f))
#endif

#ifndef _AddFlag
#	define _AddFlag(l,f) ((l) |= (f))
#endif

#ifndef _RemoveFlag
#	define _RemoveFlag(l,f) ((l) &= ~(f))
#endif

#ifndef __in
#define __in
#define __out
#define __inout
#endif

namespace cron {

enum class TimerExecType {
    kNotSet = 0,	// 系统启动

    // 相对时间
    kAfterSysBoot = 0x00000001,	// 系统启动
    kAfterTimerStart = 0x00000002,	// 任务启动
    kAfterAppStart = 0x00000004,// 本程序启动
    kAfterExternalAppStart = 0x00000008,// 外部程序启动
    kAfterAppExit = 0x00000010,// 外部程序退出

    // 绝对时间标记
    kAtDate = 0x00010000,	// 绝对日期 2011/11/11
    kAtDaily = 0x00020000,	// 每隔x天
    kAtMonthDay = 0x00040000,	// 每月的x号
    kAtWeekday = 0x00080000,	// 每月的x周 x[所有周|第一周|。。|第4周]
    kAtYearDay = 0x00100000,	// 每年的某个日期 2011~2020 的 [11/11]，可以有一个提前量。类似于生日提醒

    //////////////////////////////////////////////////////////////////////////
    kExecAndThen = 0x01000000,	// 相对时间之后再次执行多次间隔
};

// 任务运行时状态
enum class TimerRunningStatus {
    kBadTimer = -2,	// 不能解析timer表达式
    kAppError = -1,	// 应用程序出现了错误
    kOk = 0,	// 任务正常启动
    kNotStartup = 1,	// 任务还未启动
    kPaused,	// 任务还未启动
    kOverdue,	// 任务过期了
    kUntilNextSysReboot,	// 需要下次机器重启，任务才执行
    kUntilNextAppReboot,	// 需要程序重启，任务才执行
    kBasedOnExternalApp,	// 依赖的外部程序并没有运行
    // 绝对时间
    kTimeNotMatch,	// 无可执行的时间匹配
    kNoChanceExec,	// 虽然任务未过期，但是余下的时间里，任务都没有机会再执行了
};

}

