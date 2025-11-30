#include <iostream>
#include <coroutine>
#include <chrono>
#include <thread>

std::string myname = "YurovaLena";

struct promise_type
{
    int current_value = 0;
    auto get_return_object()
    {
        return std::coroutine_handle<promise_type>::from_promise(*this);
    }

    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() {}

    std::suspend_always yield_value(int value)
    {
        current_value = value;
        return {};
    }
};

struct task
{
    std::coroutine_handle<promise_type> handle;

    task(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~task() { if (handle) handle.destroy(); }

    void resume() { handle.resume(); }
    bool done() const { return handle.done(); }
    int get_value() const { return handle.promise().current_value; }
};

namespace std
{
    template<>
    struct coroutine_traits<task, int>
    {
        using promise_type = promise_type;
    };
}

task long_computation(int steps)
{
    for (int i = 1; i <= steps; i++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        co_yield i;
    }
    co_return;
}

void print_progress(int current, int total)
{
    float percent = static_cast<float>(current) / total * 100.f;
    int bar_width = myname.length() + 1;
    std::string bar(bar_width, ' ');
    size_t pos = static_cast<size_t>(percent / 100.f * bar_width);
    for (size_t i = 0; i < pos; ++i) bar[i] = myname[i];
    if (pos > 0 && pos <= bar.size()) bar[pos - 1] = '>';

    std::cout << "\r[" << bar << "] " << std::fixed << std::setprecision(1) << percent << "%";
    std::flush(std::cout);
}

int main()
{
    constexpr int TOTAL_STEPS = 100;
    auto coro = long_computation(TOTAL_STEPS);

    while (!coro.done())
    {
        coro.resume();
        int progress = coro.get_value();
        print_progress(progress, TOTAL_STEPS);
        std::this_thread::yield();
    }
    std::cout << "\nDone" << std::endl;
    return 0;
}