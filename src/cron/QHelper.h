#pragma once

#include <list>
#include <algorithm>
#include <iterator>
#include "defs.h"
#include "crontimer.h"

namespace cron {

class QHelper {
public:
    // s/m/h
    static mstring GetTimeUnitString(WCHAR cUnit);
    static uint32_t HowManySeconds( int nX,WCHAR cUnit );

    static bool ParseInt(__in const CStdStringW& src, __out int& nT);

    static mstring MakeReleateExp( ExpTimerExecFlag eExec,
                                   int nSpan, wchar_t cUnit,   // 相对于eExec一定时间
                                   int nSpan1 = 0, wchar_t cUnit1 = L's', // 然后间隔， 0 为无间隔
                                   int nExecCount = 0); // 总共能执行的次数, 0为不限制

    static bool MakeAbsExp(ExpTimerExecFlag eFlag,dt::time& tmB, dt::time& tmE,
                           StringArray& arDatePots, /* 执行日期点 */ StringArray& arTimePots,
                           __out mstring& sResultExp, __out mstring& sError);

    static mstring MakeRemindExp( int nA, WCHAR cAUnit, const mstring&sSound,const mstring&sMsg );

    /** 10h，10s，10m 这样的时间格式解析
     *	return:
     *      TRUE    数据格式正确
     *	params:
     *		-[in]
     *          src     被解析的字符串
     *		-[out]
     *          nT      数字部分
     *          cUnit   单位部分
    **/
    static bool ParseUnitTime(__in const CStdStringW& src, __out int &nT, __out wchar_t& cUnit);
};
}
