#pragma once

#include <iostream>
#include <vector>
#include "console.h"
#include "cron/defs.h"
#include "cron/utils.h"

using namespace std;
using namespace cron;

namespace con = JadedHoboConsole;

enum ENUM_AUTOTASK_DOWHAT {
	AUTOTASK_DO_NOTSET = 0,	// 未设置
	AUTOTASK_DO_REMIND = 2, // 提示信息
	AUTOTASK_DO_EXECPROG = 3,  // 执行程序
	AUTOTASK_DO_SYSSHUTDOWN = 4,  // 关机
	AUTOTASK_DO_SYSREBOOT = 5,  // 重启
	AUTOTASK_DO_BREAKAMOMENT = 6, // 休息一会儿
};

class QArgvParser {
    typedef StringArray::iterator StrArrayItr;

public:
    // 第一步：Set
    bool SetArgv(_TCHAR* argv[], int nArgc) {
        if (nArgc < 3) {
            m_arErrors.push_back("参数太少！");
            return false;
        }

        m_args.clear();

        // 从第二个参数进行记录
        mstring sArgv;
        for (int i = 1; i < nArgc; i++) {
            sArgv = argv[i];
            sArgv.Trim();
            if (!sArgv.IsEmpty()) {
                m_args.push_back(mstring(argv[i]).Trim());
            }
        }

        if (m_args.size() < 2) {
            m_arErrors.push_back("参数太少！");
            return false;
        }
        m_arErrors.clear();
        return true;
    }

    // 第二步：解析
    bool ParseArgs(__out ENUM_AUTOTASK_DOWHAT& eDoWhat, __out mstring& sDoWhatParam,
                   __out mstring& sWhenDo, __out mstring& sRemindExp,
                   __out dt::time& tmBegin, __out dt::time& tmEnd) {
        bool bOK = false;
        do {
            // 做什么
            eDoWhat = GetDoWhat(sDoWhatParam);
            if (AUTOTASK_DO_NOTSET == eDoWhat)
                break;

            // 何时做
            if (!GetExecTime(sWhenDo, tmBegin, tmEnd))
                break;

            // 关于任务提示
            if (!GetTaskRemind(sRemindExp))
                break;

            bOK = true;
        } while (false);

        return bOK;
    }

    /** 在控制台输出错误
     *	params: none
    **/
    void EchoErrors() {
        std::wcout<<con::bg_black<<con::fg_red;
        for (auto i = m_arErrors.begin(); i != m_arErrors.end(); ++i) {
            std::wcout<<*i<<std::endl;
        }
        std::wcout<<con::fg_white;
    }

#ifdef _DEBUG
    void TestString(const CStdString& src) {
        StringArray ar;
        SplitString(src, ar);

        std::wcout<<src<<" :  size: "<<ar.size()<<"\n------------"<<endl;
        for (auto i = ar.begin(); i != ar.end(); ++i) {
            std::wcout<<*i<<std::endl;
        }
        std::wcout<<"-----------------"<<endl;
    }

    // 测试代码
    void TestSplitString() {
        TestString("a;b;c");

        TestString("a;;c");

        TestString("a;b;");

        TestString(";b;c");

        TestString(";b;");

        TestString(";;");
    }
#endif

protected:
    /** 获取执行命令
     *	return:
     *      AUTOTASK_DO_NOTE    无效命令或help命令。
    **/
    ENUM_AUTOTASK_DOWHAT GetDoWhat(__out mstring& sDoWhat) {
        assert(m_args.size() >= 2);

        ENUM_AUTOTASK_DOWHAT eRet = AUTOTASK_DO_NOTSET;

        StringArray::iterator iB = m_args.begin();
        mstring s1 = *iB;
        m_args.erase(iB);

        bool bNeedParam = false;
        if (s1.CompareNoCase("-m") == 0) {
            bNeedParam = true;
            eRet = AUTOTASK_DO_REMIND; // remind message
        } else if (s1.CompareNoCase("-e") == 0) {
            bNeedParam = true;
            eRet = AUTOTASK_DO_EXECPROG;
        } else if (s1.CompareNoCase("-s") == 0) {
            eRet = AUTOTASK_DO_SYSSHUTDOWN;
        } else if (s1.CompareNoCase("-b") == 0) {
            eRet = AUTOTASK_DO_BREAKAMOMENT;
        }

        if (AUTOTASK_DO_NOTSET  == eRet) {
            m_arErrors.push_back("无效的任务执行类型！");
        } else if (bNeedParam) {
            iB = m_args.begin();
            mstring s2 = *iB;
            m_args.erase(iB);

            if (!IsCommand(s2)) {
                sDoWhat = s2;
            } else {
                mstring sTmp;
                sTmp.Format("命令 %s 缺少参数", s1);
                m_arErrors.push_back(sTmp);
                eRet = AUTOTASK_DO_NOTSET;
            }
        }

        return eRet;
    }

    /** 任务提示
     *	return:
     *      TRUE    搞定
     *	params:
     *		-[out]
     *      sRemind
    **/
    bool GetTaskRemind(__out mstring& sRemind) {
        // -w
        StrArrayItr i = FindX("-w");
        if (_End() == i) {
            return true;
        }

        // 下一个就是参数
        ++i;
        if (_End() == i) {
            m_arErrors.push_back("-w   任务提示    未指定参数");
            return false;
        }

        StringArray arP;
        if (SplitString(*i, arP) != 3) {
            m_arErrors.push_back("-w   任务提示    参数个数错误：" + *i);
            return false;
        }

        // 时间
        int nT;
        char unit;
        if (!helper::parseUnitTime(arP[0], nT, unit)) {
            m_arErrors.push_back("-w   任务提示    提示时间错误：" + *i);
            return false;
        }
        // 提示时间不能过长
        DWORD nSecs = helper::howManySecs(nT, unit);
        if ((nSecs > 86400) || (nSecs < 5)) {
            m_arErrors.push_back("-w   任务提示    提示时间范围[10s, 24h)：" + *i);
            return false;
        }

        sRemind = helper::makeRemindExp(nT, unit, arP[1], arP[2]);
        return !sRemind.IsEmpty();
    }

    /** 任务的执行时间
     *	return:
     *      TRUE    解析成功
     *	params:
     *		-[out]
     *      sTime   解析过后的表达式，能够直接被QTimer使用
    **/
    bool GetExecTime(__out mstring& sTime, __out dt::time& tmBegin, __out dt::time& tmEnd) {
        if ( !GetLifePeriod(tmBegin, tmEnd) ) {
            return false;
        }

        // 检查相对时间
        StrArrayItr iR = FindX("-r");
        StrArrayItr iA = FindX("-a");
        if ((_End() != iR) && (_End() != iA)) {
            m_arErrors.push_back("-r/-a    执行时间    只能指定一个执行时间");
            return false;
        }

        if ( (_End() == iA) && (_End() == iR)) {
            m_arErrors.push_back("-r/-a    未设定执行时间");
            return false;
        }

        if (_End() != iA) {
            // 绝对时间
            // 下一个就是参数
            ++iA;
            if (_End() == iA) {
                m_arErrors.push_back("-a   执行时间（绝对时间）    未指定参数");
                return false;
            }
            return ParseAbsTime(*iA, tmBegin, tmEnd,sTime);
        }

        if (_End() != iR) {
            // 下一个就是参数
            ++iR;
            if (_End() == iR) {
                m_arErrors.push_back("-r   执行时间（相对时间）    未指定参数");
                return false;
            }
            return ParseRelateTime(*iR, sTime);
        }

        return false;
    }

    /** 判断是否是命令，以英文'-'开头的是Command
     *	return:
     *      TRUE    是
     *	params:
     *		-[in]
     *          sText
    **/
    inline bool IsCommand(__in const CStdString& sText) {
        assert(sText.size() > 1);
        return (sText[0] == '-');
    }

    /** 任务有效期
     *	return:
     *      TRUE    成功
    **/
    bool GetLifePeriod(__out dt::time& tmBegin, __out dt::time& tmEnd) {
        // -l
        StrArrayItr i = FindX("-");
        if (_End() == i) {
            tmBegin = dt::now();
            // 执行有效期：10年！够长了吧。
            tmEnd = tmBegin + date::years(10);
            return true;
        }

        // 下一个就是参数
        ++i;
        if (_End() == i) {
            m_arErrors.push_back("-l   任务有效期    未指定参数");
            return false;
        }
        StringArray arP;
        if (SplitString(*i, arP) != 2) {
            m_arErrors.push_back("-l   任务有效期   参数错误：" + *i);
            return false;
        }

        if (arP[0].IsEmpty()) {
            tmBegin = dt::now();
        } else if (!dt::parse(tmBegin, arP[0])) {
            m_arErrors.push_back("-l   任务有效期   开始时间错误：" + *i);
            return false;
        }

#ifdef _DEBUG
        std::cout<<"开始时间:"<<dt::format_dt(tmBegin);
#endif

        if (arP[1].IsEmpty()) {
            tmBegin += date::years(10);
        } else if (!dt::parse(tmEnd, arP[1])) {
            m_arErrors.push_back("-l   任务有效期   结束时间错误：" + *i);
            return false;
        }

#ifdef _DEBUG
        std::cout<<"开始时间:"<< dt::format_dt(tmEnd);
#endif

        if (tmBegin >= tmEnd) {
            m_arErrors.push_back("-l   任务有效期   开始时间大于结束时间：" + *i);
            return false;
        }
        return true;
    }

    bool ParseAbsTime(__in const mstring& src, __inout dt::time& tmBegin,
                      __inout dt::time& tmEnd, __out mstring& sTime) {
        // abs time
        assert(tmEnd > tmBegin);

        StringArray arP;
        if (SplitString(src, arP) != 2) {
            m_arErrors.push_back("-a   执行时间（绝对时间）    参数错误：" + src);
            return false;
        }

        // 日期点；时间点
        StringArray aS;
        if (SplitString(arP[0], aS, ':') != 2) {
            m_arErrors.push_back("-a   执行时间（绝对时间）    参数错误: " + arP[0]);
            return false;
        }

        TimerExecType eFlag;
        // aS[0] 只能有3种可能：p,s,t
        if (aS[0].CompareNoCase("d") == 0) {
            // littlet 启动后
            eFlag = TimerExecType::kAtDate;
        } else if (aS[0].CompareNoCase("s") == 0) {
            eFlag = TimerExecType::kAtDaily;
        } else if (aS[0].CompareNoCase("w") == 0) {
            eFlag = TimerExecType::kAtWeekday;
        } else if (aS[0].CompareNoCase("m") == 0) {
            eFlag = TimerExecType::kAtMonthDay;
        } else {
            m_arErrors.push_back("-a   执行时间（绝对时间）    无效参数错误：" + src);
            return false;
        }
        StringArray arDatePots;
        SplitString(aS[1], arDatePots, ',');

        StringArray arTimePots;
        if (SplitString(arP[1], arTimePots, ',') <= 0) {
            m_arErrors.push_back("-a   执行时间（绝对时间）    执行时间点无效：" + src);
            return false;
        }
        mstring sError;
        if ( !helper::makeAbsTimerExp(eFlag, tmBegin, tmEnd, arDatePots, arTimePots, sTime, sError )) {
            m_arErrors.push_back(sError);
            return false;
        }
        return true;
    }

    bool ParseRelateTime(__in const mstring& src, __out mstring& sTime) {
        StringArray arP;
        if (SplitString(src, arP) != 3) {
            m_arErrors.push_back("-r   执行时间（相对时间）    参数错误：" + src);
            return false;
        }

        // 相对于；间隔；执行次数
        StringArray aS;
        if (SplitString(arP[0], aS, ':') != 2) {
            m_arErrors.push_back("-r   执行时间（相对时间）    参数错误: " + src);
            return false;
        }

        TimerExecType eFlag;
        // aS[0] 只能有3种可能：p,s,t
        if (aS[0].CompareNoCase("p") == 0) {
            // littlet 启动后
            eFlag = TimerExecType::kAfterAppStart;
        } else if (aS[0].CompareNoCase("s") == 0) {
            eFlag = TimerExecType::kAfterSysBoot;
        } else if (aS[0].CompareNoCase("t") == 0) {
            eFlag = TimerExecType::kAfterTimerStart;
        } else {
            m_arErrors.push_back("-r   执行时间（相对时间）    参数错误：" + src);
            return false;
        }
        int nSpan = 0;
        char unit = 's';
        if (!helper::parseUnitTime(aS[1], nSpan, unit)) {
            m_arErrors.push_back("-r   执行时间（相对时间）    参数错误：" + src);
            return false;
        }

        // 间隔和执行次数
        int nSpan1 = 0, nExecCount = 0;
        char cUnit1 = 's';
        if (!arP[1].IsEmpty()) {
            if (!helper::parseUnitTime(arP[1], nSpan1, cUnit1)) {
                m_arErrors.push_back("-r   执行时间（相对时间）    参数错误：" + src);
                return false;
            }
            if (!arP[2].IsEmpty()) {
                if (!helper::parseInt(arP[2], nExecCount)) {
                    m_arErrors.push_back("-r   执行时间（相对时间）    参数错误" + src);
                    return false;
                }
            }
        }
        sTime = helper::makeRelateTimerExp(eFlag, nSpan, unit, nSpan1, cUnit1, nExecCount);
        return true;
    }

    inline StrArrayItr _End() {
        return m_args.end();
    }

    StrArrayItr FindX(const mstring& sP ) {
        StrArrayItr iEnd = _End();
        for (StrArrayItr i = m_args.begin(); i != iEnd; ++i) {
            if (sP == *i) {
                return i;
            }
        }
        return iEnd;
    }

    /** 分割字符串到数组中
     *	return:
     *      分割的字符串个数，分割的个数由chSep的个数决定
     *      分割的个数为 COUNT(chSep) + 1
     *	params:
     *		-[in]
     *          src     源字符串
     *          chSep   分割符号
     *		-[out]
     *          result  结果
    **/
    int SplitString(__in const mstring& src, __out StringArray& result,
                    __in const char chSep = ';') {
        result.clear();

        mstring tmp = src;
        if (tmp.Trim().IsEmpty()) {
            return 0;
        }
        // 最后一个是 分隔符的话，后面的算作空的字符串
        bool bPushEmpty = (tmp.back() == chSep);
        do {
            int i = tmp.Find(chSep);
            if (-1 == i) {
                result.push_back(tmp);
                break;
            }
            result.push_back(tmp.Left(i));
            tmp = tmp.Mid(i + 1);
        } while ( !tmp.IsEmpty() );

        if (bPushEmpty) {
            result.push_back(mstring());
        }
        return result.size();
    }

private:
    StringArray    m_args;
    StringArray    m_arErrors;
};

