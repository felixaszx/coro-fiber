#include "internal.hxx"

using namespace fiber;

extern thread_local thr_ctx local;

bool //
mutex::try_lock [[nodiscard]] () noexcept
{
    return !(m_.load(std::memory_order_relaxed) || //
             m_.exchange(true, std::memory_order_acquire));
}

coro_ctx //
mutex::lock [[nodiscard]] () noexcept
{
    local.ctrl_ = ctrl::lock_mutex;
    auto& ctrl_data = local.ctrl_data_.lock_mutex_;
    ctrl_data.success_ = try_lock();
    ctrl_data.mtx_ = this;
    return {};
}

void //
mutex::unlock() noexcept
{
    m_.store(false, std::memory_order_release);
}
