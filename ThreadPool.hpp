#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class ThreadPool 
{
public:
    ThreadPool(); // Default constructor: Initialize with available threads
    ThreadPool(size_t num_threads);
    ~ThreadPool();

    void submitTask(std::function<void()> task);
    void submitPrintTask(std::function<void()> task);
    void shutdown();

    int getWorkerCount();

private:
    void worker_loop(); // Worker thread loop
    std::vector<std::thread> workers; // Pool of worker threads
    std::queue<std::function<void()>> tasks; // Queue of tasks to be executed
    std::mutex queue_mutex; // Mutex for thread synchronization
    std::condition_variable condition; // Condition variable to manage task execution
    std::atomic<bool> taskStop = false; // Flag to stop the threads when shutting down

    void printer_loop(); // Function for the printing thread
    std::thread printerThread;
    std::queue<std::function<void()>> printQueue; // Queue for messages to print
    std::mutex printQueueMutex;
    std::condition_variable printCondition;
    std::atomic<bool> printStop = false;

    bool verbose = false; // Flag to enable verbose output
};

#endif // THREADPOOL_HPP
