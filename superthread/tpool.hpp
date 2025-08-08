#include <thread>
#include <future>
#include <vector>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <type_traits>

#ifndef THREADS_C
#define THREADS_C

class threadPool {
public:
    std::deque<std::function<void()>> tasks;

    // Create thread pool
    explicit threadPool(size_t num_threads = std::thread::hardware_concurrency()) : stop(false) {
        threads.reserve(num_threads);
        for (size_t i = 0; i < num_threads; ++i) {
            threads.emplace_back([this] { workerLoop(); });
        }
    }

    // Construct thread pool
    ~threadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (auto& thread : threads) {
            thread.join();
        }
    }

    // Add tasks to workers
    template <typename F, typename... Args>
    auto addTask(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        using ReturnType = std::invoke_result_t<F, Args...>;
        
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            [func = std::forward<F>(f), tup = std::make_tuple(std::forward<Args>(args)...)]() mutable {
                return std::apply(func, std::move(tup));
            }
        );

        std::future<ReturnType> future = task->get_future();

        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if(stop) {
                throw std::runtime_error("addTask called on stopped thread pool");
            }
            tasks.emplace_back([task]() { (*task)(); });
        }

        condition.notify_one();
        return future;
    }

    threadPool(const threadPool&) = delete;
    threadPool& operator=(const threadPool&) = delete;

private:
    // Single-thread loop
    void workerLoop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                condition.wait(lock, [this]() { return stop || !tasks.empty(); });
                
                if (stop && tasks.empty()) {
                    return;
                }
                
                task = std::move(tasks.front());
                tasks.pop_front();
            }
            task();
        }
    }

    std::vector<std::thread> threads;
    
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

#endif