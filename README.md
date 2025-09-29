# thread-pool-cpp
A thread pool implementation for C++

## Building the Library

### Prerequisites
- CMake 3.16.3 or higher
- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019+)

### Build Steps
```bash
# Clone the repository
git clone https://github.com/tnr00071/thread-pool-cpp.git
cd thread-pool-cpp

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make
```

## Linking the Library

### Method 1: Using CMake (Recommended)

If you're using CMake in your project, add this library as a subdirectory:

```cmake
# In your project's CMakeLists.txt
add_subdirectory(path/to/thread-pool-cpp)

# Link to your executable
target_link_libraries(your_executable_name PRIVATE thread-pool-cpp)
```

### Method 2: Using find_package (for installed library)

```cmake
find_package(thread-pool-cpp REQUIRED)
target_link_libraries(your_executable_name PRIVATE thread-pool-cpp)
```

### Method 3: Manual Linking

If building manually:
```bash
# Compile your project with the library
g++ -std=c++20 -pthread your_code.cpp -I/path/to/thread-pool-cpp -L/path/to/build -lthread-pool-cpp -o your_program
```

## Usage Examples

### Basic Usage

```cpp
#include "ThreadPool.hpp"
#include <iostream>
#include <chrono>

int main() {
    // Create thread pool with default number of threads (hardware_concurrency)
    ThreadPool pool;
    
    // Or specify number of threads
    // ThreadPool pool(4);
    
    // Submit tasks
    for (int i = 0; i < 10; ++i) {
        pool.submitTask([i]() {
            std::cout << "Task " << i << " executing on thread " 
                      << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }
    
    // Submit print tasks (these go to a dedicated print thread)
    pool.submitPrintTask([]() {
        std::cout << "This is printed from the dedicated print thread" << std::endl;
    });
    
    // Wait a bit for tasks to complete
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Shutdown the pool
    pool.shutdown();
    
    return 0;
}
```

### Advanced Usage

```cpp
#include "ThreadPool.hpp"
#include <vector>
#include <numeric>

// Example: Parallel computation
int main() {
    ThreadPool pool(8);  // 8 worker threads
    
    std::vector<int> data(1000);
    std::iota(data.begin(), data.end(), 1);  // Fill with 1, 2, 3, ..., 1000
    
    std::atomic<long long> sum{0};
    const size_t chunk_size = 100;
    
    // Process data in chunks
    for (size_t i = 0; i < data.size(); i += chunk_size) {
        pool.submitTask([&data, &sum, i, chunk_size]() {
            long long local_sum = 0;
            size_t end = std::min(i + chunk_size, data.size());
            
            for (size_t j = i; j < end; ++j) {
                local_sum += data[j] * data[j];  // Square each number
            }
            
            sum += local_sum;
        });
    }
    
    // Log progress using print thread
    pool.submitPrintTask([&sum]() {
        std::cout << "Current sum: " << sum.load() << std::endl;
    });
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << "Final sum of squares: " << sum.load() << std::endl;
    
    pool.shutdown();
    return 0;
}
```

## API Reference

### Constructors
- `ThreadPool()` - Creates thread pool with `std::thread::hardware_concurrency()` threads
- `ThreadPool(size_t num_threads)` - Creates thread pool with specified number of threads

### Methods
- `void submitTask(std::function<void()> task)` - Submit a task to the worker thread pool
- `void submitPrintTask(std::function<void()> task)` - Submit a task to the dedicated print thread
- `void shutdown()` - Gracefully shutdown the thread pool
- `int getWorkerCount()` - Get the number of worker threads

### Notes
- The thread pool automatically starts threads on construction
- Use `submitPrintTask()` for thread-safe console output
- Always call `shutdown()` before the ThreadPool object goes out of scope
- The destructor will call `shutdown()` automatically

## Requirements
- C++20 or later
- Threading support (automatically linked via CMake)

## License
[Add your license information here]
