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

// until we have C++17 and std::scoped_lock...
class scopedLock {
public:
	scopedLock(std::mutex* m) {m_lock = new std::unique_lock<std::mutex>(*m);}
	~scopedLock() {if (m_lock) delete(m_lock);}
	void unlock() {delete(m_lock); m_lock = nullptr;}
	std::unique_lock<std::mutex>& ulock() {return *m_lock;}
private:
	std::unique_lock<std::mutex> *m_lock;
};

class StringQueue
{
private:
    std::queue<PSpecString> m_Queue;
    mutable std::mutex m_Mutex;
    std::condition_variable cv_QueueEmpty;
    std::condition_variable cv_QueueFull;
    queueTimer m_timer;
    bool m_Done;
public:
    StringQueue() {m_Done = false;}
    void push(PSpecString const& data)
    {
    	MYASSERT(data!=nullptr);
        scopedLock lock(&m_Mutex);
        while (m_Queue.size()>=QUEUE_HIGH_WM) {
        	cv_QueueFull.wait(lock.ulock());
        }
        m_Queue.push(data);
        m_timer.increment();
        lock.unlock();
        cv_QueueEmpty.notify_one();
    }

    bool empty() const
    {
        scopedLock lock(&m_Mutex);
        return m_Queue.empty();
    }

    bool wait_and_pop(PSpecString& popped_value)
    {
        scopedLock lock(&m_Mutex);
        while(m_Queue.empty() && false==m_Done)
        {
            cv_QueueEmpty.wait_for(lock.ulock(), std::chrono::milliseconds(2000));
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


#endif
