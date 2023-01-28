#ifndef SPECS2016__PROCESSING__STRINGQUEUE__H
#define SPECS2016__PROCESSING__STRINGQUEUE__H

#include <mutex>
#include "SpecString.h"
#include <thread>
#include <memory>
#include <queue>
#include <condition_variable>
#include "utils/platform.h"
#include "utils/TimeUtils.h"

typedef std::unique_lock<std::mutex> uniqueLock;

template <class T> class MTQueue
{
private:
    std::queue<T> m_Queue;
    mutable std::mutex m_Mutex;
    std::condition_variable cv_QueueEmpty;
    std::condition_variable cv_QueueFull;
    queueTimer m_timer;
    bool m_Done;
public:
    MTQueue() : m_Done(false) {}
    void push(T const& data)
    {
    	MYASSERT(data!=nullptr);
        uniqueLock lock(m_Mutex);
        while (m_Queue.size()>=QUEUE_HIGH_WM) {
        	cv_QueueFull.wait(lock);
        }
        m_Queue.push(data);
        m_timer.increment();
        lock.unlock();
        cv_QueueEmpty.notify_one();
    }

    bool empty() const
    {
        uniqueLock lock(m_Mutex);
        return m_Queue.empty();
    }

    bool wait_and_pop(T& popped_value)
    {
        uniqueLock lock(m_Mutex);
        while(m_Queue.empty() && false==m_Done)
        {
            cv_QueueEmpty.wait_for(lock, std::chrono::milliseconds(2000));
        }

        if (m_Done && m_Queue.empty()) return false;

        popped_value=m_Queue.front();
        m_Queue.pop();
        size_t queueSize = m_Queue.size();
        m_timer.decrement();
        lock.unlock();
        if (queueSize < QUEUE_LOW_WM) {
        	cv_QueueFull.notify_one();
        }
        return true;
    }

    void Done() {
    	m_Done = true;
    	m_timer.drain();
    	cv_QueueEmpty.notify_one();
    }

    void DumpStats(std::string title) {
    	m_timer.dump(title);
    }

};


typedef MTQueue<PSpecString> StringQueue;

#endif
