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
    size_t m_highWaterMark;
    size_t m_lowWaterMark;
    size_t m_notifyThreshold;
    static constexpr size_t maxNotifyBatch = 64;
    static constexpr auto maxBatchDelay = std::chrono::milliseconds(10);
    static size_t computeLowWaterMark(size_t hwm) {
        return (hwm > 10) ? (hwm - hwm / 10) : (hwm > 2 ? hwm - 2 : 0);
    }
    static size_t computeNotifyThreshold(size_t hwm) {
        size_t half = hwm / 2;
        return (half < 1) ? 1 : ((half > maxNotifyBatch) ? maxNotifyBatch : half);
    }
public:
    MTQueue(size_t highWaterMark = QUEUE_HIGH_WM)
        : m_Done(false)
        , m_highWaterMark(highWaterMark)
        , m_lowWaterMark(computeLowWaterMark(highWaterMark))
        , m_notifyThreshold(computeNotifyThreshold(highWaterMark))
    {
        m_timer.setCapacity(m_highWaterMark);
    }
    void push(T const& data)
    {
    	MYASSERT(data!=nullptr);
        uniqueLock lock(m_Mutex);
        while (m_Queue.size()>=m_highWaterMark) {
        	cv_QueueFull.wait(lock);
        }
        m_Queue.push(data);
        size_t queueSize = m_Queue.size();
        m_timer.increment();
        lock.unlock();
        // Wake the consumer only once a batch has accumulated; a consumer
        // waiting on a trickle of records picks them up on its timeout.
        if (queueSize >= m_notifyThreshold) {
            cv_QueueEmpty.notify_one();
        }
    }

    bool empty() const
    {
        uniqueLock lock(m_Mutex);
        return m_Queue.empty();
    }

    bool wait_and_pop(T& popped_value)
    {
        uniqueLock lock(m_Mutex);
        if (m_Queue.empty()) {
            while (!m_Done && m_Queue.size() < m_notifyThreshold) {
                if (cv_QueueEmpty.wait_for(lock, maxBatchDelay) == std::cv_status::timeout
                        && !m_Queue.empty()) {
                    break;
                }
            }
        }

        if (m_Queue.empty()) return false;  // only when m_Done

        popped_value=m_Queue.front();
        m_Queue.pop();
        size_t queueSize = m_Queue.size();
        m_timer.decrement();
        lock.unlock();
        if (queueSize < m_lowWaterMark) {
        	cv_QueueFull.notify_one();
        }
        return true;
    }

    void Done() {
        uniqueLock lock(m_Mutex);
    	m_Done = true;
    	m_timer.drain();
        lock.unlock();
    	cv_QueueEmpty.notify_all();
    }

    void setCapacity(size_t highWaterMark) {
        m_highWaterMark = highWaterMark;
        m_lowWaterMark = computeLowWaterMark(highWaterMark);
        m_notifyThreshold = computeNotifyThreshold(highWaterMark);
        m_timer.setCapacity(highWaterMark);
    }

    void DumpStats(std::string title) {
    	m_timer.dump(title);
    }

};


typedef MTQueue<PSpecString> StringQueue;

#endif
