#include "cronman.h"

namespace cron {

void TaskCallback(void* lpParameter);

// 自动任务提前提示的的回调函数
void TaskRemindCallback(void* lpParameter) {
}

//////////////////////////////////////////////////////////////////////////
bool CronMan::init() {
    return true;
}

CronTimer* CronMan::get( int nTimerID ) {
    auto itr = _find(nTimerID);
    if (timers_.end() != itr)
        return *itr;
    CronTimer* pTimer = nullptr;// QDBEvents::GetInstance()->Timer_Get(nTimerID);
    if (NULL != pTimer) {
        timers_.push_back(pTimer);
    }
    return pTimer;
}

std::list<CronTimer*>::iterator CronMan::_find( int nID ) {
	return std::find_if(timers_.begin(), timers_.end(), [=](CronTimer * t) {
		if (t->ID() == nID) {
			return true;
		}
		return false;
	});
}

bool CronMan::destroy( CronTimer *pTimer ) {
    if (stop(pTimer)) {
//         if (QDBEvents::GetInstance()->Timer_Delete(pTimer->ID())) {
//             TimerListItr itr = _FindTimer(pTimer->ID());
//             if (m_lstTimer.end() != itr) {
//                 delete *itr;
//                 m_lstTimer.erase(itr);
//             }
//             return TRUE;
//         }
    }
    return false;
}

bool CronMan::remove( CronTimer* pTimer ) {
    ASSERT(NULL != pTimer);
    pTimer->stop();
    auto itr = _find(pTimer->ID());
    if (timers_.end() != itr) {
        delete *itr;
        timers_.erase(itr);
    }
    return false;
}

bool CronMan::stop( CronTimer* pTimer ) {
    if (nullptr != pTimer) {
        return pTimer->stop();
    }
    ASSERT(false);
    return true;
}

TimerRunningStatus CronMan::start( CronTimer *pTimer,int nTaskID) {
    if (pTimer != NULL) {
        return pTimer->start(nTaskID);
    }
    return TimerRunningStatus::kBadTimer;
}

CronTimer* CronMan::add( const dt::time &tmBegin,
                         const dt::time& tmEnd, const mstring& szWhen,
                         const mstring& szReminder,const mstring& szXField ) {
//     int nID = QDBEvents::GetInstance()->Timer_Add(
//                   tmBegin,tmEnd,szWhen,szReminder,szXField);
//     if (INVALID_ID != nID) {
    CronTimer *pTimer = new CronTimer(1, tmBegin, tmEnd, szWhen, szReminder, szXField);
    pTimer->start(2);
    timers_.push_back(pTimer);
    return pTimer;
//     }
    return NULL;
}

bool CronMan::edit( CronTimer* pTimer,const dt::time &tmBegin,
                    const dt::time& tmEnd, const mstring& szWhen,
                    const mstring& szReminder,const mstring& szXField ) {
//     if (pTimer->started()) {
//         ASSERT(false);
//         return false;
//     }
//     if (pTimer->update(pTimer->ID(), tmBegin, tmEnd, szWhen, szReminder, szXField)) {
// //         return QDBEvents::GetInstance()->EditTimer(pTimer->ID(),
// //                 tmBegin,tmEnd,szWhen,szReminder,szXField);
//     }
    return false;
}

bool CronMan::setRemindExp( CronTimer* pTimer,const mstring& pszRmdExp ) {
    if ((NULL == pTimer) || (pTimer->started())) {
        ASSERT(false);
        return false;
    }
//     if (pTimer->setRemindExp(pszRmdExp)) {
//         // return QDBEvents::GetInstance()->Timer_SetRemindExp(pTimer->ID(),pszRmdExp);
//     }
    return false;
}

void CronMan::deinit() {
    timers_.clear();
    timer::stop();
}

bool CronMan::enableReminder( CronTimer*pTimer,int nTaskID,bool bEnabled ) {
    if ((NULL == pTimer) || (nTaskID < 0)) {
        ASSERT(false);
        return false;
    }
    return pTimer->enableReminder(nTaskID,bEnabled);
}

}
