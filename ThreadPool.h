#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>


// 201103L (C++11)
// 201402L (C++14)
// 201703L (C++17)
#ifdef DESPLAY_CXX_STANDARD
#if __cplusplus >= 201703L
    #warning C++ 17
#elif __cplusplus >= 201402L 
    #warning C++ 14
#else
    #warning C++ 11
#endif
#endif//DESPLAY_CXX_STANDARD

class ThreadPool {
public:
    ThreadPool(size_t threads );
    inline ThreadPool():ThreadPool(std::max(2u, std::thread::hardware_concurrency())){}
    ThreadPool(ThreadPool&& pool);
    template<class F, class... Args>
#if __cplusplus >= 201402L
    decltype(auto) enqueue(F&& f, Args&&... args) ;
#else
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>;
#endif
    ~ThreadPool();
private:
    
    ThreadPool(const ThreadPool& )=delete;
    ThreadPool& operator=(const ThreadPool& )=delete;

    // need to keep track of threads so we can join them
    std::vector< std::thread > workers;
    // the task queue
    using task_type = std::function<void()>;
    std::queue< task_type > tasks;
    
    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

// add new work item to the pool
template<class F, class... Args>
#if __cplusplus >= 201402L
decltype(auto) ThreadPool::enqueue(F&& f, Args&&... args) 
#else
auto ThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
#endif
{
#if __cplusplus >= 201703L
    using return_type = std::invoke_result_t<F, Args...>;
#else
    using return_type = typename std::result_of<F(Args...)>::type;
#endif
    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task](){ task->operator()(); });
    }
    condition.notify_one();
    return res;
}

#endif//__THREAD_POOL_H__
