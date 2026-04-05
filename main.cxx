#include <iostream>
#include <thread>
#include <latch>

#include "fiber/fiber.hxx"
#include "fiber/ctrl.hxx"

inline const std::size_t sample_size = 1'000'000;

using namespace std::chrono_literals;

__attribute__((noinline)) //
fiber::coro               //
test_func2()
{
    while (true)
    {
        co_await fiber::this_fiber::yield();
    }
}

__attribute__((noinline)) //
fiber::coro               //
test_func(bool a)
{
    auto diff = 0ns;
    auto prev = std::chrono::high_resolution_clock::now();
    std::size_t counter = 0;

    while (true)
    {
        prev = std::chrono::high_resolution_clock::now();
        co_await fiber::this_fiber::yield();
        diff += std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - prev);
        counter++;
        if (counter == sample_size)
        {
            auto avg = diff / counter;
            std::cout << std::format("Context switching time: {}ns, thr: {}\r", avg.count(),
                                     std::this_thread::get_id());
            counter = 0;
            diff = 0ns;
        }
    }
}

fiber::ctx_pool pool(std::thread::hardware_concurrency());

int //
main(int argc, const char** argv)
{
    std::vector<std::thread> test_thrs = {};
    std::latch terminated(std::thread::hardware_concurrency());

    for (std::size_t i = 1; i < std::thread::hardware_concurrency(); i++)
    {
        test_thrs.emplace_back(
            [&]()
            {
                fiber::this_thread::init_scheduler(pool);
                std::uint8_t cycle = 0;
                while (true)
                {
                    auto h = fiber::this_thread::pick_next_fiber();
                    if (h)
                    {
                        fiber::this_thread::run_fiber(h);
                        cycle = 0;
                    }
                    else
                    {
                        if (cycle > 0)
                        {
                            std::this_thread::sleep_for(1ns * cycle);
                        }
                        cycle++;
                    }
                }
                terminated.arrive_and_wait();
            });
    }

    fiber::this_thread::init_scheduler(pool);

    fiber::fiber a(test_func, false);

    std::uint8_t cycle = 0;
    while (true)
    {
        auto h = fiber::this_thread::pick_next_fiber();
        if (h)
        {
            fiber::this_thread::run_fiber(h);
            cycle = 0;
        }
        else
        {
            if (cycle > 0)
            {
                std::this_thread::sleep_for(1ns * cycle);
            }
            cycle++;
        }
    }

    terminated.arrive_and_wait();
    for (auto& thr : test_thrs)
    {
        thr.join();
    }

    return 0;
}
