#ifndef FIBER_FWD_HXX
#define FIBER_FWD_HXX

#include <coroutine>

namespace fiber
{
    struct fiber;
    struct coro;
    struct coro_ctx;
    struct internal;
    struct property;
    struct this_fiber;
    struct this_thread;
    struct mutex;

    using coro_handle = std::coroutine_handle<property>;

    struct coro : coro_handle
    {
        using promise_type = property;
    };
}; // namespace fiber

#endif // FIBER_FWD_HXX
