#include "internal.hxx"

using namespace fiber;

thread_local thr_ctx local = {};

bool //
coro_ctx::awaitable::await_ready [[nodiscard]] () noexcept
{
    switch (local.ctrl_)
    {
        case ctrl::wait_for_join:
        {
            auto jh = local.ctrl_data_.wait_for_join_;
            bool done = false;
            bool run = internal::lock_run(jh, [&done, jh]() { done = jh.done(); });
            return run && done;
        }
        case ctrl::wait_for_cond:
        {
            return local.ctrl_data_.wait_for_cond_();
        }
        case ctrl::yield:
        case ctrl::yield_to:
        {
            return false;
        }
        case ctrl::lock_mutex:
        {
            return local.ctrl_data_.lock_mutex_.success_;
        }
        case ctrl::sleep_until:
        {
            return false;
        }
        case ctrl::noop:
        case ctrl::resume:
        case ctrl::new_fiber:
        {
            return true;
        }
        case ctrl::start_stealing:
        {
            local.allow_stealing_ = true;
            return true;
        }

        case ctrl::stop_stealing:
        {
            local.allow_stealing_ = false;
            return true;
        }
    }
}

void //
coro_ctx::awaitable::await_suspend(coro_handle h) noexcept
{
    panic_if(h != local.curr_fiber_, //
             "coro_ctx::await_suspend() does not executed on the same thread with the calling fiber");

    switch (local.ctrl_)
    {
        case ctrl::yield:
        case ctrl::yield_to:
        case ctrl::lock_mutex:
        case ctrl::wait_for_join:
        case ctrl::wait_for_cond:
        case ctrl::sleep_until:
        {
            internal::schedule(h, true);
            break;
        }
        default:
        {
            break;
        }
    }

    auto& prop = prop_of(h);
    prop.ctrl_ = local.ctrl_;
    prop.ctrl_data_ = local.ctrl_data_;
    local.ctrl_ = ctrl::noop;
    local.ctrl_data_ = {};
}