#include "internal.hxx"

#include <mutex>

using namespace fiber;

extern thread_local thr_ctx local;

void //
internal::schedule(coro_handle h, bool blocked_schedule) noexcept
{
    local.queue_.push(h);
}

ctx_pool::ctx_pool(std::size_t ctx_limit) noexcept
    : impl_(std::make_unique<impl>(ctx_limit))
{
}

ctx_pool::~ctx_pool() noexcept = default;

bool //
this_thread::init_scheduler(ctx_pool& pool, bool allow_stealing) noexcept
{
    bool success = false;
    local.allow_stealing_ = allow_stealing;

    std::unique_lock lk(pool.impl_->mtx_);
    auto& global = pool.impl_;

    usz curr_size = global->size_.load(std::memory_order_relaxed);
    if (curr_size < global->limit_)
    {
        global->thr_ctxs_[curr_size] = &local;
        global->size_.fetch_add(1, std::memory_order_release);
        local.pool_ = &pool;
        success = true;
    }

    return success;
}

coro_handle //
this_thread::pick_next_fiber() noexcept
{
    coro_handle next = {};

    // No reason to release the lock if the local queue is empty
    if (local.queue_.empty())
    {
        if (!local.allow_stealing_)
        {
            return next;
        }

        next = internal::steal_from(*local.pool_, local);
    }
    else
    {
        if (!local.queue_.steal(next))
        {
            next = {};
        }
    }

    return next;
}
