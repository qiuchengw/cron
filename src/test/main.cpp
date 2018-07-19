#include <iostream>
#include <chrono>
#include "cron/exptimer.h"

using namespace cron;

class MyTask {
public:
	MyTask() {
		std::cout << "MyTask Instance created:" << dt::format_time(dt::now(), true) << std::endl;
	}

	void doTask() {
		std::cout << "do task:" << dt::format_time(dt::now(), true) <<std::endl;
	}
};

int main() {

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

	MyTask a_task;
	// 启动任务后5s执行一次，然后每隔2s执行一次，共计执行5次后停止
	if (auto t = ExpTimer::create("R=2;P=5s; Q = 2s; C = 5;")) {
		auto tm_now = dt::now();
		t->startFrom(tm_now, [](void *p) {
			reinterpret_cast<MyTask*>(p)->doTask();
		}, &a_task);
	}

	// 不要让这个线程立即退出，以观察Task的执行
	std::this_thread::sleep_for(std::chrono::minutes(1));
}

