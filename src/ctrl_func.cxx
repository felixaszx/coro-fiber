#include "internal.hxx"

using namespace fiber;

extern thread_local thr_ctx local;

coro_ctx //
this_fiber::yield [[nodiscard]] () noexcept
{
    local.ctrl_ = ctrl::yield;
    return {};
}

coro_ctx //
this_fiber::yield_to [[nodiscard]] (coro_handle h) noexcept
{
    local.ctrl_ = ctrl::yield_to;
    local.ctrl_data_.yield_to_ = h;
    return {};
}

coro_ctx //
this_fiber::wait_for [[nodiscard]] (const std::function<bool()>& cond) noexcept
{
    local.ctrl_ = ctrl::wait_for_cond;
    local.ctrl_data_.wait_for_cond_ = std::ref(cond);
    return {};
}

coro_ctx //
this_fiber::noop [[nodiscard]] () noexcept
{
    local.ctrl_ = ctrl::noop;
    return {};
}

::fiber::fiber::id //
this_fiber::get_id [[nodiscard]] () noexcept
{
    return castr<fiber::id>(local.curr_fiber_.address());
}

coro_ctx //
this_thread::start_stealing [[nodiscard]] () noexcept
{
    local.ctrl_ = ctrl::start_stealing;
    return {};
}

coro_ctx //
this_thread::stop_stealing [[nodiscard]] () noexcept
{
    local.ctrl_ = ctrl::stop_stealing;
    return {};
}

coro_ctx //
this_fiber::sleep_until [[nodiscard]] (const std::chrono::time_point<std::chrono::high_resolution_clock>& tp) noexcept
{
    local.ctrl_ = ctrl::sleep_until;
    local.ctrl_data_.sleep_until_ = tp;
    return {};
}
