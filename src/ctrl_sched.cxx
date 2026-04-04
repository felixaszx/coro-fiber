#include "internal.hxx"

#include <mutex>

using namespace fiber;

ctx_ref thr_ctxs = {};
atomic_usz queue_size = 0;
static std::mutex global_lock = {};

extern thread_local thr_ctx local;

void //
internal::schedule(coro_handle h, bool blocked_schedule) noexcept
{
    local.queue_.push(h);
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
        queue_size.fetch_add(1, std::memory_order_release);
        success = true;
    }

    global_lock.unlock();
    return success;
}

coro_handle //
this_thread::pick_next_fiber() noexcept
{
    coro_handle next = {};
    usz seed = castr<uptr>(&local);
    // No reason to release the lock if the local queue is empty
    if (local.queue_.empty())
    {
        if (!local.allow_stealing_)
        {
            return next;
        }

        usz curr_size = queue_size.load(std::memory_order_acquire); // this can only go up
        for (u32 q = 0; q < curr_size && !next; q++)
        {
            auto other_ctx = thr_ctxs[(seed + q) % curr_size];
            if (other_ctx == &local)
            {
                continue;
            }

            auto& other_queue = other_ctx->queue_;
            if (!other_queue.steal(next))
            {
                next = {};
            }
        }
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
