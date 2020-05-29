#ifndef _GENO_SYNCQUEUE_H__
#define _GENO_SYNCQUEUE_H__

#include <list>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <iostream>

using namespace std;

template <typename T>
class SyncQueue {
    public:
        SyncQueue(int max_size):m_maxsize(max_size),m_need_stop(false) {}

        void put(const T &x) {
            add(x);
        }

        void put(T && x) {
            add(std::forward<T>(x));
        }

        void take(std::list<T> &list) {
            std::unique_lock<std::mutex> locker(m_mutex);
            m_noempty.wait(locker,[this] {return m_need_stop || not_empty();});

            if(m_need_stop)
                return ;

            list = std::move(m_queue);
            m_nofull.notify_one();
        }

        void take(T & t) {
            std::unique_lock<std::mutex> locker(m_mutex);
            m_noempty.wait(locker,[this] {return m_need_stop || not_empty();});

            if(m_need_stop)
                return ;

            t = m_queue.front();
            m_queue.pop_front();
            m_nofull.notify_one();
        }

        void stop() {
            {
                std::lock_guard<std::mutex> locker(m_mutex);
                m_need_stop = true;
            }

            m_nofull.notify_all();
            m_noempty.notify_all();
        }

        bool empty() {
            std::lock_guard<std::mutex> locker(m_mutex);
            return m_queue.empty();
        }

        bool full() {
            std::lock_guard<std::mutex> locker(m_mutex);
            return m_queue.size == m_maxsize;
        }

        size_t size() {
            std::lock_guard<std::mutex> locker(m_mutex);
            return m_queue.size();
        }

        int count() {
            return m_queue.size();
        }

    private:
        bool not_full() const {
            bool full = m_queue.size() >= m_maxsize;
            if(full)
                cout << "缓冲区满了,需要等待 " << endl;
            return !full;
        }

        bool not_empty() const {
            bool empty = m_queue.empty();
            if(empty)
                cout << "缓冲区空了,需要等待 异步线程 ID:" << this_thread::get_id() << endl;
            return !empty;
        }

        template <typename F>
        void add(F && x) {
            std::unique_lock<std::mutex> locker(m_mutex);
            m_nofull.wait(locker,[this] {return m_need_stop || not_full();});

            if(m_need_stop)
                return ;

            m_queue.push_back(std::forward<F>(x));
            m_noempty.notify_one();
        }

    private:
        std::list<T> m_queue;               ///< 缓冲区
        std::mutex m_mutex;                 ///< 互斥量和条件变量结合使用
        std::condition_variable m_noempty;  ///< 不为空的条件变量
        std::condition_variable m_nofull;   ///< 没有满的条件变量
        int m_maxsize;                      ///< 同步队列
        bool m_need_stop;
};





#endif //_GENO_SYNCQUEUE_H__
