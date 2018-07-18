
#include "reminderexp.h"


namespace cron {

ReminderExp::ReminderExp() {
}

ReminderExp::~ReminderExp() {
}

bool ReminderExp::setExp(const mstring& exp) {
    exp_ = exp;
    return true;
}

bool ReminderExp::parse(const mstring&sExp, __out int &nA,
                        __out wchar_t&cAUnit, __out mstring&sSound, __out mstring&sMsg) {
    wchar_t cProp;
    mstring sPart, sValue, sTemp = sExp;
    int idx;
    while (true) {
        idx = sTemp.Find(";\n");
        if (-1 == idx)
            return false;
        sPart = sTemp.Left(idx + 1);
        sTemp = sTemp.Mid(idx + 2);
        if (!_parse_prop_val(sPart, cProp, sValue))
            return false;
        switch (cProp) {
        case L'A': {
            if (!helper::parseUnitTime(sValue, nA, cAUnit) || (nA <= 0))
                return false;
            break;
        }
        case L'S': {	// sound
            sSound = sValue;
            break;
        }
        case L'M': {
            sMsg = sValue;
            break;
        }
        default:
            return false;
        }
        if (sTemp.IsEmpty())
            break;
    }
    return true;
}

bool ReminderExp::getRemindString(__out mstring& sReminderDes) {
    int nA;
    wchar_t cUnit;
    mstring sSound, sMsg;
    if (parse(exp_, nA, cUnit, sSound, sMsg)) {
        sReminderDes.Format(
            "在执行前:%d %s. 播放声音:%s. 提示消息:%s",
            nA, helper::timeUnitStr(cUnit),
            sSound.IsEmpty() ? "无" : sSound,
            sMsg.IsEmpty() ? "无" : sMsg);
        return true;
    }
    return false;
}

// 此函数执行流程让人容易迷糊。
// 可以在纸上画一条横坐标，标出来3个时间点来清晰思路：提示时间，任务执行时间，现在时间
// 移动现在时间就明白啦
bool ReminderExp::setRemindTimer(const dt::time& t_exe, OnTimeoutCallback cb, void* d) {
    dt::time t_now = dt::now();
    if (dt::total_seconds(t_exe - t_now) < 5) {
        // 距离任务执行时间太短了，就不提示了。
        return false;
    }

    wchar_t unit = '\0';
    int nA = 0;
    mstring sound, msg;
    if (!parse(exp_, nA, unit, sound, msg)) {
        return false;
    }

    trp_.nSeconds = helper::howManySecs(nA, unit);
    ASSERT(trp_.nSeconds > 0);
    trp_.data = d;
    trp_.sSound = sound;
    trp_.sMsg = msg;
    trp_.tmExec = t_exe;

    // 何时应该提示时间
    dt::time tmRemind = t_exe - dt::secs(trp_.nSeconds);
    if (t_now > tmRemind) {
        // 已经过了提示时间了
        // 应该立即提示
        trp_.nSeconds = dt::total_seconds(t_exe - t_now);   // 距离任务执行时间有多少秒

        if (cb) {
            cb(&trp_);
        }
        return true;
    }

    // 还没有到提示时间
    // 具体还有多少秒去提示呢？
    uint32_t dwSecToRemind = dt::total_seconds(tmRemind - t_now);
    if (dwSecToRemind < 5) {
        // 距离提示时间还有5秒，太短啦，直接显示提示框吧
        // 但是倒计时应该加上这段时间
        trp_.nSeconds += dwSecToRemind;
        if (cb) {
            cb(&trp_);
        }
        return true;
    }

    trp_.nSeconds = helper::howManySecs(nA, unit);
    // 如果距离该提示的时间还足够长，那么需要创建一个定时器回调函数
    // 当回调发生时再向窗口发消息，通知显示提示窗口
    // 创建单次定时器来执行提示回调

    // 单次定时器
    return setTimer(dwSecToRemind * 1000, 0, cb, &trp_);
}
}
