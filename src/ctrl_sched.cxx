#include "internal.hxx"

#include <mutex>

using namespace fiber;

ctx_ref thr_ctxs = {};
atomic_usz queue_size = 0;
static std::mutex global_lock = {};

extern thread_local thr_ctx local;

void //
this_thread::schedule(coro_handle h, bool blocked_schedule) noexcept
{
    local.lock_.lock();
    local.queue_.push_back(h);
    local.lock_.unlock();
}

void //
this_thread::schedule_next(coro_handle h) noexcept
{
    local.lock_.lock();
    local.queue_.push_front(h);
    local.lock_.unlock();
}

bool //
this_thread::init_scheduler(bool allow_stealing) noexcept
{
    bool success = false;
    global_lock.lock();

    local.allow_stealing_ = allow_stealing;

    // To prevent any memory allocation
    if (!thr_ctxs)
    {
        thr_ctxs = std::make_unique<thr_ctx*[]>(std::thread::hardware_concurrency());
    }

    usz curr_size = queue_size;
    if (queue_size.load(std::memory_order_relaxed) < std::thread::hardware_concurrency())
    {
        thr_ctxs[curr_size] = &local;
        queue_size.fetch_add(1, std::memory_order_relaxed);
        success = true;
    }

    global_lock.unlock();
    return success;
}

coro_handle //
this_thread::pick_next_fiber() noexcept
{
    coro_handle next = {};
    local.lock_.lock(); // Always lock the local_queue

    // No reason to release the lock if the local queue is empty
    if (local.queue_.empty())
    {
        if (!local.allow_stealing_)
        {
            local.lock_.unlock();
            return next;
        }

        usz curr_size = queue_size.load(std::memory_order_relaxed); // this can only go up
        for (u32 q = 0; q < curr_size && !next; q++)
        {
            auto other_ctx = thr_ctxs[q];
            if (other_ctx == &local)
            {
                continue;
            }

            // All empty queues are locked already
            if (other_ctx->lock_.try_lock())
            {
                auto& other_queue = other_ctx->queue_;

                // Steal from this queue
                if (!other_queue.empty())
                {
                    next = other_queue.back();
                    other_queue.pop_back();
                }

                other_ctx->lock_.unlock();
            }
        }
    }
    else
    {
        next = local.queue_.front();
        local.queue_.pop_front();
    }

    local.lock_.unlock();
    return next;
}
