#pragma once

#include "cron_timer.h"

typedef std::list<CronTimer*>	TimerList;
typedef TimerList::iterator TimerListItr;

class CronMan : public Singleton<CronMan> {
public:
    ~CronMan() {
		deinit();
    }

    bool init();

    CronTimer* get(int nID);
    CronTimer* add(const dt::time &tmBegin,const dt::time& tmEnd,
                        const mstring& szWhen,const mstring& szReminder,const mstring& szXField);
    bool edit(CronTimer* pTimer,const dt::time &tmBegin,const dt::time& tmEnd,
                   const mstring& szWhen,const mstring& szReminder,const mstring& szXField);
    bool setRemindExp(CronTimer* pTimer,const mstring& pszRmdExp);

    bool destroy( CronTimer *pTimer );
    bool remove( CronTimer* pTimer );

    bool stop(CronTimer* pTimer);
    ExpTimerRunningStatus start(CronTimer *pTimer,int nTaskID);
    bool enableReminder(CronTimer*pTimer,int nTaskID,bool bEnabled);

protected:
    TimerListItr _find(int nID);
    void deinit();

private:

private:
    TimerList		m_lstTimer;
};

