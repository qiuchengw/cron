#pragma once

// 一个定时器实现（时间轮实现）
// 复制自：http://www.cnblogs.com/junye/p/5836552.html

#include <list>
#include <vector>
#include <functional>

//////////////////////////////////////////////////////////////////////////
// TimerManager tm;
// Timer t(tm);
// t.Start(&TimerHandler, 1000);
// while (true)
// {
// 	tm.DetectTimers();
// 	std::this_thread::sleep_for(std::chrono::milliseconds(100));
// }
//////////////////////////////////////////////////////////////////////////

class WheelTimer;
typedef std::function<void(void*)> OnTimeoutCallback;

class WheelTimerManager {
public:
    WheelTimerManager();

    static unsigned long long GetCurrentMillisecs();
    void DetectTimers();

private:
    friend class WheelTimer;
    void AddTimer(WheelTimer* timer);
    void RemoveTimer(WheelTimer* timer);

    int Cascade(int offset, int index);

private:
    typedef std::list<WheelTimer*> TimeList;
    std::vector<TimeList> tvec_;
    unsigned long long checkTime_;
};

class WheelTimer {
public:
	enum TimerType { ONCE, CIRCLE };

	WheelTimer(WheelTimerManager& manager);
	~WheelTimer();

	template<typename Fun>
	void Start(Fun fun, void* data, unsigned interval, TimerType timeType = CIRCLE) {
		Stop();
		interval_ = interval;
		timerFun_ = fun;
		timerType_ = timeType;
		data_ = data;
		expires_ = interval_ + WheelTimerManager::GetCurrentMillisecs();
		manager_.AddTimer(this);
	}

	void Stop();

private:
	void OnTimer(unsigned long long now);

private:
	friend class WheelTimerManager;

	WheelTimerManager& manager_;
	TimerType timerType_;
	OnTimeoutCallback timerFun_;
	void *data_ = nullptr;

	unsigned interval_;
	unsigned long long expires_;

	int vecIndex_;
	std::list<WheelTimer*>::iterator itr_;
};
