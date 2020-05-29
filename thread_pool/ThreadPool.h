#ifndef _THREAD_POOL_H__
#define _THREAD_POOL_H__

#include <list>
#include <thread>
#include <functional>
#include <memory>
#include <atomic>

#include "SyncQueue.h"

//#define MAX_THREAD_POOL_TASK_COUNT  =  100;
const int max_task_count = 20;
class ThreadPool {
    public:
        using Task = std::function<void()>;
        ThreadPool(int num_threads = std::thread::hardware_concurrency()) : m_queue(max_task_count) {
            start(num_threads);

            cout << "多少个内核 : " << num_threads << endl;
        }
        ~ThreadPool() {
            //没有停止则主动停止线程池
            stop();
        }

        void stop() {
            //保证多线程情况下只调用一次
            std::call_once(m_flag,[this] {stop_thread_group();});
        }

        void add_task(Task && task) {
            m_queue.put(std::forward<Task>(task));
        }

        void add_task(const Task & task) {
            m_queue.put(task);
        }
    private:
        void start(int num_threads) {
            m_running = true;
            //创建线程组
            for(int i = 0;i < num_threads;i++) {
                m_thread_group.push_back(std::make_shared<std::thread>(&ThreadPool::RunThread,this));
            }
        }

        void RunThread() {
            while(m_running) {
                //取任务分别执行
                std::list<Task> list;
                m_queue.take(list);

                for(auto &task : list) {
                    if(!m_running)
                        return ;
                    task();
                }
            }
        }

        void stop_thread_group() {
            m_queue.stop();

            m_running = false;

            for(auto thread : m_thread_group) {
                if(thread)
                    thread->join();
            }

            m_thread_group.clear();
        }

    private:
        std::list<std::shared_ptr<std::thread>> m_thread_group;
        SyncQueue<Task> m_queue;
        atomic_bool m_running;
        std::once_flag m_flag;
};

#endif //_THREAD_POOL_H__
