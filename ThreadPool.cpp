#include "ThreadPool.hpp"
#include <iostream>

ThreadPool::ThreadPool()
{
  int num_threads = std::thread::hardware_concurrency(); // Get the number of available threads

  for (size_t i = 0; i < num_threads; ++i) 
  {
    workers.emplace_back(&ThreadPool::worker_loop, this);
  }

  // Start the printing thread
  printerThread = std::thread(&ThreadPool::printer_loop, this);
}

ThreadPool::ThreadPool(size_t num_threads)
{
  for (size_t i = 0; i < num_threads; ++i) 
  {
    workers.emplace_back(&ThreadPool::worker_loop, this);
  }
}

void ThreadPool::worker_loop() 
{
  while (true) 
  {
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      condition.wait(lock, [&]() {
          return taskStop || !tasks.empty();
      });

      if (taskStop && tasks.empty()) break;

      task = std::move(tasks.front());
      tasks.pop();
    }

    task();
  }
}

void ThreadPool::printer_loop() 
{
  while (true) 
  {
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> lock(printQueueMutex);
      printCondition.wait(lock, [&]() {
          return printStop || !printQueue.empty();
      });

      if (verbose)
      {
        std::cout << "printStop: " << printStop << ", printQueue.size(): " << printQueue.size() << "\n";
      }

      if (printStop && printQueue.empty()) 
      {
        if (verbose)
        {
          std::cout << "ThreadPool: Printer thread stopping.\n";
        }
        break;
      }

      if (!printQueue.empty()) 
      {
        task = std::move(printQueue.front());
        printQueue.pop();

        if (verbose)
        {
          std::cout << "ThreadPool: Task retrieved from printQueue. Remaining size: " << printQueue.size() << "\n";
        }
      }
    }
    task();
  }
}

void ThreadPool::submitTask(std::function<void()> task) 
{
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    tasks.push(std::move(task));
  }
  condition.notify_one();
}

void ThreadPool::submitPrintTask(std::function<void()> task) {
  {
    std::unique_lock<std::mutex> lock(printQueueMutex);
    printQueue.push(task);

    if (verbose)
    {
      std::cout << "ThreadPool: Task added to printQueue. Queue size: " << printQueue.size() << "\n";
    }
  }
  printCondition.notify_one();
}

void ThreadPool::shutdown() 
{
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    taskStop = true;
  }
  condition.notify_all();

  // Wait for all threads to finish
  for (std::thread &worker : workers) 
  {
    if (worker.joinable()) 
    {
      worker.join();
    }
  }

  // Signal the printer thread to stop after all tasks are submitted
  {
    std::unique_lock<std::mutex> lock(printQueueMutex);
    printStop = true; // Signal the printer thread to stop
  }
  printCondition.notify_one();

  // Wait for the printing thread to finish
  if (printerThread.joinable()) 
  {
    printerThread.join();
  }
}

ThreadPool::~ThreadPool() 
{
  shutdown();
}

//getters
int ThreadPool::getWorkerCount() 
{
  return workers.size();
}