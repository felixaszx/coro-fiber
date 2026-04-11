#include <iostream>

#include "fiber/fiber.hxx"
#include "fiber/thr_ctx.hxx"
#include "fiber/ctrl.hxx"

fiber::coro //
test_coro()
{
    std::cout << "test_coro\n";
    co_yield fiber::ctrl::reschedule();
    std::cout << "test_coro is back!\n";
    co_return;
}

fiber::coro //
test_coro2()
{
    std::cout << "test_coro2\n";
    co_yield fiber::ctrl::reschedule();
    std::cout << "test_coro2 is back!\n";
    co_return;
}

int //
main(int argc, char** argv)
{
    fiber::shared_ctx shared_ctx = {};
    fiber::thr_ctx local_ctx(shared_ctx);
    fiber::fiber fb({}, local_ctx, test_coro);
    fiber::fiber fb2({}, local_ctx, test_coro2);

    fiber::coro_handle h = {};
    while (h = local_ctx.next_fiber(), h)
    {
        local_ctx.run_fiber(h);
    }

    return 0;
}
