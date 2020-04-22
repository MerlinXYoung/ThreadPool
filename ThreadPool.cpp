#include "ThreadPool.h"

using lock_type = std::unique_lock<std::mutex>;
// the constructor just launches some amount of workers
ThreadPool::ThreadPool(size_t threads)
    :   stop(false)
{
    for(size_t i = 0;i<threads;++i)
        workers.emplace_back(
            [this]
            {
                for(;;)
                {
                    task_type task;

                    {
                        lock_type lock(this->queue_mutex);
                        this->condition.wait(lock,
                            [this]{ return this->stop || !this->tasks.empty(); });
                        if(this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    task();
                }
            }
        );
}

// the destructor joins all threads
ThreadPool::~ThreadPool()
{
    {
        lock_type lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(auto &worker: workers)
        worker.join();
}
