#include <iostream>

#include "../../deps/date/include/date/date.h"
#include "../../deps/date/include/date/tz.h"
#include <chrono>

#include "../cron/stdafx.h"
#include "../cron/time.h"

// Timer* test_timers(TimerManager & man, int sec, const mstring& des) {
//     Timer *t = new Timer(man);
//     t->Start([=]() {
//         // std::cout << "des:[" << des << "]-[" << sec << "]fired!" << std::endl;
//     }, sec, Timer::CIRCLE);
//     return t;
// }

int main() {
    using namespace date;

    TimerManager m;
    for (int i = 0; i < 10000; ++i) {
        test_timers(m, i * 10 + 100, "timer1");
    }

//	test_timers(m, 300, "timer3");

    while (true) {
        m.DetectTimers();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

// 	auto d1 = sys_days{ 2017_y / 3 / 4 } +hours{ 23 } +minutes{ 59 } +
// 		seconds{ 59 } +milliseconds{ 999 };
// 	std::cout << d1 << std::endl;
//
// 	typedef sys_time<std::chrono::milliseconds> time_2;
// 	time_2 d2 = sys_days{ 2017_y / 3 / 4 } +hours{ 23 } +minutes{ 59 } +
// 		seconds{ 57 } +milliseconds{ 999 };
// 	std::cout << d2 << std::endl;
//
//     auto t = dt::parse("2015-08-27 10:31:40", dt::ParseFlag::kFlagDateTime);
//     std::cout << t << "  --" << t.time_since_epoch() << std::endl;
//
//     auto tt1 = dt::parse("2015-08-27", dt::ParseFlag::kFlagDate);
//     std::cout << tt1 << std::endl;
//
//     tt1 = dt::parse("10:31:40", dt::ParseFlag::kFlagTime);
//     std::cout << tt1 << std::endl;
//
//     std::cout << dt::format("%D %T", t) << std::endl;
//     std::cout << dt::format_date(t) << std::endl;
//     std::cout << dt::format_time(t) << std::endl << std::endl;
//
//     auto t_now = dt::now();
//     std::cout << dt::format("%D %T", t_now) << std::endl;
//     std::cout << dt::total_seconds(t_now-t) << std::endl << std::endl;
//
//     auto t3 = dt::parse("2015-09-27T10:31:40Z");;
//     std::cout << dt::format("%D %T", t3) << std::endl;
//     std::cout << dt::total_seconds(t3 - t) << std::endl << std::endl;
//
//     auto span_1 = t3 - t;
//     date::days dddd(dt::total_seconds(span_1)/86400);
//     std::cout << dddd;
//
// // 	std::ratio<4, 9> r1;
// // 	std::cout << r1.den << " --- " << r1.num << std::endl << std::endl;
// // 	std::cout << dt::time_ratio::den << "-- " << dt::time_ratio::num << std::endl << std::endl;
//
//     std::chrono::hours h(1);
//     std::cout << t + h << std::endl << std::endl;
//
//     uint32_t dw_t = dt::make_uint_time(t);
//     auto t4 = dt::parse_uint_time(dw_t);
//     std::cout << t4 << std::endl << std::endl;
//
//     uint32_t dw_d = dt::make_uint_date(t_now);
//     auto t5 = dt::parse_uint_time(dw_d);
//     std::cout << t5 << std::endl << std::endl;
//
//     auto t6 = dt::combine_date_time(dw_d, dt::make_uint_time(t_now));
//     std::cout << t6 << std::endl << std::endl;


    int ii;
    std::cin >> ii;
}

