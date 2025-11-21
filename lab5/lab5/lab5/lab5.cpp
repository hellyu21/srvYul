#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>
#include <random>

class MyMutex {
private:
    std::atomic<bool> locked = false;

public:
    void lock() {
        bool expected = false;
        while (!locked.compare_exchange_weak(expected, true, std::memory_order_acquire)) {
            expected = false; 
            std::this_thread::yield();
        }
    }

    void unlock() {
        locked.store(false, std::memory_order_release);
    }
};

