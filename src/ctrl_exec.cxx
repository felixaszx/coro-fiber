#include "internal.hxx"
#include <mutex>

using namespace fiber;

extern thread_local thr_ctx local;

inline static bool //
wait_for_join(property::impl& prop, coro_handle joining_for)
{
    bool done = false;
    bool run = internal::lock_run(joining_for, [&done, joining_for]() { done = joining_for.done(); });

    if (run && done)
    {
        prop.ctrl_data_ = {};
        return true;
    }

    // reschedule
    return false;
}

inline static bool //
lock_mutex(property::impl& prop)
{
    auto& mtx = prop.ctrl_data_.lock_mutex_;
    panic_if(mtx.success_, "Successful try_lock() should not be rescheduled");

    if (mtx.mtx_->try_lock())
    {
        prop.ctrl_data_ = {};
        return true;
    }

    // reschedule
    return false;
}

void //
this_thread::run_fiber(coro_handle h) noexcept
{
    bool exec = false;
    auto& prop = prop_of(h);

    // this fiber is reading by others, reschedule it
    if (!prop.lock_.try_lock())
    {
        this_thread::schedule(h);
        return;
    }

    switch (prop.ctrl_)
    {
        case ctrl::wait_for_join:
        {
            exec = wait_for_join(prop, prop.ctrl_data_.wait_for_join_);
            if (!exec)
            {
                this_thread::schedule(h, true);
            }
            break;
        }
        case ctrl::lock_mutex:
        {
            exec = lock_mutex(prop);
            if (!exec)
            {
                this_thread::schedule(h, true);
            }
            break;
        }
        default:
        {
            exec = !h.done();
            break;
        }
    }

    if (exec)
    {
        local.curr_fiber_ = h;
        prop.ctrl_ = ctrl::resume;
        prop.ctrl_data_ = {};
        h.resume(); // exit from fiber_ctx::await_suspend()

        if (h.done() && prop.detached_.load(std::memory_order_acquire))
        {
            h.destroy();
        }
    }

    prop.lock_.unlock();
}
