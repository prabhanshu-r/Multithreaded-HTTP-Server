#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

// A fixed set of worker threads that take jobs from a shared queue.
//
//   main thread --submit()--> [ queue ] --> worker 1
//                                       --> worker 2 ...
class ThreadPool {
public:
    ThreadPool(std::size_t threads, std::size_t queueLimit);
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // Queues a job. Returns false if the queue is full or the pool is stopping.
    bool submit(std::function<void()> job);

    // Finishes every queued job, then joins all threads. Safe to call twice.
    void shutdown();

private:
    void workerLoop();

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> jobs_;
    std::mutex mutex_;
    std::condition_variable wakeup_;
    std::size_t queueLimit_;
    bool stopping_ = false;
};

#endif
