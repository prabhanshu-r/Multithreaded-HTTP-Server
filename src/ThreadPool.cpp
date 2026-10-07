#include "server/ThreadPool.hpp"

#include <exception>

#include "server/Logger.hpp"

ThreadPool::ThreadPool(std::size_t threads, std::size_t queueLimit) : queueLimit_(queueLimit) {
    workers_.reserve(threads);
    for (std::size_t i = 0; i < threads; ++i) {
        workers_.emplace_back([this] { workerLoop(); });
    }
}

ThreadPool::~ThreadPool() { shutdown(); }

bool ThreadPool::submit(std::function<void()> job) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopping_ || jobs_.size() >= queueLimit_) return false;
        jobs_.push(std::move(job));
    }
    wakeup_.notify_one();  // wake exactly one sleeping worker
    return true;
}

void ThreadPool::shutdown() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopping_) return;
        stopping_ = true;
    }
    wakeup_.notify_all();
    for (std::thread& worker : workers_) {
        if (worker.joinable()) worker.join();
    }
}

void ThreadPool::workerLoop() {
    for (;;) {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            // Sleep until there is work or we are told to stop.
            wakeup_.wait(lock, [this] { return stopping_ || !jobs_.empty(); });
            if (jobs_.empty()) return;  // stopping and nothing left to do
            job = std::move(jobs_.front());
            jobs_.pop();
        }
        // Run the job outside the lock so other workers can take jobs meanwhile.
        try {
            job();
        } catch (const std::exception& e) {
            Logger::error(std::string("worker job failed: ") + e.what());
        } catch (...) {
            Logger::error("worker job failed with an unknown error");
        }
    }
}
