
#include "qhelper.h"

namespace cron {

uint32_t QHelper::HowManySeconds( int nX,WCHAR cUnit ) {
    switch (cUnit) {
    case L's':
    case L'S':
        return nX;
    case L'm':
    case L'M':
        return nX * 60;
    case L'h':
    case L'H':
        return nX * 3600;
    default:
        ASSERT(false);
        return 0;
    }
}

mstring QHelper::GetTimeUnitString(WCHAR cUnit) {
    switch (cUnit) {
    case L's':
    case L'S':
        return "秒";
    case L'm':
    case L'M':
        return "分";
    case L'h':
    case L'H':
        return "时";
    default:
        ASSERT(false);
        return "无效单位";
    }
}

mstring QHelper::MakeReleateExp( ExpTimerExecFlag exec,
                                 int span, wchar_t unit, /* 相对于eExec?定时间 */
                                 int span1 /*= 0*/, wchar_t unit1 /*= L's'*/, /* ?后间隔， 0 为无间隔 */
                                 int exec_count /*= 0*/ ) {
    mstring exp;
    // R=1;P=20m;Q=22m;C=100;
    // R
    exp.Format("R=%d;", exec);

    //P - span1
    exp.AppendFormat("P=%d%c;", span, unit);

    // Q - span2
    if (span1 > 0) {
        // span2
        exp.AppendFormat("Q=%d%c;",span1, unit1);
        // C - exec count
        if (exec_count > 0) {
            exp.AppendFormat("C=%d;",exec_count);
        }
    }
    return exp;
}

bool QHelper::ParseInt(__in const CStdStringW& src, __out int& nT) {
    nT = std::stoi(src);
    return true;
}

bool QHelper::MakeAbsExp(ExpTimerExecFlag flag,dt::time& tmB, dt::time& tmE,
                         StringArray& arDatePots, StringArray& arTimePots,
                         __out mstring& sResultExp, __out mstring& sError) {
    if (arTimePots.empty() || arDatePots.empty()) {
        ASSERT(false);
        sError = "必须至少指定一个日期和一个时间";
        return false;
    }

    // 先获取时间点
    mstring sTimePots;
    std::vector<uint32_t> vExist;
    for (unsigned int i = 0; i < arTimePots.size(); ++i) {
        dt::time tTime;
        if (!dt::parse(tTime, arTimePots[i], dt::ParseFlag::kFlagTime)) {
            sError = "执行时间点错误";
            return false;
        }
        uint32_t dwT = dt::make_uint_time(tTime);
        if (std::find(vExist.begin(), vExist.end(), dwT) == vExist.end()) {
            vExist.push_back(dwT);
            sTimePots.AppendFormat("%u,", dwT);
        }
    }
    // 去掉最后一个字符','
    sTimePots.TrimRight(L',');

    // 日期部分
    if (AUTOTASK_EXEC_ATDATE == flag) {
        // 单个日期和时间点执行
        dt::time tmNow = dt::now();
        dt::time tDate;
        dt::parse(tDate, arDatePots[0]);
        if (dt::compare_date(tDate, tmNow) < 0) {
            sError = "执行日期已经过去了";
            return false;
        }
        // 自动调整任务的生命期为合适的时间
        if ((tmB >= tmE) || (tmE <= tmNow) || (tmB <= tmNow)
                || (tmB > tDate) || (tmE < tDate)) {
            tmB = tmNow;
            tmE = tDate + date::days(1);
        }
        sResultExp.Format("A=%d;X=%u;T=%s;",AUTOTASK_EXEC_ATDATE, dt::make_uint_date(tDate), sTimePots);
        return true;
    } else if (AUTOTASK_EXEC_ATYEARDAY == flag) {
        // 像生日一样，有提前量
        if (arDatePots.size() != 2) {
            sError = "按年执行     参数只能有2个（[0]:执行日期，[1]:提前量）";
            return false;
        }

        sResultExp.Format("A=%d;Q=%d;X=%s;T=%s;",AUTOTASK_EXEC_ATYEARDAY,
                          std::stol(arDatePots[1]), arDatePots[0], sTimePots);
        return true;
    } else if (AUTOTASK_EXEC_ATDAILY == flag) {
        // 每隔x天执行一次
        if (arDatePots.size() > 1) {
            sError = "按日间隔执行   参数太多";
            return false;
        }
        int nXDay;
        if (!QHelper::ParseInt(arDatePots[0], nXDay)) {
            sError = "按日间隔执行   参数解析错误      应该指定一个整数间隔";
        }
        if ((nXDay < 1) || (nXDay > 30)) {
            sError = "按日间隔执行   间隔范围应该在[1,30]之间";
            return false;
        }
        sResultExp.Format("A=%d;P=%u;T=%s;", AUTOTASK_EXEC_ATDAILY, nXDay, sTimePots);
        return true;
    } else if (AUTOTASK_EXEC_ATWEEKDAY == flag) {
        // 周几
        int v, weeks = 0;
        for (unsigned int i = 0; i < arDatePots.size(); ++i) {
            if (!QHelper::ParseInt(arDatePots[i], v) || (v > 6)) {
                sError = "按周执行   参数解析错误    应该指定一个[0, 6]的整数";
                return false;
            }
            weeks |= (0x1<<v);
        }
        if (0 == weeks) {
            sError = "按周执行   参数解析错误    应该至少指定一个[0, 6]的整数";
            return false;
        }
        sResultExp.Format("A=%d;X=%u;T=%s;",AUTOTASK_EXEC_ATWEEKDAY, weeks, sTimePots);
        return true;
    } else if (AUTOTASK_EXEC_ATMONTHDAY == flag) {
        int d, days = 0;
        for (unsigned int i = 0; i < arDatePots.size(); ++i) {
            if (!QHelper::ParseInt(arDatePots[i], d) || (d > 31)) {
                sError = "按日执行   参数解析错误    参数范围[0, 31], 0 - 每日都执行";
                return false;
            } else if (0 == d) {
                days = 0x0fffffff;  // 全日执行
                break;
            }
            days |= (0x1 << d);
        }
        if (0 == days) {
            sError = "按日执行   参数解析错误    应该至少指定一个[0, 31]的整数,  0 - 每日都执行";
            return false;
        }
        sResultExp.Format("A=%d;X=%u;T=%s;",AUTOTASK_EXEC_ATMONTHDAY,  days, sTimePots);
        return true;
    }
    sError = "绝对表达式    执行参数错误";
    return false;
}

mstring QHelper::MakeRemindExp( int nA, WCHAR cAUnit,
                                const mstring&sSound,const mstring&sMsg ) {
    cAUnit = tolower(cAUnit);
    if ((nA <= 0) || ((cAUnit != L's') && (cAUnit != L'm') && (cAUnit != L'h')))
        return "";
    mstring sRet;
    sRet.Format("A=%d%c;\nS=%s;\nM=%s;\n",nA,cAUnit,sSound,sMsg);
    return sRet;
}

bool QHelper::ParseUnitTime(__in const CStdStringW& src, __out int &nT, __out wchar_t& cUnit) {
    CStdStringW tmp = src;
    if (tmp.Trim().IsEmpty())
        return false;

    cUnit = tolower(tmp.back());
    if ( (L's' != cUnit) && (L'm' != cUnit) && (L'h' != cUnit))
        return false;
    tmp.pop_back();

    return QHelper::ParseInt(tmp, nT);
}
}
