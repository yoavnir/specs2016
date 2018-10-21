#ifndef SPECS2016__PROCESSING__STRINGQUEUE__H
#define SPECS2016__PROCESSING__STRINGQUEUE__H

#include <mutex>
#include <string>
#include <thread>
#include <queue>
#include <condition_variable>

typedef std::string* pstr;

#ifdef DEBUG
#define QUEUE_HIGH_WM 5
#define QUEUE_LOW_WM  2
#else
#define QUEUE_HIGH_WM 500
#define QUEUE_LOW_WM  200
#endif

// until we have C++17 and std::scoped_lock...
class scopedLock {
public:
	scopedLock(std::mutex* m) {m_lock = new std::unique_lock<std::mutex>(*m);}
	~scopedLock() {if (m_lock) delete(m_lock);}
	void unlock() {delete(m_lock); m_lock = NULL;}
	std::unique_lock<std::mutex>& ulock() {return *m_lock;}
private:
	std::unique_lock<std::mutex> *m_lock;
};

class StringQueue
{
private:
    std::queue<pstr> m_Queue;
    mutable std::mutex m_Mutex;
    std::condition_variable cv_QueueEmpty;
    std::condition_variable cv_QueueFull;
    bool m_Done;
public:
    StringQueue() {m_Done = false;}
    void push(pstr const& data)
    {
    	assert(data!=NULL);
        scopedLock lock(&m_Mutex);
        while (m_Queue.size()>=QUEUE_HIGH_WM) {
        	cv_QueueFull.wait(lock.ulock());
        }
        m_Queue.push(data);
        lock.unlock();
        cv_QueueEmpty.notify_one();
    }

    bool empty() const
    {
        scopedLock lock(&m_Mutex);
        return m_Queue.empty();
    }

    bool wait_and_pop(pstr& popped_value)
    {
        scopedLock lock(&m_Mutex);
        while(m_Queue.empty() && false==m_Done)
        {
            cv_QueueEmpty.wait(lock.ulock());
        }

        if (m_Done && m_Queue.empty()) return false;

        popped_value=m_Queue.front();
        m_Queue.pop();
        size_t queueSize = m_Queue.size();
        lock.unlock();
        if (queueSize < QUEUE_LOW_WM) {
        	cv_QueueFull.notify_one();
        }
        return true;
    }

    void Done() {
    	m_Done = true;
    	cv_QueueEmpty.notify_one();
    }

};


#endif
