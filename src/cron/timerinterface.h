#pragma once

#include <unordered_map>
#include <queue>
#include <atomic>
#include <array>
#include <thread>
#include <mutex>

#include "wheeltimer.h"

namespace cron {

class TimerInterface {
public:
    typedef std::chrono::milliseconds Interval;

    virtual bool start(uint32_t due, uint32_t period, OnTimeoutCallback cb, void* p) = 0;
    virtual bool stop() = 0;
    virtual bool actived()const = 0;
};

class MyTimer : public TimerInterface {
public:
    MyTimer(WheelTimerManager& man) {
        t_ = new WheelTimer(man);
    }

    ~MyTimer() {
        delete t_;
    }

    virtual bool start(uint32_t due, uint32_t period, OnTimeoutCallback cb, void* p) {
        if (!cb || (due == 0 && period == 0)) {
            // 没意义
            return false;
        }

        if (due > 0) {
            // due 一次，然后再执行period
            t_->Start([=](void* that) {
                // 回调一次
                cb(p);

                if (period > 0) {
                    // 再启动一次circle定时器
                    t_->Start(cb, p, period, WheelTimer::CIRCLE);
                }
            }, this, due, WheelTimer::ONCE);
        } else {
            if (period > 0) {
                // 再启动一次circle定时器
                t_->Start(cb, p, period, WheelTimer::CIRCLE);
            }
        }

        return true;
    }

    virtual bool stop() {
        t_->Stop();
        return true;
    }

    virtual bool actived()const {
        return false;
    }

private:
    WheelTimer* t_ = nullptr;
    uint32_t period = 0;
};

template<class TimerT, int ThreadNum, int ResolutionMS>
class MyTimerMan {
    typedef TimerT _TimerTyp;

public:
    MyTimerMan() {
		seq_ = 0;

        for (_TimerThread& t: timer_thread_) {
            t.run();
        }
    }

    // 创建一个Timer
    int timer(uint32_t due, uint32_t period, OnTimeoutCallback cb, void* p) {
        // 放入到某个线程中
        int n = hash_idx(++seq_);
        timer_thread_[n].add(seq_, due, period, cb, p);
        return seq_;
    }

    void stopTimer(int timer_id) {
        int n = hash_idx(timer_id);
        timer_thread_[n].remove(timer_id);
    }

    void stop() {
        for (_TimerThread& t : timer_thread_) {
            t.stop();
        }
    }
protected:
    inline int hash_idx(int n)const {
        return n % ThreadNum;
    }

    struct _TimerThread {

        enum class TimerReqType {
            kReqAdd = 1,
            kReqStop,
            kReqClear,
        };
        struct TimerReq {
            TimerReqType cmd;
            int id;
            uint32_t due;
            uint32_t period;
            OnTimeoutCallback cb;
            void* data;
        };

    public:
		_TimerThread() {
			stop_ = false;
		}

        ~_TimerThread() {
        }

        static void thread_body(_TimerThread *that) {
            that->stop_ = false;
            while (!that->stop_) {
                auto & q = that->req_;
                that->mutx_.lock();
                while (!q.empty()) {
                    TimerReq & req = q.back();
                    switch (req.cmd) {
                    case TimerReqType::kReqAdd:
                        that->_add(req);
                        break;

// 					case TimerReqType::kReqClear:
// 						that->_clear();
// 						break;

                    case TimerReqType::kReqStop:
                        that->_remove(req.id);
                        break;
                    }
                    q.pop();
                }
                that->mutx_.unlock();

                that->wheel_man_.DetectTimers();
                std::this_thread::sleep_for(std::chrono::milliseconds(ResolutionMS));
            }
        }

        void run() {
            if (!th_) {
                th_ = new std::thread(&_TimerThread::thread_body, this);
            }
        }

        void add(int id, uint32_t due, uint32_t period, OnTimeoutCallback cb, void* p) {
            std::lock_guard<std::mutex> g(mutx_);
            TimerReq req;
            req.cmd = TimerReqType::kReqAdd;
            req.id = id;
            req.cb = cb;
            req.data = p;
            req.due = due;
            req.period = period;
            req_.push(req);
        }

        void remove(int id) {
            std::lock_guard<std::mutex> g(mutx_);
            req_.push(TimerReq{ TimerReqType::kReqStop, id });
        }

        void stop() {
            stop_ = true;
        }

    protected:
        _TimerTyp* _add(TimerReq& req) {
            _TimerTyp *t = new _TimerTyp(wheel_man_);
            t->start(req.due, req.period, req.cb, req.data);
            timer_.insert({ req.id, t });
            return t;
        }

        void _remove(int id) {
            auto i = timer_.find(id);
            if (i != timer_.end()) {
                _TimerTyp* t = i->second;
                t->stop();
                timer_.erase(i);
            }
        }

        void _clear() {
            for (auto i = timer_.begin(); i != timer_.end(); ++i) {
                _TimerTyp* t = i->second;
                t->stop();
            }
            timer_.clear();
        }

    protected:
        WheelTimerManager wheel_man_;
        std::unordered_map<int, _TimerTyp*> timer_;
        std::thread *th_ = nullptr;
        std::mutex mutx_;
        std::atomic_bool stop_/* = false*/;

        std::queue<TimerReq> req_;
    };

private:
    // 序列号
    std::atomic_int seq_/* = 0*/;
    std::array<_TimerThread, ThreadNum> timer_thread_;
};

namespace timer {
int set(uint32_t due, uint32_t period, OnTimeoutCallback cb, void* p);
void stop(int id);
void stop();
}

}
