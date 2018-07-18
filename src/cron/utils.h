#pragma once

#include <iostream>
#include <string>
#include <sstream>

#include "deps/date/include/date/date.h"
#include "defs.h"

namespace cron {

namespace dt { // datetime

using clock = std::chrono::system_clock;
using time = std::chrono::system_clock::time_point;
using time_span = time::duration;
using time_ratio = clock::period;

// 	using hours = std::chrono::hours;
// 	using minutes = std::chrono::hours;
using secs = std::chrono::seconds;

inline time now() {
    return clock::now();
}

enum class ParseFlag {
    kFlagDate = 1,
    kFlagTime = 2,
    kFlagDateTime = 3,
};

inline bool parse(time& t, const std::string& str, ParseFlag f = ParseFlag::kFlagDate) {

    std::istringstream in(str);
    date::sys_seconds tp;
    switch (f) {
    case ParseFlag::kFlagDate:
        in >> date::parse("%F", tp);
        break;
    case ParseFlag::kFlagTime:
        // 必须要有 日期字段，否则会解析失败
        in >> date::parse("%F %T", tp);
        if (in.fail()) {
            in.clear();
            // 22:01 --> 1970-1-1 22:01:00
            mstring s;
            if (str.find(" ") == -1) {
                s = "1970-1-1 "; // 加日期
            }
            s += str;
            if (1 == std::count_if(str.begin(), str.end(), [](auto&t) {
            return t == ':';
        })) {
                s.append(":00");
            }

            in.str(s);
            in >> date::parse("%F %T", tp);
        }
        break;
    case ParseFlag::kFlagDateTime:
        in >> date::parse("%F %T", tp);
        break;
    }

    if (in.fail()) {
        return false;
    }

    t = tp;
    return true;
}

// 直接使用date::formate
using date::format;
inline std::string format_dt(const time& t) {
    return date::format("%c", t);
}

inline std::string format_date(const time& t) {
    return date::format("%F", t);
}

inline std::string format_time(const time& t, bool with_seconds = false) {
    if (with_seconds) {
        return date::format("%T", t);
    }
    return date::format("%R", t);
}

inline int64_t total_seconds(const time_span& ts) {
    return ts.count() / (time_ratio::den);
}

inline int64_t total_days(const time_span& ts) {
    return total_seconds(ts) / (24 * 60 * 60);
}

inline std::tm to_calendar_time(const time& tp) {
    auto date = date::floor<date::days>(tp);
    auto ymd = date::year_month_day(date);
    auto weekday = date::year_month_weekday(date).weekday_indexed().weekday();
    auto tod = date::make_time(tp - date);
    date::days daysSinceJan1 = date - date::sys_days(ymd.year() / 1 / 1);

    std::tm result{};
    result.tm_sec = tod.seconds().count();
    result.tm_min = tod.minutes().count();
    result.tm_hour = tod.hours().count();
    result.tm_mday = unsigned(ymd.day());
    result.tm_mon = unsigned(ymd.month());// -1u; // Zero-based!
    result.tm_year = int(ymd.year());// -1900;
    result.tm_wday = unsigned(weekday);
    result.tm_yday = daysSinceJan1.count();
    result.tm_isdst = -1; // Information not available
    return result;
}

inline std::tm __extract_uint_date(uint32_t ddate) {
    std::tm d;
    d.tm_mday = ddate & 0xFF;
    d.tm_mon = (ddate >> 8) & 0xFF;
    d.tm_year = (ddate >> 16);
    d.tm_wday = 0;
    return d;
}

inline std::tm __extract_uint_time(uint32_t dwTime) {
    std::tm t;
    t.tm_hour = ((dwTime >> 24) & 0x1F);
    t.tm_min = ((dwTime >> 16) & 0x3F);
    t.tm_sec = ((dwTime >> 10) & 0x3F);
    // + std::chrono::milliseconds((dwTime & 0x3FF));
    return t;
}

inline uint32_t make_uint_date(const time& ts) {
    std::tm st = to_calendar_time(ts);
    uint32_t r = st.tm_year;
    r <<= 8;
    r |= st.tm_mon;
    r <<= 8;
    r |= st.tm_mday;
    return r;
}

inline uint32_t make_uint_time(const time& t) {
    std::tm st = to_calendar_time(t);
    uint32_t dwTime = (st.tm_hour & 0x1F);
    dwTime <<= 8;
    dwTime |= (st.tm_min & 0x3F);
    dwTime <<= 8;
    dwTime |= (((st.tm_sec & 0x3F) << 2));
    dwTime <<= 8;
    dwTime |= 0;	// 毫秒值为0
    return dwTime;
}

inline time combine_date_time(const std::tm& ymd, const std::tm& hms) {
    auto new_ymd = date::year(ymd.tm_year) / ymd.tm_mon / ymd.tm_mday; // year_month_day type
    if (new_ymd.ok()) {
        return date::sys_days(new_ymd) + std::chrono::hours(hms.tm_hour)
               + std::chrono::minutes(hms.tm_min)
               + std::chrono::seconds(hms.tm_sec)
               + std::chrono::milliseconds(0);
    }
    throw std::runtime_error("Invalid date");
}

// convert date/time from UTC, to time_point
// note: this function does NOT need the tz library and header
inline time parse_uint_time(uint32_t dwTime, const time& ymd = now()) {
    return combine_date_time(to_calendar_time(ymd), __extract_uint_time(dwTime));
}

inline time parse_uint_date(uint32_t ddate, const time& hms = now()) {
    return combine_date_time(__extract_uint_date(ddate), to_calendar_time(hms));
}

inline time combine_date_time(uint32_t ddate, uint32_t dwTime) {
    return combine_date_time(__extract_uint_date(ddate), __extract_uint_time(dwTime));
}

inline time& set_time(time& ymd, int h, int m, int s, int mills = 0) {
    std::tm tm_t = to_calendar_time(ymd);
    auto new_ymd = date::year(tm_t.tm_year) / tm_t.tm_mon / tm_t.tm_mday; // year_month_day type
    if (new_ymd.ok()) {
        ymd = date::sys_days(new_ymd)
              + std::chrono::hours(h)
              + std::chrono::minutes(m)
              + std::chrono::seconds(s)
              + std::chrono::milliseconds(mills);
        return ymd;
    }
    throw std::runtime_error("Invalid date");
}

inline time& set_date(time& hms, int y, int m, int d) {
    auto ymd = date::year(y) / m / d; // year_month_day type
    if (ymd.ok()) {
        std::tm t = to_calendar_time(hms);
        hms = date::sys_days(ymd)
              + std::chrono::hours(t.tm_hour)
              + std::chrono::minutes(t.tm_min)
              + std::chrono::seconds(t.tm_sec)
              + std::chrono::milliseconds(0);
        return hms;
    }
    throw std::runtime_error("Invalid date");
}

inline int compare_date(const time& t1, const time& t2 ) {
    return make_uint_date(t1) - make_uint_date(t2);
}

inline int compare_time(const time& t1, const time& t2) {
    return make_uint_time(t1) - make_uint_time(t2);
}

//
// 	time IgnoreMillSecond()
// 	{
// 		SYSTEMTIME st;
// 		if ((GetStatus() == invalid) && GetAsSystemTime(st))
// 		{
// 			st.wMilliseconds = 0;
// 			m_status = ConvertSystemTimeToVariantTime(st) ? valid : invalid;
// 		}
// 		return *this;
// 	}

}

namespace helper {

inline uint32_t howManySecs(int nX, WCHAR unit) {
    switch (unit) {
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

inline mstring timeUnitStr(WCHAR unit) {
    switch (unit) {
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

inline mstring makeRelateTimerExp(ExpTimerExecFlag exec,
                                  int span, char unit, /* 相对于eExec?定时间 */
                                  int span1 = 0, char unit1 = L's', /* ?后间隔， 0 为无间隔 */
                                  int exec_count = 0) {
    mstring exp;
    // R=1;P=20m;Q=22m;C=100;
    // R
    exp.Format("R=%d;", exec);

    //P - span1
    exp.AppendFormat("P=%d%c;", span, unit);

    // Q - span2
    if (span1 > 0) {
        // span2
        exp.AppendFormat("Q=%d%c;", span1, unit1);
        // C - exec count
        if (exec_count > 0) {
            exp.AppendFormat("C=%d;", exec_count);
        }
    }
    return exp;
}

inline bool parseInt(__in const CStdStringW& src, __out int& nT) {
    try {
        nT = std::stoi(src);
        return true;
    } catch (...) {
        return false;
    }
}

inline bool makeAbsTimerExp(ExpTimerExecFlag flag, dt::time& tmB, dt::time& tmE,
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
        sResultExp.Format("A=%d;X=%u;T=%s;", AUTOTASK_EXEC_ATDATE, dt::make_uint_date(tDate), sTimePots);
        return true;
    } else if (AUTOTASK_EXEC_ATYEARDAY == flag) {
        // 像生日一样，有提前量
        if (arDatePots.size() != 2) {
            sError = "按年执行     参数只能有2个（[0]:执行日期，[1]:提前量）";
            return false;
        }

        sResultExp.Format("A=%d;Q=%d;X=%s;T=%s;", AUTOTASK_EXEC_ATYEARDAY,
                          std::stol(arDatePots[1]), arDatePots[0], sTimePots);
        return true;
    } else if (AUTOTASK_EXEC_ATDAILY == flag) {
        // 每隔x天执行一次
        if (arDatePots.size() > 1) {
            sError = "按日间隔执行   参数太多";
            return false;
        }
        int nXDay;
        if (!parseInt(arDatePots[0], nXDay)) {
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
            if (!parseInt(arDatePots[i], v) || (v > 6)) {
                sError = "按周执行   参数解析错误    应该指定一个[0, 6]的整数";
                return false;
            }
            weeks |= (0x1 << v);
        }
        if (0 == weeks) {
            sError = "按周执行   参数解析错误    应该至少指定一个[0, 6]的整数";
            return false;
        }
        sResultExp.Format("A=%d;X=%u;T=%s;", AUTOTASK_EXEC_ATWEEKDAY, weeks, sTimePots);
        return true;
    } else if (AUTOTASK_EXEC_ATMONTHDAY == flag) {
        int d, days = 0;
        for (unsigned int i = 0; i < arDatePots.size(); ++i) {
            if (!parseInt(arDatePots[i], d) || (d > 31)) {
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
        sResultExp.Format("A=%d;X=%u;T=%s;", AUTOTASK_EXEC_ATMONTHDAY, days, sTimePots);
        return true;
    }
    sError = "绝对表达式    执行参数错误";
    return false;
}

inline mstring makeRemindExp(int nA, WCHAR cAUnit,
                             const mstring&sSound, const mstring&sMsg) {
    cAUnit = tolower(cAUnit);
    if ((nA <= 0) || ((cAUnit != L's') && (cAUnit != L'm') && (cAUnit != L'h')))
        return "";
    mstring sRet;
    sRet.Format("A=%d%c;\nS=%s;\nM=%s;\n", nA, cAUnit, sSound, sMsg);
    return sRet;
}

/** 10h，10s，10m 这样的时间格式解析
*	return:
*      TRUE    数据格式正确
*	params:
*		-[in]
*          src     被解析的字符串
*		-[out]
*          nT      数字部分
*          unit   单位部分
**/
inline bool parseUnitTime(__in const CStdStringW& src, __out int &nT, __out char& unit) {
    CStdStringW tmp = src;
    if (tmp.Trim().IsEmpty())
        return false;

    unit = tolower(tmp.back());
    if ((L's' != unit) && (L'm' != unit) && (L'h' != unit))
        return false;
    tmp.pop_back();

    return parseInt(tmp, nT);
}
}

}
