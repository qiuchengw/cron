#pragma once

#include "exptimer.h"

//////////////////////////////////////////////////////////////////////////
// ʱ�䣺[2018/4/11]
//
//  ����ʱ�����ʽ��
//  A = d; S = d; E = d; P = d; T = d1, d2, d3...; X = s;
//
//  A: ִ��ʱ���־ENUM_TASK_EXECFLAG
//
//  	S, E : ��ֹ���ڣ��������Ч��
//
//  	P : ����INTֵ
//  	������ڣ�A = TASK_EXEC_ATDAILY��Ч����λ�졣 ����ÿ��3�� P = 3;
//
//  Q: ����INTֵ
//  	��ǰ������A = AUTOTASK_EXEC_ATYEARDAY��Ч����λ�죬������ǰ3��
//
//  	T : ִ��ʱ��㣬һ������INTֵ����ֵ�ɽ���Ϊ����ʱ��
//  	����һ���е�13��30��14��30��16��20������
//
//  	X : 1���������ֵ���ԡ�, ���ָ������庬����A����
//  	1 > A = TASK_EXEC_ATDATE, XΪ������ڵ㣬�ɽ���Ϊ����
//  	2 > A = TASK_EXEC_ATDAILY, XΪһ������ֵ������P���������
//  	3 > A = TASK_EXEC_ATMONTHDAY, Xһ������ֵ����Ϊ�µ��պ�
//  	����ֵΪ��һ���ִ�У�ʹ��λ��ʾ����32λ�����λΪ1��..
//  	4 > A = TASK_EXEC_ATWEEKDAY, X����һ������ֵ��
//  	��ֵ��ʾ��һ�죨�����յ�����������ִ�У�ʹ��λ��ʶ��ʾ��ENUM_NTH_WEEKDAY
//  	5 > A = AUTOTASK_EXEC_ATYEARDAY��XΪһ������ֵ������[�� - ��]
//  	ÿ���ĳ������ 2011~2020 ��[11 / 11]��������һ����ǰ������������������
//////////////////////////////////////////////////////////////////////////

namespace cron {

class AbsTimer : public ExpTimer {
public:
    AbsTimer(const mstring& exp);
    ~AbsTimer();

    TimerRunningStatus nextExecDate(__inout uint32_t& ddate);

    virtual bool parse() override;

    virtual mstring description() override;

    virtual void execTimeSpot(std::vector<dt::time>& time_points) override;

    virtual TimerRunningStatus getNextExecTimeFrom(__inout dt::time& tm_test,
            __out dt::time& tm_exec, __out int32_t &period_s) override;

protected:
    TimerRunningStatus nextRightTimeFrom(__in const dt::time&tm_test,
                                            __in const dt::time& tm_exec, __inout uint32_t &dwNextExecTime);

private:
    uint32_t span_;		// ���ʱ���������

    Ints arx_;		// ִ�����ڵ㣬�����ENUM_TASK_EXECFLAG����ȷ����ֵ������
    Ints ar_time_;	// ִ��ʱ��㣬

    // ��һ���е� [03:00 ~ 20:00] ÿ�� 30m ִ��һ��
    uint16_t t_begin_;	// ִ��ʱ��
    uint16_t t_end_;
};

}