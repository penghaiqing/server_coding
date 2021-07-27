/*
 * @Author: your name
 * @Date: 2021-07-20 02:10:28
 * @LastEditTime: 2021-07-20 02:26:19
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /network_program/server_coding/timer_alarm/timer.cpp
 */
#include "timer.h"
#include <time.h>

//std::atomic<int> Timer::s_initialId = 0; // 它是已删除函数C/C++(1815) 因为已经 delete 了atomic的拷贝构造函数
//Timer::s_initialId = 0;

Timer::Timer(int32_t repeatedTimes, int64_t interval, const TimerCallback& timerCallback)
{
    m_repeatedTimes = repeatedTimes;
    m_interval = interval;

    //当前时间加上触发间隔得到下一次的过期时间
    m_expiredTime = (int64_t)time(nullptr) + interval;

    m_callback = timerCallback;

    //生成一个唯一的id
    ++s_initialId;
    m_id = s_initialId;
}

bool Timer::isExpired()
{
    int64_t now = time(nullptr);
    return now >= m_expiredTime;
}

void Timer::run()
{
    m_callback();

    if (m_repeatedTimes >= 1)
    {
        --m_repeatedTimes;
    }

    m_expiredTime += m_interval;
}