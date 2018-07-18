#pragma once

#include "ExpTimer.h"


struct ReminderParam {
    int		nSeconds; // 提前多少秒提醒
    //int		nTaskID; // 任务id
    void*	data;
    dt::time	tmExec; // 任务的执行时间
    mstring sSound;	// 声音文件路径
    mstring sMsg; // 提示信息
};

class ReminderExp : public ExpTimer {
public:
    ReminderExp();
    ~ReminderExp();

    bool setExp(const mstring& exp);

    mstring remindExp()const {
        return exp_;
    }

    // 执行任务前的提示表达式
    // 格式：
    // A=15[s|h|m];\nsound=path;\nmsg=message;\n
    // A=15[s|h|m] 执行任务前15[秒，分，时]提示
    // S=path; path 为声音文件路径 ,空为不播放声音
    // M=message; messsage 为自定义消息，空为无自定义消息
    bool parse(const mstring&sExp, __out int &nA,
               __out wchar_t&cAUnit, __out mstring&sSound, __out mstring&sMsg);

    bool getRemindString(__out mstring& sReminderDes);

    bool setRemindTimer(const dt::time& t_exe, OnTimeoutCallback cb, void* d);

private:
    // 提示表达式
    mstring exp_;

    // 任务提示的数据
    ReminderParam   trp_;
};

