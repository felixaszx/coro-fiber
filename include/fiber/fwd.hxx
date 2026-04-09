#ifndef FIBER_FWD_HXX
#define FIBER_FWD_HXX

#include <coroutine>

namespace fiber
{
    struct coro_state;
    struct shared_ctx;
    struct thr_ctx;
    struct fiber;

    using coro_handle = std::coroutine_handle<coro_state>;

    struct coro : coro_handle
    {
        using promise_type = coro_state;
    };

}; // namespace fiber

namespace fiber::ctrl
{
    struct low_priority;

    template <typename Q = void>
    struct reschedule; // you can query using this control class, but this will suspend the fiber

    template <typename R = coro_state>
    struct query; // This will not usually suspend the fiber

    template <typename C = void>
    struct set; // set the coroutine control without yielding back to the scheduler
} // namespace fiber::ctrl

#endif // FIBER_FWD_HXX
