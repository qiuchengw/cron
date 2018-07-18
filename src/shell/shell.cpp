// LittleTShell.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ArgParser.h"
#include "LittleTcmn.h"
#include "..\cron\cron_man.h"

#ifdef APP_NAME
#define APP_NAME "LittleTShe"
#endif

#pragma comment(lib, "cron.lib")

mstring QUIGetAppName() {
    return "LittleTShe";
}

void ShowHelp() {
//    wcout<<L"LittleTShell (-m|-e|-s|-b) [-w] (-r|-b) [-i]";
    wcout<<con::fg_white
         <<L"LittleTShell 参数解释 \n"
         <<con::fg_green
         <<L"    -h      显示帮助\n"
         <<con::fg_white
         <<endl;
    wcout <<L"[1] 任务类别（必填项）：\n"
          <<L"-------------------------------------------------------------\n"
          <<con::fg_green
          <<L"    -m  \"message\"         提示信息\n"
          <<L"    -e  \"文件路径\"        执行程序\n"
          <<L"    -s  关机（无参数）\n"
          <<L"    -b  休息一会儿（无参数）\n"
          <<con::fg_white
          <<endl;
    wcout<<L"[2] 任务执行时间（必填项）\n"
         <<L"-------------------------------------------------------------\n"
         <<con::fg_green
         <<L"    -r  \"相对于;间隔时间;执行次数\" \n"
         <<con::fg_gray
         <<L"        相对于[p|s|t]\n"
         <<L"            p   LittleT启动后  eg: p:20m; LittleT启动后20分\n"
         <<L"            s   系统启动后     eg: s:30s; 操作系统启动后30秒\n"
         <<L"            t   任务创建后     eg: t:1h;  任务创建后1小时\n"
         <<L"\n"
         <<L"        间隔时间（可选）   n[h|s|m]\n"
         <<L"            例如: 20m;     每隔20分执行一次\n"
         <<L"\n"
         <<L"        执行次数（可选）  \n"
         <<L"            例如: 30       执行30后停止。\n"
         <<L"\n"
         <<con::fg_cyan
         <<L"    例如： -r \"s:30m;1h;100\"     在系统启动后30分执行任务，然后每隔1小时执行一次，执行100次后结束。"
         <<L"\n"
         <<endl;
    wcout<<con::fg_green
         <<L"    -a  \"绝对日期;绝对时间点\"\n"
         <<con::fg_gray
         <<L"        绝对日期      [d|s|w|m]\n"
         <<L"            d:date     单个日期        eg: d:2013/5/5  在2013/5/5执行\n"
         <<L"            s:n        每几天          eg: s:1       每一天都执行\n"
         <<L"            w:1,2...   星期几          eg: w:1,3    每周1，3执行。0代表星期日\n"
         <<L"            m:1,2...   每月的几号       eg: m:1,3    每月的1，3号执行。\n"
         <<L"\n"
         <<L"        绝对时间点，多个时间点以英文','分割\n"
         <<L"            例如： 10:20,11:30:25      在符合日期的10:20和11:30:25执行\n"
         <<L"\n"
         <<con::fg_cyan
         <<L"    例如：-b \"w:0,6;8:00,11:00\" 在周六、日的8:00和11:00执行"
         <<endl;
    wcout<<con::fg_white
         <<L"\n[3] 任务有效期（可选项）"
         <<L"\n-----------------------------------------------------------\n"
         <<con::fg_green
         <<L"    -l  \"开始时间;结束时间\"\n"
         <<con::fg_gray
         <<L"        开始时间    eg: 2013/5/5 10:00:00     如果不填写，默认从当前的系统时间开始\n"
         <<L"        结束时间    eg: 2013/6/2 10:00:00     如果不填写，默认到开始时间的3年后\n"
         <<con::fg_cyan
         <<L"\n"
         <<L"    例如：-l \"2013/5/5; 2013/6/5\"    有效期为：2013/5/5 0:0:1 到 2013/6/5 23:59:59\n"
         <<endl;
    wcout<<con::fg_white
         <<L"[4] 任务提示（可选项）：\n"
         <<L"-------------------------------------------------------------\n"
         <<con::fg_green
         <<L"    -w  \"时间;声音;消息\"\n"
         <<con::fg_gray
         <<L"        时间是必填项，声音和消息可同时或至少填一个\n"
         <<L"        选项之间以英文';'分割，不指定可以默认为空\n"
         <<L"        时间      n[s|m|h]        eg:20m\n"
         <<L"        声音      播放的声音      eg:C:\\dir\\sound.wav\n"
         <<L"        消息      提示的消息      eg:订餐不？\n"
         <<L"\n"
         <<con::fg_cyan
         <<L"    例如： -w \"20m;;即将关机\" 在每次任务执行前20分钟提示\"即将关机\"\n"
         <<con::fg_white
         <<endl;
    wcout<<con::fg_magenta
         <<L"[5] 举俩栗子：\n"
         <<L"-------------------------------------------------------------\n"
         <<con::fg_cyan
         <<L"LittleTShell -s -w \"5m;;即将关机\" -a \"s:1;22:00\" -l \";2015/6/5\" \n"
         <<con::fg_gray
         <<L"    表示：从现在到2015/6/5，每天晚上22:00自动关机，并在关机之前5分钟提示用户\"即将关机!\""
         <<endl;
    wcout<<L"\n"
         <<con::fg_cyan
         <<L"LittleTShell -b -r \"s:1h;1h;\"  \n"
         <<con::fg_gray<<con::bg_black
         <<L"    表示：在使用电脑的过程中，每隔1小时显示一次屏保（不提示，不可结束）"
         <<endl<<endl;
}

BOOL SendRequestToLittleT(ENUM_AUTOTASK_DOWHAT nDoWhat, const mstring& sDoWhatParam,
                          const mstring& sWhenDo, const mstring& sRemindexp, dt::time tmBegin, dt::time tmEnd,
                          __out mstring& sError) {
    // 创建timer
    auto * man = CronMan::instance();
    man->add(tmBegin, tmEnd, sWhenDo, sRemindexp, sDoWhatParam);

    return TRUE;
}

int _tmain(int argc, _TCHAR* argv[]) {
    ::CoInitialize(NULL);

    // 输出中文
    wcout.imbue(locale("chs"));
    wcout.imbue(locale("chs"));

    CronMan::instance()->init();

    // 参数对不对？
    QArgvParser aParser;
    if ( !aParser.SetArgv(argv, argc) ) {
        ShowHelp();
    } else {
        // 解析对不对？
        ENUM_AUTOTASK_DOWHAT eDoWhat;
        mstring sDoWhatParam, sWhenDo,sRemindExp;
        dt::time tmBegin, tmEnd;
        if (aParser.ParseArgs(eDoWhat, sDoWhatParam,
                              sWhenDo, sRemindExp, tmBegin, tmEnd)) {
            // 解析成功，通过管道把数据传给LittleT
            mstring sError;
            if (!SendRequestToLittleT(eDoWhat, sDoWhatParam,
                                      sWhenDo, sRemindExp, tmBegin, tmEnd, sError)) {
                std::wcout<<con::fg_red<<L"任务未创建："<<sError<<con::fg_white<<endl;
            } else {
                std::wcout<<con::fg_red<<L"任务已创建"<<con::fg_white<<endl;
            }
        } else {
            aParser.EchoErrors();
        }
    }

    int ii;
    std::cin >> ii;

    ::CoUninitialize();
    return 0;
}

