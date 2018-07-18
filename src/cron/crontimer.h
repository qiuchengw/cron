#pragma once

#include <list>
#include "exarray.h"
#include "constvalues.h"
#include "timerinterface.h"
#include "reminderexp.h"

// enum ENUM_AUTOTASK_EXECFLAG
// {
//     AUTOTASK_EXEC_NOTSET	= 0,	// 系统启动
//     // 相对时间
//     AUTOTASK_EXEC_AFTERSYSBOOT	= 0x00000001,	// 系统启动
//     AUTOTASK_EXEC_AFTERTASKSTART = 0x00000002,	// 任务启动
//     AUTOTASK_EXEC_AFTERMINDERSTART = 0x00000004,// 本程序启动
//     AUTOTASK_EXEC_AFTERPROGSTART = 0x00000008,// 外部程序启动
//     AUTOTASK_EXEC_AFTERPROGEXIT = 0x00000010,// 外部程序退出
//     // 绝对时间标记
//     AUTOTASK_EXEC_ATDATE = 0x00010000,	// 绝对日期 2011/11/11
//     AUTOTASK_EXEC_ATDAILY = 0x00020000,	// 每隔x天
//     AUTOTASK_EXEC_ATMONTHDAY = 0x00040000,	// 每月的x号
//     AUTOTASK_EXEC_ATWEEKDAY = 0x00080000,	// 每月的x周 x[所有周|第一周|。。|第4周]
//
//     //////////////////////////////////////////////////////////////////////////
//     AUTOTASK_EXEC_RELATE_EXECANDTHEN = 0x01000000,	// 相对时间之后再次执行多次间隔
//
// };
//
// // 任务运行时状态
// enum ENUM_AUTOTASK_RUNNING_STATUS
// {
//     AUTOTASK_RUNNING_STATUS_BADTIMER = -2,	// 不能解析timer表达式
//     AUTOTASK_RUNNING_STATUS_APPERROR = -1,	// 应用程序出现了错误
//     AUTOTASK_RUNNING_STATUS_OK = 0,	// 任务正常启动
//     AUTOTASK_RUNNING_STATUS_NOTSTARTUP = 1,	// 任务还未启动
//     AUTOTASK_RUNNING_STATUS_PAUSED,	// 任务还未启动
//     AUTOTASK_RUNNING_STATUS_OVERDUE,	// 任务过期了
//     AUTOTASK_RUNNING_STATUS_UNTILNEXTSYSREBOOT,	// 需要下次机器重启，任务才执行
//     AUTOTASK_RUNNING_STATUS_UNTILNEXTMINDERREBOOT,	// 需要程序重启，任务才执行
//     AUTOTASK_RUNNING_STATUS_BASEDONEXETERNALPROG,	// 依赖的外部程序并没有运行
//     //////////////////////////////////////////////////////////////////////////
//     // 绝对时间
//     AUTOTASK_RUNNING_STATUS_TIMENOTMATCH,	// 无可执行的时间匹配
//     AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC,	// 虽然任务未过期，但是余下的时间里，任务都没有机会再执行了
// };

//
// // 任务优先级
// enum ENUM_TASK_PRIORITY
// {
// 	TASK_PRIORITY_LOWEST = 1,	// 最低
// 	TASK_PRIORITY_LOW = 2,		// 低
// 	TASK_PRIORITY_NOMAL = 3,	// 正常
// 	TASK_PRIORITY_HIGH = 4,		// 高
// 	TASK_PRIORITY_HIGHEST = 5,	// 最高优先级
// };
//

class ExpTimer;

enum ENUM_TIMER_FLAG {
    TIMER_FLAG_REMINDERDISABLED = 0x00000001,	// 启用提示定时器
};

mstring getRunningStatusDescription(ExpTimerRunningStatus eStatus);
mstring getExecFlagText(ExpTimerExecFlag eFlag);

class CronMan;
class CronTimer {
    friend class CronMan;
    enum ENUM_TIMER_TYPE {
        TIMER_TYPE_NOTSET = 0,
        TIMER_TYPE_RELATE,
        TIMER_TYPE_ABSOLUTE,
    };

public:
    CronTimer(void);

    CronTimer(int nID,dt::time tmBegin,dt::time tmEnd,
              const mstring& szWhen,const mstring& szReminder,const mstring& szXFiled);

    ~CronTimer(void);

    // 更新数据
    bool update(dt::time tmB, dt::time tmEnd, const mstring& szExpWhen, const mstring& szExpRemind, const mstring& szExpXFiled);

    // 定时器是否已经启动
    bool started()const {
        return false;
    }

    bool stop();

    ExpTimerRunningStatus start(int nTaskID);

    bool reminderEnabled()const {
        return !(timer_flag_ & TIMER_FLAG_REMINDERDISABLED);
    }

    bool enableReminder( int nTaskID,bool bEnable=true );

    int32_t ID()const {
        return 1;
    }
protected:
    mstring xFiled()const {
        return xfield_;
    }

private:
    mstring			xfield_;	//
    uint32_t			timer_flag_;

    ReminderExp* reminder_;
    ExpTimer* timer_ = nullptr;
};
