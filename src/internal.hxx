#ifndef FIBER_INTERNAL_HXX
#define FIBER_INTERNAL_HXX

#include "std_extension.hxx"
#include "fiber/fiber.hxx"
#include "fiber/ctrl.hxx"
#include "fiber/sync.hxx"
#include "wsq.hxx"

#include <deque>

namespace fiber
{
    enum class ctrl : u32
    {
        // Creation ctrl
        new_fiber = 0,

        // fiber ctrl
        yield,
        yield_to,
        wait_for_join,
        wait_for_cond,
        sleep_until,
        lock_mutex,
        resume,
        start_stealing,
        stop_stealing,

        // thread ctrl
        reschedule,
        noop = max_v,
    };

    using fiber_queue = work_stealing_queue<coro_handle>;

    union ctrl_data
    {
        struct
        {
            bool success_;
            mutex* mtx_;
        } lock_mutex_;
        coro_handle wait_for_join_;
        coro_handle yield_to_;
        std::chrono::time_point<std::chrono::high_resolution_clock> sleep_until_;
        std::reference_wrapper<const std::function<bool()>> wait_for_cond_;
    };

    struct thr_ctx;
    using ctx_ref = unique<thr_ctx*[]>;

    struct ctx_pool::impl
    {
        ctx_ref thr_ctxs_ = {};
        const usz limit_ = {};
        atomic_usz size_ = 0;
        std::mutex mtx_ = {};

        inline constexpr impl(usz limit) noexcept
            : thr_ctxs_(std::make_unique<thr_ctx*[]>(limit)),
              limit_(limit)
        {
        }
    };

    struct property::impl
    {
        spinlock lock_ = {};
        ctrl ctrl_ = ctrl::new_fiber;
        ctrl_data ctrl_data_ = {};

        // No lock is required for fields below
        atomic_bool detached_ = false;
    };

    struct thr_ctx
    {
        bool allow_stealing_ = true;
        ctrl ctrl_ = ctrl::noop;
        coro_handle curr_fiber_ = {};
        ctx_pool* pool_ = nullptr;
        ctrl_data ctrl_data_ = {};
        fiber_queue queue_ = fiber_queue();
    };

#define prop_of(fiber_handle) (*fiber_handle.promise().impl_)

    struct internal
    {
        static void //
        schedule(coro_handle h, bool reschedule = false) noexcept;

        inline static bool //
        lock_run(coro_handle h, const std::function<void()>& func, bool blocked = false) noexcept
        {
            return lock_run(prop_of(h), func, blocked);
        }

        inline static bool //
        lock_run(property::impl& prop, const std::function<void()>& func, bool blocked = false) noexcept
        {
            auto& lk = prop.lock_;
            if (blocked)
            {
                lk.lock();
            }
            else
            {
                if (!lk.try_lock())
                {
                    return false;
                }
            }

            func();

            lk.unlock();
            return true;
        }

        inline static coro_handle //
        steal_from(const ctx_pool& pool, thr_ctx& local)
        {
            coro_handle h = {};
            usz seed = castr<uptr>(&local);
            auto& other = *pool.impl_;

            usz curr_size = other.size_.load(std::memory_order_acquire); // this can only go up
            for (u32 q = 0; q < curr_size && !h; q++)
            {
                auto other_ctx = other.thr_ctxs_[(seed + q) % curr_size];
                if (other_ctx == &local)
                {
                    continue;
                }

                auto& other_queue = other_ctx->queue_;
                if (!other_queue.steal(h))
                {
                    h = {};
                }
            }

            return h;
        }
    };
}; // namespace fiber

#endif // FIBER_INTERNAL_HXX
