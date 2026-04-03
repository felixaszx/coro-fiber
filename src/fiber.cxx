#include "internal.hxx"

extern thread_local fiber::thr_ctx local;

fiber::fiber::~fiber() noexcept
{
    if (h_)
    {
        h_.destroy();
    }
}

void //
fiber::fiber::launch(const std::function<coro()>& func) noexcept
{
    h_ = func();
    this_thread::schedule(h_, false);
}

fiber::fiber::id //
fiber::fiber::get_id [[nodiscard]] () const noexcept
{
    return castr<id>(h_.address());
}

void //
fiber::fiber::detach() noexcept
{
    prop_of(h_).detached_.store(true, std::memory_order_release);
    h_ = {};
}

fiber::coro_ctx //
fiber::fiber::join [[nodiscard]] () noexcept
{
    if (joinable())
    {
        local.ctrl_ = ctrl::wait_for_join;
        local.ctrl_data_.wait_for_join_ = h_;
    }
    else
    {
        local.ctrl_ = ctrl::noop;
    }
    return {};
}

bool //
fiber::fiber::joinable [[nodiscard]] () const noexcept
{
    return bool(h_);
}
